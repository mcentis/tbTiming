#include "AnalyzeWithTracking.hh"

#include "TimingFixedFraction.hh"

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

    for(int iCh = 0; iCh < _nCh; ++iCh){ // slices with amplitude
      if(_hits[_nTracks-1][iCh][1] > _ySliceLow[iCh] && _hits[_nTracks-1][iCh][1] < _ySliceHigh[iCh])
	_ampliVsX[iCh]->Fill(_hits[_nTracks-1][iCh][0], _ampli[iCh]);

      if(_hits[_nTracks-1][iCh][0] > _xSliceLow[iCh] && _hits[_nTracks-1][iCh][0] < _xSliceHigh[iCh])
	_ampliVsY[iCh]->Fill(_hits[_nTracks-1][iCh][1], _ampli[iCh]);
    }

    bool detected = false;
    for(int iCh = 0; iCh < _nCh; ++iCh){ // slices with efficiency
      if(_ampli[iCh] > _maxAmpliCut[iCh]) // remove staturating events, to have same selection as for timing
	continue;

      if(_ampli[iCh] > _thr[iCh])
	detected = true;
      else
	detected = false;
      
      if(_hits[_nTracks-1][iCh][1] > _ySliceLow[iCh] && _hits[_nTracks-1][iCh][1] < _ySliceHigh[iCh])
	_effVsX[iCh]->Fill(detected, _hits[_nTracks-1][iCh][0]);

      if(_hits[_nTracks-1][iCh][0] > _xSliceLow[iCh] && _hits[_nTracks-1][iCh][0] < _xSliceHigh[iCh])
	_effVsY[iCh]->Fill(detected, _hits[_nTracks-1][iCh][1]);
    }

    for(int iCh = 0; iCh < _nCh; ++iCh) // slices with risetime
      if(_ampli[iCh] > _thr[iCh] && _ampli[iCh] < _maxAmpliCut[iCh]){
	if(_hits[_nTracks-1][iCh][1] > _ySliceLow[iCh] && _hits[_nTracks-1][iCh][1] < _ySliceHigh[iCh])
	  _riseTimeVsX[iCh]->Fill(_hits[_nTracks-1][iCh][0], _riseTime[iCh]);
	
	if(_hits[_nTracks-1][iCh][0] > _xSliceLow[iCh] && _hits[_nTracks-1][iCh][0] < _xSliceHigh[iCh])
	  _riseTimeVsY[iCh]->Fill(_hits[_nTracks-1][iCh][1], _riseTime[iCh]);
      }

    for(int iPair = 0; iPair < _nPairs; ++iPair){ // X coordinate of timing pairs slice
      bool passes = true; // if true, fill histo for the pair
      for(int j = 0; j < 2; ++j){
	int iCh = _pairs[iPair][j];
	if(_ampli[iCh] > _thr[iCh] && _ampli[iCh] < _maxAmpliCut[iCh]) // ampli cut
	  if(_hits[_nTracks-1][iCh][1] > _ySliceLow[iCh] && _hits[_nTracks-1][iCh][1] < _ySliceHigh[iCh]) // cut in y (for x plots)
	    continue; // the passes variable remains true if both conditions are fulfilled
	passes = false;
      }
      if(passes)
	for(int j = 0; j < 2; ++j){
	  int iCh = _pairs[iPair][j];
	  _dtCFDVsX[iPair][j]->Fill(_hits[_nTracks-1][iCh][0], _tCFD[_pairs[iPair][0]] - _tCFD[_pairs[iPair][1]]);
	  _dtLinReg0VsX[iPair][j]->Fill(_hits[_nTracks-1][iCh][0], _linRegT0[_pairs[iPair][0]] - _linRegT0[_pairs[iPair][1]]);
	}
    }

    for(int iPair = 0; iPair < _nPairs; ++iPair){ // Y coordinate of timing pairs slice
      bool passes = true; // if true, fill histo for the pair
      for(int j = 0; j < 2; ++j){
	int iCh = _pairs[iPair][j];
	if(_ampli[iCh] > _thr[iCh] && _ampli[iCh] < _maxAmpliCut[iCh]) // ampli cut
	  if(_hits[_nTracks-1][iCh][0] > _xSliceLow[iCh] && _hits[_nTracks-1][iCh][0] < _xSliceHigh[iCh]) // cut in x (for y plots)
	    continue; // the passes variable remains true if both conditions are fulfilled
	passes = false;
      }
      if(passes)
	for(int j = 0; j < 2; ++j){
	  int iCh = _pairs[iPair][j];
	  _dtCFDVsY[iPair][j]->Fill(_hits[_nTracks-1][iCh][1], _tCFD[_pairs[iPair][0]] - _tCFD[_pairs[iPair][1]]);
	  _dtLinReg0VsY[iPair][j]->Fill(_hits[_nTracks-1][iCh][1], _linRegT0[_pairs[iPair][0]] - _linRegT0[_pairs[iPair][1]]);
	}
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
    sprintf(title, "Amplitude Ch%d vs X, %.1f < Y < %.1f mm;X [mm];Amplitude [V];Entries", iCh+1, _ySliceLow[iCh], _ySliceHigh[iCh]);
    _ampliVsX[iCh] = new TH2F(name, title, 500, 0, 100, 200, 0, 1);
  }

  _ampliVsY = new TH2F*[_nCh];
  for(int iCh = 0; iCh < _nCh; ++iCh){
    sprintf(name, "ampliVsY_Ch%d", iCh+1);
    sprintf(title, "Amplitude Ch%d vs Y, %.1f < X < %.1f mm;Y [mm];Amplitude [V];Entries", iCh+1, _xSliceLow[iCh], _xSliceHigh[iCh]);
    _ampliVsY[iCh] = new TH2F(name, title, 500, -50, 50, 200, 0, 1);
  }

  _effVsX = new TEfficiency*[_nCh];
  for(int iCh = 0; iCh < _nCh; ++iCh){
    sprintf(name, "effVsX_Ch%d", iCh+1);
    sprintf(title, "Efficiency Ch%d vs X, THR %d mV, A < %.2f V, %.1f < Y < %.1f mm;X [mm];Efficiency", iCh+1, (int) (_thr[iCh] * 1000), _maxAmpliCut[iCh], _ySliceLow[iCh], _ySliceHigh[iCh]);
    _effVsX[iCh] = new TEfficiency(name, title, 500, 0, 100);
  }

  _effVsY = new TEfficiency*[_nCh];
  for(int iCh = 0; iCh < _nCh; ++iCh){
    sprintf(name, "effVsY_Ch%d", iCh+1);
    sprintf(title, "Efficiency Ch%d vs Y, THR %d mV, A < %.2f V, %.1f < X < %.1f mm;Y [mm];Efficiency", iCh+1, (int) (_thr[iCh] * 1000), _maxAmpliCut[iCh], _xSliceLow[iCh], _xSliceHigh[iCh]);
    _effVsY[iCh] = new TEfficiency(name, title, 500, -50, 50);
  }

  _riseTimeVsX = new TH2F*[_nCh];
  for(int iCh = 0; iCh < _nCh; ++iCh){
    sprintf(name, "riseTimeVsX_Ch%d", iCh+1);
    sprintf(title, "Risetime Ch%d vs X, %.1f < Y < %.1f mm, %.2f < A < %.2f V;X [mm];Risetime [s];Entries", iCh+1, _ySliceLow[iCh], _ySliceHigh[iCh], _thr[iCh], _maxAmpliCut[iCh]);
    _riseTimeVsX[iCh] = new TH2F(name, title, 500, 0, 100, 500, 0, 10e-9);
  }

  _riseTimeVsY = new TH2F*[_nCh];
  for(int iCh = 0; iCh < _nCh; ++iCh){
    sprintf(name, "riseTimeVsY_Ch%d", iCh+1);
    sprintf(title, "Risetime Ch%d vs Y, %.1f < X < %.1f mm, %.2f < A < %.2f V;Y [mm];Risetime [s];Entries", iCh+1, _xSliceLow[iCh], _xSliceHigh[iCh], _thr[iCh], _maxAmpliCut[iCh]);
    _riseTimeVsY[iCh] = new TH2F(name, title, 500, 0, 100, 500, 0, 10e-9);
  }

  _dtCFDVsX = new TH2F**[_nPairs];
  for(int i = 0; i < _nPairs; ++i){
    _dtCFDVsX[i] = new TH2F*[2];
    for(int j = 0; j < 2; ++j){
      sprintf(name, "dtCFDVsX_Ch%d-%d_onCh%d", _pairs[i][0] + 1, _pairs[i][1] + 1, _pairs[i][j] + 1);
      sprintf(title, "#Delta t CFD Ch%d - Ch%d vs plane Ch%d X, y slices and amplitude cuts fulfilled for both Ch;X [mm];#Delta t [s];Entries", _pairs[i][0]+1, _pairs[i][1]+1, _pairs[i][j]+1);
      _dtCFDVsX[i][j] = new TH2F(name, title, 500, 0, 100, 500, -10e-9, 10e-9);
    }
  }

  _dtCFDVsY = new TH2F**[_nPairs];
  for(int i = 0; i < _nPairs; ++i){
    _dtCFDVsY[i] = new TH2F*[2];
    for(int j = 0; j < 2; ++j){
      sprintf(name, "dtCFDVsY_Ch%d-%d_onCh%d", _pairs[i][0] + 1, _pairs[i][1] + 1, _pairs[i][j] + 1);
      sprintf(title, "#Delta t CFD Ch%d - Ch%d vs plane Ch%d Y, x slices and amplitude cuts fulfilled for both Ch;Y [mm];#Delta t [s];Entries", _pairs[i][0]+1, _pairs[i][1]+1, _pairs[i][j]+1);
      _dtCFDVsY[i][j] = new TH2F(name, title, 500, 0, 100, 500, -10e-9, 10e-9);
    }
  }
  
  _dtLinReg0VsX = new TH2F**[_nPairs];
  for(int i = 0; i < _nPairs; ++i){
    _dtLinReg0VsX[i] = new TH2F*[2];
    for(int j = 0; j < 2; ++j){
      sprintf(name, "dtLinReg0VsX_Ch%d-%d_onCh%d", _pairs[i][0] + 1, _pairs[i][1] + 1, _pairs[i][j] + 1);
      sprintf(title, "#Delta t LinReg0 Ch%d - Ch%d vs plane Ch%d X, y slices and amplitude cuts fulfilled for both Ch;X [mm];#Delta t [s];Entries", _pairs[i][0]+1, _pairs[i][1]+1, _pairs[i][j]+1);
      _dtLinReg0VsX[i][j] = new TH2F(name, title, 500, 0, 100, 500, -10e-9, 10e-9);
    }
  }

  _dtLinReg0VsY = new TH2F**[_nPairs];
  for(int i = 0; i < _nPairs; ++i){
    _dtLinReg0VsY[i] = new TH2F*[2];
    for(int j = 0; j < 2; ++j){
      sprintf(name, "dtLinReg0VsY_Ch%d-%d_onCh%d", _pairs[i][0] + 1, _pairs[i][1] + 1, _pairs[i][j] + 1);
      sprintf(title, "#Delta t LinReg0 Ch%d - Ch%d vs plane Ch%d Y, x slices and amplitude cuts fulfilled for both Ch;Y [mm];#Delta t [s];Entries", _pairs[i][0]+1, _pairs[i][1]+1, _pairs[i][j]+1);
      _dtLinReg0VsY[i][j] = new TH2F(name, title, 500, 0, 100, 500, -10e-9, 10e-9);
    }
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

  dir = _outFile->mkdir("efficiencySlices");
  dir->cd();
  for(int iCh = 0; iCh < _nCh; ++iCh){
    _effVsX[iCh]->Write();
    _effVsY[iCh]->Write();
  }
  
  dir = _outFile->mkdir("riseTimeSlices");
  dir->cd();
  for(int iCh = 0; iCh < _nCh; ++iCh){
    _riseTimeVsX[iCh]->Write();
    _riseTimeVsY[iCh]->Write();
  }

  dir = _outFile->mkdir("dtCFDSlices");
  dir->cd();
  for(int i = 0; i < _nPairs; ++i)
    for(int j = 0; j < 2; ++j){
      _dtCFDVsX[i][j]->Write();
      _dtCFDVsY[i][j]->Write();
    }

  dir = _outFile->mkdir("dtLinReg0Slices");
  dir->cd();
  for(int i = 0; i < _nPairs; ++i)
    for(int j = 0; j < 2; ++j){
      _dtLinReg0VsX[i][j]->Write();
      _dtLinReg0VsY[i][j]->Write();
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

  ReadTimingPairs();
  _nPairs = _timingPairs.size();
  _pairs = new int*[_nPairs];
  for(int i = 0; i < _nPairs; ++i)
    _pairs[i] = TimingFixedFraction::GetPair(_timingPairs[i], _nCh);
  
  return;
}

void AnalyzeWithTracking::ReadTimingPairs(){ // duplicate... check if possible to change
  std::string valStr = _cfg->GetValue("timingPairs");
  std::stringstream strstr(valStr);
  std::string sub;
  
  while(strstr.good()){
    getline(strstr, sub, ',');
    _timingPairs.push_back(sub);
  }

  // for(std::vector<std::string>::iterator it = _timingPairs.begin(); it != _timingPairs.end(); ++it)
  //  std::cout << *it << std::endl;
  
  return;
}

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


