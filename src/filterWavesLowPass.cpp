#include <iostream>

#include "TFile.h"
#include "TTree.h"
#include "TMath.h"

void Convolute(float* coeff, int conLen, Float_t* input, Float_t* output, int inputLen) // the convolution is "centered" to avoid delaying the output wrt the input
{
  for(int i = (conLen - 1) / 2 ; i < inputLen - (conLen - 1) / 2; ++i){
    output[i] = 0;
    for(int j = 0; j < conLen; ++j)
      output[i] += coeff[j] * input [i + j - (conLen - 1) / 2];
  }

  for(int i = 0; i < (conLen - 1) / 2; ++i) // cannot convolute first points... i think
    output[i] = output[(conLen - 1) / 2];

  for(int i = inputLen - (conLen - 1) / 2; i < inputLen; ++i) // cannot convolute last points... i think
    output[i] = output[inputLen - (conLen - 1) / 2 - 1];
  
  return;
}

float* GenLPFCoeff(float cutFreq, int filtLen, float dt)
{
  float* ret = new float[filtLen];

  // calculate the omega cut in units of dt time
  float omegac = 2 * TMath::Pi() * cutFreq * dt;

  for(int i = 0; i < filtLen; ++i){
    if(i == (filtLen - 1) / 2){ // avoid nan due to the "ideal" filter denominator
      ret[i] = 0.3; // arbitrary number, it will be corrected afterwards
      continue;
    }
    
    ret[i] = TMath::Sin(omegac * (i - (filtLen - 1) / 2)) / (TMath::Pi() * (i - (filtLen - 1) / 2)); // "ideal" filter shifted and truncated
    ret[i] *= 0.54 - 0.46 * TMath::Cos(2 * TMath::Pi() * i / filtLen); // Hamming window
  }

  // determine point at (filtLen - 1) / 2 from normalization
  float sum = 0;
  for(int i = 0; i < filtLen; ++i){
    if(i == (filtLen - 1) / 2) continue; // avoid the point that needs to be determined

    sum += ret[i];
  }

  ret[(filtLen - 1) / 2] = 1 - sum;
  
  return ret;
}

int main(int argc, char* argv[])
{
  if(argc != 10){
    std::cout << "\tUsage filterWavesLowPass fileName f1 npt1 f2 npt2 f3 npt3 f4 npt4" << std::endl;
    std::cout << "\tAll the npt must be positive odd integers, frequencies and number of points set to 0 avoid filtering on that channel" << std::endl;
    return -1;
  }

  const int outCh = 4; // number of channels used in producing the root file

  float freqCut[outCh];
  int nptFilt[outCh];
  for(int i = 0; i < outCh; ++i){ // assign running average points
    freqCut[i] = atof(argv[2+i*2]);
    nptFilt[i] = atoi(argv[3+i*2]);
    
    if((nptFilt[i] < 3 || nptFilt[i] % 2 == 0) && nptFilt[i] != 0){ // check that the points are more than 3 and odd, or 0s
      std::cout << "Error: The number of points does not fulfill the requrements: they must be positive odd integers" << std::endl;
      return -1;
    }
  }
  
  std::string inFileName = argv[1];
  TFile* inFile = TFile::Open(inFileName.c_str());

  TTree* preambleOld = (TTree*) inFile->Get("preamble");
  TTree* wavTree = (TTree*) inFile->Get("waves");

  UInt_t npt = 1e6;
  Float_t* channels[outCh];
  for(int i = 0; i < outCh; ++i)
    channels[i] = new Float_t[npt];

  Float_t* time = new Float_t[npt];
  UInt_t transfer;
  ULong64_t event;
  
  wavTree->SetBranchAddress("npt", &npt);

  char name[100];
  for(int i = 0; i < outCh; ++i){
    sprintf(name, "ch%d", i+1);
    wavTree->SetBranchAddress(name, channels[i]);
  }

  wavTree->SetBranchAddress("time", time);
  wavTree->SetBranchAddress("transfer", &transfer);
  wavTree->SetBranchAddress("event", &event);
  
  // open output root file
  std::string outFileName = inFileName;
  std::string rep = std::string("."); // to be replaced
  std::string add = std::string("_filtLowPass."); // replacement
  std::string::size_type i = outFileName.find_last_of(rep);
  
  if(i != std::string::npos)
    outFileName.replace(i, rep.length(), add);
  else
    outFileName.insert(0, add);
  
  TFile* outFile = TFile::Open(outFileName.c_str(), "RECREATE");
  TTree* preambleNew = preambleOld->CloneTree(); // copy the preamble
  TTree* wavTreeNew = new TTree(wavTree->GetName(), wavTree->GetTitle());
  TTree* filterProp = new TTree("filterProp", "Number of points used in the running average of each channel");
  int nCh = outCh; // to be used by tree
  filterProp->Branch("nCh", &nCh,"nCh/I");
  filterProp->Branch("freqCut", freqCut, "freqCut[nCh]/I");
  filterProp->Branch("nptFilt", nptFilt, "nptFilt[nCh]/I");
  filterProp->Fill();
  filterProp->Write();
  
  wavTreeNew->Branch("npt", &npt, "npt/i");

  Float_t* channelsFilt[outCh];
  for(int i = 0; i < outCh; ++i)
    channelsFilt[i] = new Float_t[npt];
  
  char brName[100];
  char brNameType[100];
  for(int i = 0; i < outCh; ++i){
    sprintf(brName, "ch%d", i+1);
    sprintf(brNameType, "ch%d[npt]/F", i+1);
    wavTreeNew->Branch(brName, channelsFilt[i], brNameType);
  }

  wavTreeNew->Branch("time", time, "time[npt]/F");
  wavTreeNew->Branch("transfer", &transfer, "transfer/i");
  wavTreeNew->Branch("event", &event, "event/l");

  long int nentries = wavTree->GetEntries();
  float* filtCoeff[outCh]; // coefficients for the filters

  for(long int i = 0; i < nentries; ++i){
    wavTree->GetEntry(i);

    if(i == 0) // generate filter coefficients
      for(int iCh = 0; iCh < outCh; ++iCh){
	if(freqCut[iCh] == 0 || nptFilt[iCh] == 0) continue; // use 0 to not apply cuts
	filtCoeff[iCh] = GenLPFCoeff(freqCut[iCh], nptFilt[iCh], time[1] - time[0]);
      }
    
    if((i+1) % 1000 == 0 || (i+1) == nentries)
      std::cout << " Processing event " << i+1 << " / " << nentries << "                             \r" << std::flush;

    for(int iCh = 0; iCh < outCh; ++iCh){
      if(freqCut[iCh] == 0 || nptFilt[iCh] == 0){ // use 0 to not apply cuts
	for(unsigned int j = 0; j < npt; ++j)
	  channelsFilt[iCh][j] = channels[iCh][j];
	continue;
      }
      Convolute(filtCoeff[iCh], nptFilt[iCh], channels[iCh], channelsFilt[iCh], npt);// apply filters
    }
 
    wavTreeNew->Fill();
  }

  std::cout << std::endl;
  
  preambleNew->Write();
  wavTreeNew->Write();
  
  outFile->Close();
  inFile->Close();
  
  return 0;
}
