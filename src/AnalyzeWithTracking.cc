#include "AnalyzeWithTracking.hh"

#include <iostream>

#include "TFile.h"
#include "TTree.h"

AnalyzeWithTracking::AnalyzeWithTracking(const char* scopeAnaName, const char* trackDataName, const char* confFileName){

  _cfg = new ConfigFileReader(confFileName);
  _pulsePropFile = TFile::Open(scopeAnaName);
  _hitFile = TFile::Open(trackDataName);
  _pulsePropTree = (TTree*) _pulsePropFile->Get("evtPropTree");
  _hitTree = (TTree*) _hitFile->Get("hitTree");

  _pulsePropTree->AddFriend(_hitTree);

  AssociateBranches();

  ReadCfg();
  
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

  InitializePlots();
  
  return;
}

AnalyzeWithTracking::~AnalyzeWithTracking(){
  _outFile->Close();

  delete[] _ampli;
  delete[] _ampliTime;
  delete[] _baseline;
  delete[] _noise;
  delete[] _riseTime;
  delete[] _linRegT0;
  delete[] _integral;
  delete[] _tCFD;
  delete[] _pars;

  delete[] _thr;
  delete[] _maxAmpliCut;
  
  return;
}

void AnalyzeWithTracking::AssociateBranches(){

  _ampli = new Float_t[_nCh];
  _ampliTime = new Float_t[_nCh];
  _baseline = new Float_t[_nCh];
  _noise = new Float_t[_nCh];
  _riseTime = new Float_t[_nCh];
  _linRegT0 = new Float_t[_nCh];
  _integral = new Float_t[_nCh];
  _tCFD = new Float_t[_nCh];

  _nTracks = 500;
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

void AnalyzeWithTracking::Analyze(){

  long int nEntries = _pulsePropTree->GetEntries();

  for(long int i = 0; i < nEntries; ++i){
    _pulsePropTree->GetEntry(i);

    if((i+1) % 1000 == 0 || (i+1) == nEntries)
      std::cout << " Processing event " << i+1 << " / " << nEntries << "                             \r" << std::flush;
    
    if(_nTracks != 1) // use only events with one track
      continue;
    
    for(int iCh = 0; iCh < _nCh; ++iCh){ // hitmaps
      _hitMaps[iCh]->Fill(_hits[_nTracks-1][iCh][0], _hits[_nTracks-1][iCh][1]);

      if(_ampli[iCh] >= _thr[iCh])
      	_hitMapsWithThr[iCh]->Fill(_hits[_nTracks-1][iCh][0], _hits[_nTracks-1][iCh][1]);
    }

    for(int iCh = 0; iCh < _nCh; ++iCh){
      if(_hits[_nTracks-1][iCh][1] >= _ySliceLow[iCh] && _hits[_nTracks-1][iCh][1] <= _ySliceHigh[iCh])
	_ampliVsX[iCh]->Fill(_hits[_nTracks-1][iCh][0], _ampli[iCh]);

      if(_hits[_nTracks-1][iCh][0] >= _xSliceLow[iCh] && _hits[_nTracks-1][iCh][0] <= _xSliceHigh[iCh])
	_ampliVsY[iCh]->Fill(_hits[_nTracks-1][iCh][1], _ampli[iCh]);
    }
    
  }

  std::cout << std::endl;
  
  return;
}

void AnalyzeWithTracking::InitializePlots(){
  char name [50];
  char title[200];

  _hitMaps = new TH2F*[_nCh];
  for(int iCh = 0; iCh < _nCh; ++iCh){
    sprintf(name, "hitMap_Ch%d", iCh+1);
    sprintf(title, "Hit Map Ch%d;X [mm];Y [mm];Entries", iCh+1);
    _hitMaps[iCh] = new TH2F(name, title, 500, 0, 100, 500, -50, 50);
  }

  _hitMapsWithThr = new TH2F*[_nCh];
  for(int iCh = 0; iCh < _nCh; ++iCh){
    sprintf(name, "hitMapWithThr_Ch%d", iCh+1);
    sprintf(title, "Hit Map Ch%d threshold %d mV;X [mm];Y [mm];Entries", iCh+1, (int) (_thr[iCh] * 1000));
    _hitMapsWithThr[iCh] = new TH2F(name, title, 500, 0, 100, 500, -50, 50);
  }

  _ampliVsX = new TH2F*[_nCh];
  for(int iCh = 0; iCh < _nCh; ++iCh){
    sprintf(name, "ampliVsX_Ch%d", iCh+1);
    sprintf(title, "Amplitude Ch%d;X [mm];Amplitude [V];Entries", iCh+1);
    _ampliVsX[iCh] = new TH2F(name, title, 500, 0, 100, 200, 0, 1);
  }

  _ampliVsY = new TH2F*[_nCh];
  for(int iCh = 0; iCh < _nCh; ++iCh){
    sprintf(name, "ampliVsY_Ch%d", iCh+1);
    sprintf(title, "Amplitude Ch%d;Y [mm];Amplitude [V];Entries", iCh+1);
    _ampliVsY[iCh] = new TH2F(name, title, 500, -50, 50, 200, 0, 1);
  }

  return;
}

void AnalyzeWithTracking::Save(){
  _outFile->cd();

  TDirectory* dir = _outFile->mkdir("hitMaps");
  dir->cd();
  for(int iCh = 0; iCh < _nCh; ++iCh)
    _hitMaps[iCh]->Write();

  dir = _outFile->mkdir("hitMapsWithThr");
  dir->cd();
  for(int iCh = 0; iCh < _nCh; ++iCh)
    _hitMapsWithThr[iCh]->Write();

  dir = _outFile->mkdir("amplitudeSlices");
  dir->cd();
  for(int iCh = 0; iCh < _nCh; ++iCh){
    _ampliVsX[iCh]->Write();
    _ampliVsY[iCh]->Write();
  }
  
  return;
}

void AnalyzeWithTracking::ReadCfg(){
  _thr = new float[_nCh];
  _maxAmpliCut = new float[_nCh];

  _xSliceLow = new float[_nCh];
  _xSliceHigh = new float[_nCh];
  _ySliceLow = new float[_nCh];
  _ySliceHigh = new float[_nCh];
  
  ReadCfgArray(_thr, "threshold");
  ReadCfgArray(_maxAmpliCut, "maxAmpli");

  ReadCfgArray(_xSliceLow, "xSliceLow");
  ReadCfgArray(_xSliceHigh, "xSliceHigh");
  ReadCfgArray(_ySliceLow, "ySliceLow");
  ReadCfgArray(_ySliceHigh, "ySliceHigh");
  
  return;
}

// void AnalyzeWithTracking::ReadTimingPairs(){ // duplicate... check if possible to change
//   std::string valStr = _cfg->GetValue("timingPairs");
//   std::stringstream strstr(valStr);
//   std::string sub;
  
//   while(strstr.good()){
//     getline(strstr, sub, ',');
//     _timingPairs.push_back(sub);
//   }

//   //for(std::vector<std::string>::iterator it = _timingPairs.begin(); it != _timingPairs.end(); ++it)
//   //  std::cout << *it << std::endl;
  
//   return;
// }

template<typename T> void AnalyzeWithTracking::ReadCfgArray(T* parameter, const char* key){ // duplicate... check if possible to change
  std::string valStr = _cfg->GetValue(key);
  std::stringstream strstr(valStr);
  std::string sub;
  
  for(int i = 0; i < _nCh; ++i){
    if(strstr.good() == false){
      std::cout << "Error while reading config file for parameter " << key << std::endl;
      exit(1);
    }
    getline(strstr, sub, ',');
    parameter[i] = atof(sub.c_str());
  }
  
  return;
}


