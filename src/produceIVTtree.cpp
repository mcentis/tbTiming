#include <iostream>
#include <string>

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

  
  long int nEntries = preTree->GetEntries();
  for(long int i = 0; i < nEntries; ++i){
    preTree->GetEntry(i);
    temperature = tempGr->Eval(dateTime);
    v1 = v1Gr->Eval(dateTime);
    i1 = i1Gr->Eval(dateTime);
    v2 = v2Gr->Eval(dateTime);
    i2 = i2Gr->Eval(dateTime);

    // std::cout << temperature << "  " << v1 << "  " << i1 << "  " << v2 << "  " << i2 << std::endl;
    
    ivtTree->Fill();    
  }

  ivtTree->BuildIndex("transfer"); // index the tree using the transfer number to synchronize it with the wavTree in analysis (tree friends)
  
  ivtTree->Write();
  outFile->Close();
  inFile->Close();
  
  return 0;
}
