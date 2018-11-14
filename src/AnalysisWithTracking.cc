#include "AnalysisWithTracking.hh"

#include <iostream>

#include "TFile.h"
#include "TTree.h"

AnalysisWithTracking::AnalysisWithTracking(const char* scopeAnaName, const char* trackDataName, const char* confFileName){

  _cfg = new ConfigFileReader(confFileName);
  _pulsePropFile = TFile::Open(scopeAnaName);
  _hitFile = TFile::Open(trackDataName);
  _pulsePropTree = (TTree*) _pulsePropFile->Get("evtPropTree");
  _hitTree = (TTree*) _hitFile->Get("hitTree");

  _pulsePropTree->AddFriend(_hitTree);

  AssociateBranches();
  
  // open output root file
  std::string outFileName = scopeAnaName;
  std::string rep = std::string("/"); // to be replaced
  std::string add = std::string("/trackInfO"); // replacement
  std::string::size_type i = outFileName.find_last_of(rep);
  
  if(i != std::string::npos)
    outFileName.replace(i, rep.length(), add);
  else
    outFileName.insert(0, add);
  
  _outFile = TFile::Open(outFileName.c_str(), "RECREATE");

  return;
}

AnalysisWithTracking::~AnalysisWithTracking(){
  _outFile->Close();

  delete[] _ampli;
  delete[] _ampliTime;
  delete[] _baseline;
  delete[] _noise;
  delete[] _riseTime;
  delete[] _linRegT0;
  delete[] _integral;
  delete[] _tCFD;
  delete[] _hits;
  delete[] _pars;
  
  return;
}

void AnalysisWithTracking::AssociateBranches(){

  _ampli = new Float_t[_nCh];
  _ampliTime = new Float_t[_nCh];
  _baseline = new Float_t[_nCh];
  _noise = new Float_t[_nCh];
  _riseTime = new Float_t[_nCh];
  _linRegT0 = new Float_t[_nCh];
  _integral = new Float_t[_nCh];
  _tCFD = new Float_t[_nCh];

  _nTracks = 500;
  _hits = new Float_t**[_nTracks];
  for(int i = 0; i < _nTracks; ++i){
    _hits[i] = new Float_t*[_nCh];
      for(int j = 0; j < _nCh; ++j)
	_hits[i][j] = new Float_t[3];
  }

  _pars = new Double_t[4];
  
  _pulsePropTree->SetBranchAddress("event", &_event);
  _pulsePropTree->SetBranchAddress("ampli", _ampli);
  _pulsePropTree->SetBranchAddress("ampliTime", _ampliTime);
  _pulsePropTree->SetBranchAddress("baseline", _baseline);
  _pulsePropTree->SetBranchAddress("noise", _noise);
  _pulsePropTree->SetBranchAddress("riseTime", _riseTime);
  _pulsePropTree->SetBranchAddress("integral", _integral);
  _pulsePropTree->SetBranchAddress("linRegT0", _linRegT0);
  _pulsePropTree->SetBranchAddress("tCFD", _tCFD);

  _hitTree->SetBranchAddress("nTracks", &_nTracks);
  _hitTree->SetBranchAddress("hits", _hits);
  _hitTree->SetBranchAddress("trackPar", _pars);

  return;
}

void AnalysisWithTracking::Analyze(){

  long int nEntries = _pulsePropTree->GetEntries();

  for(long int i = 0; i < nEntries; ++i){
    _pulsePropTree->GetEntry(i);

    if((i+1) % 1000 == 0 || (i+1) == nEntries)
      std::cout << " Processing event " << i+1 << " / " << nEntries << "                             \r" << std::flush;
    
  }
  
  return;
}

void AnalysisWithTracking::Save(){
  return;
}


