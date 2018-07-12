#include <iostream>
#include <string>
#include <time.h>

#include "TFile.h"
#include "TTree.h"
#include "TGraph.h"

int main(int argc, char* argv[])
{
  if(argc != 4){
    std::cout << "\tUsage: produceIVTtree scopeDataRootFile currentVoltageLog temperatureLog" << std::endl;
    return 1;
  }

  TFile* inFile = TFile::Open(argv[1]);
  UInt_t transfer;
  ULong64_t dateTime;
  TTree* preTree = (TTree*) inFile->Get("preamble");
  preTree->SetBranchAddress("transfer", &transfer);
  preTree->SetBranchAddress("dateTime", &dateTime);
  
  std::string inFileName(argv[1]);
  std::string outFileName = inFileName.substr(0, inFileName.size() - 5); // assume that the root file ends in .root
  outFileName.append("_IVTtree.root");
  TFile* outFile = TFile::Open(outFileName.c_str(), "RECREATE");
  TTree* ivtTree = new TTree("ivt", "Current voltage and temperature data");

  Float_t temperature;
  Float_t v1,i1,v2,i2;

  ivtTree->Branch("transfer", &transfer, "transfer/i");
  ivtTree->Branch("temperature", &temperature, "temperature/F");
  ivtTree->Branch("v1", &v1, "v1/F");
  ivtTree->Branch("i1", &i1, "i1/F");
  ivtTree->Branch("v2", &v2, "v2/F");
  ivtTree->Branch("i2", &i2, "i2/F");

  TGraph* tempGr = new TGraph(argv[3]);
  TGraph* v1Gr = new TGraph(argv[2], "%lg %lg %*lg %*lg %*lg");
  TGraph* i1Gr = new TGraph(argv[2], "%lg %*lg %lg %*lg %*lg");
  TGraph* v2Gr = new TGraph(argv[2], "%lg %*lg %*lg %lg %*lg");
  TGraph* i2Gr = new TGraph(argv[2], "%lg %*lg %*lg %*lg %lg");

  // there might be a difference between the oscilloscope time and the one of the DAQ PC and rasperry pi (used for the temperature and voltage logging)
  // PC and raspberry pi get the time from the internet, so are synchronized
  // the oscilloscope does not do this...
  // the difference between the two can be seen between the time of the start of the run of the PC in the run name and the first transfer of the oscilloscope
  std::string timeString = inFileName.substr(inFileName.size() - 28, 19); // get only the time part of the filename. assume that the filename has the form %Y-%m-%d_%H-%M-%S_run.root
  tm* timeStruct = new tm();
  strptime(timeString.c_str(), "%Y-%m-%d_%H-%M-%S", timeStruct);
  // next lines to get the right time stamp in GMT time
  time_t tmpTime = mktime(timeStruct); 
  timeStruct = gmtime(&tmpTime);
  time_t dateTimePC = mktime(timeStruct);
  preTree->GetEntry(0);
  int timeDiff = dateTimePC - dateTime;
  std::cout << "Time difference between DAQ and oscilloscope: " << timeDiff << " s" << std::endl;

  ULong64_t corrDateTime;  
  long int nEntries = preTree->GetEntries();
  for(long int i = 0; i < nEntries; ++i){
    preTree->GetEntry(i);
    corrDateTime = dateTime + timeDiff; // correct for difference between PC and scope clocks
    temperature = tempGr->Eval(corrDateTime);
    v1 = v1Gr->Eval(corrDateTime);
    i1 = i1Gr->Eval(corrDateTime);
    v2 = v2Gr->Eval(corrDateTime);
    i2 = i2Gr->Eval(corrDateTime);

    // std::cout << temperature << "  " << v1 << "  " << i1 << "  " << v2 << "  " << i2 << std::endl;
    
    ivtTree->Fill();    
  }

  ivtTree->BuildIndex("transfer"); // index the tree using the transfer number to synchronize it with the wavTree in analysis (tree friends)
  
  ivtTree->Write();
  outFile->Close();
  inFile->Close();
  
  return 0;
}
