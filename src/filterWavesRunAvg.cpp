#include <iostream>

#include "TFile.h"
#include "TTree.h"

void applyFilter(Float_t* data, Float_t* filtData, int len, int ptAvg) // "recursive" implementation of the filter
{
  double sum = 0; // use double to reduce float rounding errors
  
  // equation: y[i] = y[i-1] + x[i+p]/ptAvg - x[i-q]/ptAvg with p = (ptAvg - 1)/2 and q = p+1
  int p = (ptAvg-1)/2;
  int q = p+1;

  // calculate first point of the filtered array
  for(int i = 0; i < ptAvg; ++i)
    sum += data[i];
  
  sum /= ptAvg; // first point is determined
  filtData[p] = sum;
  
  for(int i = p + 1; i < len - p; ++i){ // apply recursive running average
    sum += (data[i+p] - data[i-q]) / ptAvg;
    filtData[i] = sum;
  }
  
  for(int i = 0; i < p; ++i) // copy first points (where average cannot be applied)
    filtData[i] = data[i];

  for(int i = len - p; i < len; ++i) // copy last points (where average cannot be applied)
    filtData[i] = data[i];
  
  return;
}

int main(int argc, char* argv[])
{
  if(argc != 6){
    std::cout << "\tUsage filterWavesRunAvg fileName npt1 npt2 npt3 npt4" << std::endl;
    std::cout << "\tAll the npt must be positive odd integers" << std::endl;
    return -1;
  }

  const int outCh = 4; // number of channels used in producing the root file

  int nptAvg[outCh];
  for(int i = 0; i < outCh; ++i){ // assign running average points
    nptAvg[i] = atoi(argv[2+i]);
    if(nptAvg[i] < 1 || nptAvg[i] % 2 == 0){ // check that the points are more than 1 and odd
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
  std::string add = std::string("_filtRunAvg."); // replacement
  std::string::size_type i = outFileName.find_last_of(rep);
  
  if(i != std::string::npos)
    outFileName.replace(i, rep.length(), add);
  else
    outFileName.insert(0, add);
  
  TFile* outFile = TFile::Open(outFileName.c_str(), "RECREATE");
  TTree* preambleNew = preambleOld->CloneTree(); // copy the preamble
  TTree* wavTreeNew = new TTree(wavTree->GetName(), wavTree->GetTitle());
  TTree* nPointsAverage = new TTree("nPointsAverage", "Number of points used in the running average of each channel");
  int nCh = outCh; // to be used by tree
  nPointsAverage->Branch("nCh", &nCh,"nCh/I");
  nPointsAverage->Branch("nptAvg", nptAvg, "nptAvg[nCh]/I");
  nPointsAverage->Fill();
  nPointsAverage->Write();
  
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

  for(long int i = 0; i < nentries; ++i){
    wavTree->GetEntry(i);

    if((i+1) % 1000 == 0 || (i+1) == nentries)
      std::cout << " Processing event " << i+1 << " / " << nentries << "                             \r" << std::flush;

    for(int iCh = 0; iCh < outCh; ++iCh)
      applyFilter(channels[iCh], channelsFilt[iCh], npt, nptAvg[iCh]);
    
    wavTreeNew->Fill();
  }
  
  preambleNew->Write();
  wavTreeNew->Write();
  
  outFile->Close();
  inFile->Close();
  
  return 0;
}
