#include "AnalyzeWithTracking.hh"
#include "AnalyzeScopeClass.hh"
#include "TimingFixedFraction.hh"

#include <iostream>
#include <sstream>

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

  AnalyzeScopeClass::RootBeautySettings();
    
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
  _hitTree->SetBranchAddress("minFCN", _minFCN);
  _hitTree->SetBranchAddress("recoTrackChi2", _recoTrackChi2);
  _hitTree->SetBranchAddress("recoNdetsInTrack", _recoNdetsInTrack);

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

    if(_recoNdetsInTrack[_nTracks-1] != 6) // use only tracks where all GEM planes see a hit
      continue;

    if(_recoTrackChi2[_nTracks-1] > _trackChi2cut) // track chi square cut
      continue;

    for(int iCh = 0; iCh < _nCh; ++iCh){ // hitmaps
      _hitMaps[iCh]->Fill(_hits[_nTracks-1][iCh][0], _hits[_nTracks-1][iCh][1]);

      if(_ampli[iCh] > _thr[iCh] && _ampli[iCh] < _maxAmpliCut[iCh]){
      	_hitMapsWithThr[iCh]->Fill(_hits[_nTracks-1][iCh][0], _hits[_nTracks-1][iCh][1]); // hitmap
	_ampliMap[iCh]->Fill(_hits[_nTracks-1][iCh][0], _hits[_nTracks-1][iCh][1], _ampli[iCh]); // amplitude map
      }
    }

    for(int iCh = 0; iCh < _nCh; ++iCh){ // slices with amplitude
      if(_hits[_nTracks-1][iCh][1] > _ySliceLow[iCh] && _hits[_nTracks-1][iCh][1] < _ySliceHigh[iCh]){
	_ampliVsX[iCh]->Fill(_hits[_nTracks-1][iCh][0], _ampli[iCh]);
	if(_ampli[iCh] > _thr[iCh] && _ampli[iCh] < _maxAmpliCut[iCh]) // amplitude cut
	  _medianAmpliVsX[iCh]->Fill(_hits[_nTracks-1][iCh][0], _ampli[iCh]); // median slice
      }

      if(_hits[_nTracks-1][iCh][0] > _xSliceLow[iCh] && _hits[_nTracks-1][iCh][0] < _xSliceHigh[iCh]){
	_ampliVsY[iCh]->Fill(_hits[_nTracks-1][iCh][1], _ampli[iCh]);
	if(_ampli[iCh] > _thr[iCh] && _ampli[iCh] < _maxAmpliCut[iCh]) // amplitude cut
	  _medianAmpliVsY[iCh]->Fill(_hits[_nTracks-1][iCh][1], _ampli[iCh]);
      }
    }

    bool detected = false;
    for(int iCh = 0; iCh < _nCh; ++iCh){ // slices and maps with efficiency
      if(_ampli[iCh] > _maxAmpliCut[iCh]) // remove staturating events, to have same selection as for timing
	continue;

      if(_ampli[iCh] > _thr[iCh])
	detected = true;
      else
	detected = false;

      _effMap[iCh]->Fill(detected, _hits[_nTracks-1][iCh][0], _hits[_nTracks-1][iCh][1]);
      
      if(_hits[_nTracks-1][iCh][1] > _ySliceLow[iCh] && _hits[_nTracks-1][iCh][1] < _ySliceHigh[iCh])
	_effVsX[iCh]->Fill(detected, _hits[_nTracks-1][iCh][0]);

      if(_hits[_nTracks-1][iCh][0] > _xSliceLow[iCh] && _hits[_nTracks-1][iCh][0] < _xSliceHigh[iCh])
	_effVsY[iCh]->Fill(detected, _hits[_nTracks-1][iCh][1]);
    }

    for(int iCh = 0; iCh < _nCh; ++iCh) // slices with risetime
      if(_ampli[iCh] > _thr[iCh] && _ampli[iCh] < _maxAmpliCut[iCh]){

	_riseTimeMap[iCh]->Fill(_hits[_nTracks-1][iCh][0], _hits[_nTracks-1][iCh][1], _riseTime[iCh]);
	
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
	  _meanStdDevdtCFDVsX[iPair][j]->Fill(_hits[_nTracks-1][iCh][0], _tCFD[_pairs[iPair][0]] - _tCFD[_pairs[iPair][1]]);
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
	  _meanStdDevdtCFDVsY[iPair][j]->Fill(_hits[_nTracks-1][iCh][1], _tCFD[_pairs[iPair][0]] - _tCFD[_pairs[iPair][1]]);
	}
    }

    for(int iPair = 0; iPair < _nPairs; ++iPair){ // dt maps
      bool passes = true; // if true, fill histos for the pair
      for(int j = 0; j < 2; ++j){
	int iCh = _pairs[iPair][j];
	if(_ampli[iCh] > _thr[iCh] && _ampli[iCh] < _maxAmpliCut[iCh]) // ampli cut
	    continue; // the passes variable remains true if conditions are fulfilled by both channels
	passes = false;
      }
      if(passes)
	for(int j = 0; j < 2; ++j){
	  int iCh = _pairs[iPair][j];
	  _dtCFDMap[iPair][j]->Fill(_hits[_nTracks-1][iCh][0], _hits[_nTracks-1][iCh][1], _tCFD[_pairs[iPair][0]] - _tCFD[_pairs[iPair][1]]);
	}
    }

    for(int iCh = 0; iCh < _nCh; ++iCh) // risetime distribution with all cuts for amplitude, x, and y
      if(_ampli[iCh] > _thr[iCh] && _ampli[iCh] < _maxAmpliCut[iCh]) // ampli cut
	if(_hits[_nTracks-1][iCh][0] > _xSliceLow[iCh] && _hits[_nTracks-1][iCh][0] < _xSliceHigh[iCh]) // cut in x 
	  if(_hits[_nTracks-1][iCh][1] > _ySliceLow[iCh] && _hits[_nTracks-1][iCh][1] < _ySliceHigh[iCh]) // cut in y 
	    _riseTimeDistr[iCh]->Fill(_riseTime[iCh]);
	    
    for(int iPair = 0; iPair < _nPairs; ++iPair){ // dt distribution using all geometrical and ampli cuts for the pair
      bool passes = true; // if true, fill histo for the pair
      for(int j = 0; j < 2; ++j){
	int iCh = _pairs[iPair][j];
	if(_ampli[iCh] > _thr[iCh] && _ampli[iCh] < _maxAmpliCut[iCh]) // ampli cut
	  if(_hits[_nTracks-1][iCh][0] > _xSliceLow[iCh] && _hits[_nTracks-1][iCh][0] < _xSliceHigh[iCh]) // cut in x 
	    if(_hits[_nTracks-1][iCh][1] > _ySliceLow[iCh] && _hits[_nTracks-1][iCh][1] < _ySliceHigh[iCh]) // cut in y 
	      continue; // the passes variable remains true if all conditions are fulfilled
	passes = false;
      }
      if(passes)
	_dtCFDdistr[iPair]->Fill(_tCFD[_pairs[iPair][0]] - _tCFD[_pairs[iPair][1]]);
    }
    
    for(int iPair = 0; iPair < _nPairs; ++iPair){ // dt distribution using all geometrical and ampli cuts for the pair, and RISETIME cut
      bool passes = true; // if true, fill histo for the pair
      for(int j = 0; j < 2; ++j){
	int iCh = _pairs[iPair][j];
	if(_ampli[iCh] > _thr[iCh] && _ampli[iCh] < _maxAmpliCut[iCh]) // ampli cut
	  if(_riseTime[iCh] > _minRiseTimeCut[iCh] && _riseTime[iCh] < _maxRiseTimeCut[iCh]) // RISETIME cut
	    if(_hits[_nTracks-1][iCh][0] > _xSliceLow[iCh] && _hits[_nTracks-1][iCh][0] < _xSliceHigh[iCh]) // cut in x 
	      if(_hits[_nTracks-1][iCh][1] > _ySliceLow[iCh] && _hits[_nTracks-1][iCh][1] < _ySliceHigh[iCh]) // cut in y 
		continue; // the passes variable remains true if all conditions are fulfilled
	passes = false;
      }
      if(passes)
	_dtCFDdistrRiseCut[iPair]->Fill(_tCFD[_pairs[iPair][0]] - _tCFD[_pairs[iPair][1]]);
    }

  } // loop on the events
  
  std::cout << std::endl;

  for(int iCh = 0; iCh < _nCh; ++iCh){ // process objects in need
    _medianAmpliVsX[iCh]->Process();
    _medianAmpliVsY[iCh]->Process();
    _riseTimeMap[iCh]->Process();
    _ampliMap[iCh]->Process();
  }

  for(int i = 0; i < _nPairs; ++i) // process dt maps
    for(int j = 0; j < 2; ++j){
      _meanStdDevdtCFDVsX[i][j]->Process();
      _meanStdDevdtCFDVsY[i][j]->Process();
      _dtCFDMap[i][j]->Process();
    }
  
  return;
}

void AnalyzeWithTracking::InitializePlots(){
  char name [50];
  char title[200];

  _hitMaps = new TH2F*[_nCh];
  for(int iCh = 0; iCh < _nCh; ++iCh){
    sprintf(name, "hitMap_Ch%d", iCh+1);
    sprintf(title, "Hit Map Ch%d;X [mm];Y [mm];Entries", iCh+1);
    _hitMaps[iCh] = new TH2F(name, title, 200, 0, 100, 200, -50, 50);
  }

  _hitMapsWithThr = new TH2F*[_nCh];
  for(int iCh = 0; iCh < _nCh; ++iCh){
    sprintf(name, "hitMapWithThr_Ch%d", iCh+1);
    sprintf(title, "Hit Map Ch%d %.2f < A < %.2f V;X [mm];Y [mm];Entries", iCh+1, _thr[iCh], _maxAmpliCut[iCh]);
    _hitMapsWithThr[iCh] = new TH2F(name, title, 200, 0, 100, 200, -50, 50);
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

    _medianAmpliVsX = new MedianHist*[_nCh];
  for(int iCh = 0; iCh < _nCh; ++iCh){
    sprintf(name, "AmpliVsX_Ch%d", iCh+1);
    sprintf(title, "amplitude Ch%d vs X, %.2f < A < %.2f V, %.1f < Y < %.1f mm;X [mm];Median Amplitude [V]", iCh+1, _thr[iCh], _maxAmpliCut[iCh], _ySliceLow[iCh], _ySliceHigh[iCh]);
    _medianAmpliVsX[iCh] = new MedianHist(name, title, 500, 0, 100);
  }

  _medianAmpliVsY = new MedianHist*[_nCh];
  for(int iCh = 0; iCh < _nCh; ++iCh){
    sprintf(name, "AmpliVsY_Ch%d", iCh+1);
    sprintf(title, "amplitude Ch%d vs Y, %.2f < A < %.2f V, %.1f < X < %.1f mm;Y [mm];Median Amplitude [V]", iCh+1, _thr[iCh], _maxAmpliCut[iCh],  _xSliceLow[iCh], _xSliceHigh[iCh]);
    _medianAmpliVsY[iCh] = new MedianHist(name, title, 500, -50, 50);
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
    _riseTimeVsY[iCh] = new TH2F(name, title, 500, -50, 50, 500, 0, 10e-9);
  }

  _riseTimeMap = new MeanStdDevMap*[_nCh];
  for(int iCh = 0; iCh < _nCh; ++iCh){
    sprintf(name, "RiseTime_Ch%d", iCh+1);
    sprintf(title, "Risetime Ch%d, %.2f < A < %.2f V;X [mm];Y [mm];Risetime [s]", iCh+1, _thr[iCh], _maxAmpliCut[iCh]);
    _riseTimeMap[iCh] = new MeanStdDevMap(name, title, 200, 0, 100, 200, -50, 50);
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

  _meanStdDevdtCFDVsX = new MeanStdDevHist**[_nPairs];
  for(int i = 0; i < _nPairs; ++i){
    _meanStdDevdtCFDVsX[i] = new MeanStdDevHist*[2];
    for(int j = 0; j < 2; ++j){
      sprintf(name, "DtCFDVsX_Ch%d-%d_onCh%d", _pairs[i][0] + 1, _pairs[i][1] + 1, _pairs[i][j] + 1);
      sprintf(title, "#Delta t CFD Ch%d - Ch%d vs plane Ch%d X, y slices and amplitude cuts fulfilled for both Ch;X [mm];#Delta t [s]", _pairs[i][0]+1, _pairs[i][1]+1, _pairs[i][j]+1);
      _meanStdDevdtCFDVsX[i][j] = new MeanStdDevHist(name, title, 500, 0, 100);
    }
  }

  _meanStdDevdtCFDVsY = new MeanStdDevHist**[_nPairs];
  for(int i = 0; i < _nPairs; ++i){
    _meanStdDevdtCFDVsY[i] = new MeanStdDevHist*[2];
    for(int j = 0; j < 2; ++j){
      sprintf(name, "DtCFDVsY_Ch%d-%d_onCh%d", _pairs[i][0] + 1, _pairs[i][1] + 1, _pairs[i][j] + 1);
      sprintf(title, "#Delta t CFD Ch%d - Ch%d vs plane Ch%d Y, x slices and amplitude cuts fulfilled for both Ch;Y [mm];#Delta t [s]", _pairs[i][0]+1, _pairs[i][1]+1, _pairs[i][j]+1);
      _meanStdDevdtCFDVsY[i][j] = new MeanStdDevHist(name, title, 500, 0, 100);
    }
  }
  
  _ampliMap = new MedianMap*[_nCh];
  for(int iCh = 0; iCh < _nCh; ++iCh){
    sprintf(name, "AmpliMap_Ch%d", iCh+1);
    sprintf(title, "amplitude Map Ch%d, %.2f < A < %.2f V;X [mm];Y [mm];Median Amplitude [V]", iCh+1, _thr[iCh], _maxAmpliCut[iCh]);
    _ampliMap[iCh] = new MedianMap(name, title, 200, 0, 100, 200, -50, 50);
  }
  
  _effMap = new TEfficiency*[_nCh];
  for(int iCh = 0; iCh < _nCh; ++iCh){
    sprintf(name, "effMap_Ch%d", iCh+1);
    sprintf(title, "Efficiency Map Ch%d, THR %d mV, A < %.2f V;X [mm];Y [mm];Efficiency", iCh+1, (int) (_thr[iCh] * 1000), _maxAmpliCut[iCh]);
    _effMap[iCh] = new TEfficiency(name, title, 500, 0, 100, 500, -50, 50);
  }

  _dtCFDMap = new MeanStdDevMap**[_nPairs];
  for(int i = 0; i < _nPairs; ++i){
    _dtCFDMap[i] = new MeanStdDevMap*[2];
    for(int j = 0; j < 2; ++j){
      sprintf(name, "DtCFDMap_Ch%d-%d_onCh%d", _pairs[i][0] + 1, _pairs[i][1] + 1, _pairs[i][j] + 1);
      sprintf(title, "#Delta t CFD Ch%d - Ch%d plane Ch%d ampli cut for both Ch;X [mm];Y [mm];#Delta t [s]", _pairs[i][0]+1, _pairs[i][1]+1, _pairs[i][j]+1);
      _dtCFDMap[i][j] = new MeanStdDevMap(name, title, 200, 0, 100, 200, -50, 50);
    }
  }

  _riseTimeDistr = new TH1F*[_nCh];
  for(int iCh = 0; iCh < _nCh; ++iCh){
    sprintf(name, "riseTimeDistr_Ch%d", iCh+1);
    sprintf(title, "Risetime Ch%d, x and y slices and amplitude cuts fulfilled;Risetime [s];Entries", iCh+1);
    _riseTimeDistr[iCh] = new TH1F(name, title, 400, 0, 3e-9);
  }

  _dtCFDdistr = new TH1F*[_nPairs];
  for(int i = 0; i < _nPairs; ++i){
      sprintf(name, "dtCFDdistr_Ch%d-%d", _pairs[i][0] + 1, _pairs[i][1] + 1);
      sprintf(title, "#Delta t CFD Ch%d - Ch%d, x and y slices and amplitude cuts fulfilled for both Ch;#Delta t [s];Entries", _pairs[i][0]+1, _pairs[i][1]+1);
      _dtCFDdistr[i] = new TH1F(name, title, 1500, -10e-9, 10e-9);
  }

  _dtCFDdistrRiseCut = new TH1F*[_nPairs];
  for(int i = 0; i < _nPairs; ++i){
      sprintf(name, "dtCFDdistrRiseCut_Ch%d-%d", _pairs[i][0] + 1, _pairs[i][1] + 1);
      sprintf(title, "#Delta t CFD Ch%d - Ch%d, x and y slices, amplitude, and RISETIME cuts fulfilled for both Ch;#Delta t [s];Entries", _pairs[i][0]+1, _pairs[i][1]+1);
      _dtCFDdistrRiseCut[i] = new TH1F(name, title, 1500, -10e-9, 10e-9);
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

  dir = _outFile->mkdir("medianAmpliSlices");
  dir->cd();
  for(int iCh = 0; iCh < _nCh; ++iCh){
    _medianAmpliVsX[iCh]->Write(dir);
    _medianAmpliVsY[iCh]->Write(dir);    
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

  dir = _outFile->mkdir("riseTimeMaps");
  dir->cd();
  for(int iCh = 0; iCh < _nCh; ++iCh)
    _riseTimeMap[iCh]->Write(dir);

  dir = _outFile->mkdir("dtCFDSlices");
  dir->cd();
  for(int i = 0; i < _nPairs; ++i)
    for(int j = 0; j < 2; ++j){
      _dtCFDVsX[i][j]->Write();
      _dtCFDVsY[i][j]->Write();
    }

  dir = _outFile->mkdir("meanStdDevdtCFDSlices");
  dir->cd();
  for(int i = 0; i < _nPairs; ++i)
    for(int j = 0; j < 2; ++j){
      _meanStdDevdtCFDVsX[i][j]->Write(dir);
      _meanStdDevdtCFDVsY[i][j]->Write(dir);
    }
  
  dir = _outFile->mkdir("dtLinReg0Slices");
  dir->cd();
  for(int i = 0; i < _nPairs; ++i)
    for(int j = 0; j < 2; ++j){
      _dtLinReg0VsX[i][j]->Write();
      _dtLinReg0VsY[i][j]->Write();
    }

  dir = _outFile->mkdir("amplitudeMaps");
  dir->cd();
  for(int iCh = 0; iCh < _nCh; ++iCh)
    _ampliMap[iCh]->Write(dir);
  
  dir = _outFile->mkdir("efficinecyMaps");
  dir->cd();
  for(int iCh = 0; iCh < _nCh; ++iCh)
    _effMap[iCh]->Write();

  dir = _outFile->mkdir("dtCFDMaps");
  dir->cd();
  for(int i = 0; i < _nPairs; ++i)
    for(int j = 0; j < 2; ++j)
      _dtCFDMap[i][j]->Write(dir);

  dir = _outFile->mkdir("riseTimeDistr");
  dir->cd();
  for(int iCh = 0; iCh < _nCh; ++iCh)
      _riseTimeDistr[iCh]->Write();

  dir = _outFile->mkdir("dtCFDdistr");
  dir->cd();
  for(int i = 0; i < _nPairs; ++i)
      _dtCFDdistr[i]->Write();

  dir = _outFile->mkdir("dtCFDdistrRiseCut");
  dir->cd();
  for(int i = 0; i < _nPairs; ++i)
      _dtCFDdistrRiseCut[i]->Write();

  return;
}

void AnalyzeWithTracking::ReadCfg(){
  _thr = new float[_nCh];
  _maxAmpliCut = new float[_nCh];

  _xSliceLow = new float[_nCh];
  _xSliceHigh = new float[_nCh];
  _ySliceLow = new float[_nCh];
  _ySliceHigh = new float[_nCh];

  _minRiseTimeCut = new float[_nCh];
  _maxRiseTimeCut = new float[_nCh];
  
  ReadCfgArray(_thr, "threshold");
  ReadCfgArray(_maxAmpliCut, "maxAmpli");

  ReadCfgArray(_xSliceLow, "xSliceLow");
  ReadCfgArray(_xSliceHigh, "xSliceHigh");
  ReadCfgArray(_ySliceLow, "ySliceLow");
  ReadCfgArray(_ySliceHigh, "ySliceHigh");

  _trackChi2cut = atof(_cfg->GetValue("trackMaxChi2").c_str());
  
  ReadTimingPairs();
  _nPairs = _timingPairs.size();
  _pairs = new int*[_nPairs];
  for(int i = 0; i < _nPairs; ++i)
    _pairs[i] = TimingFixedFraction::GetPair(_timingPairs[i], _nCh);

  ReadCfgArray(_minRiseTimeCut, "minRiseTime");
  ReadCfgArray(_maxRiseTimeCut, "maxRiseTime");
  
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


