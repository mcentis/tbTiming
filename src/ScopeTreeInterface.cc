#include "ScopeTreeInterface.hh"

#include "TFile.h"

#include <iostream>

ScopeTreeInterface::ScopeTreeInterface(const char* fileName)
{
  _inFile = TFile::Open(fileName);
  if(_inFile == NULL){
    std::cout << "[Error] Could not open " << fileName << std::endl;
    exit(1);
  }

  _wavTree = (TTree*) _inFile->Get("waves");
  _preTree = (TTree*) _inFile->Get("preamble");

  _npt = 1e6;
  for(int i = 0; i < _inCh; ++i)
    _channels[i] = new Float_t[_npt];

  _time = new Float_t[_npt];

  _wavTree->SetBranchAddress("npt", &_npt);

  char name[100];
  for(int i = 0; i < _inCh; ++i){
    sprintf(name, "ch%d", i+1);
    _wavTree->SetBranchAddress(name, _channels[i]);
  }

  _wavTree->SetBranchAddress("time", _time);
  _wavTree->SetBranchAddress("transfer", &_transfer);
  _wavTree->SetBranchAddress("event", &_event);

  _preTree->SetBranchAddress("nSeg", &_nSeg);
  _preTree->SetBranchAddress("dateTime", &_dateTime);

  _wavTree->AddFriend(_preTree); // to read them together
  
  return;
}

ScopeTreeInterface::~ScopeTreeInterface()
{
    for(int i = 0; i < _inCh; ++i)
      delete[] _channels[i];

    delete[] _time;
  
  return;
}
