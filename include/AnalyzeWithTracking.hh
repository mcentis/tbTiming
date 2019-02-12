#ifndef ANALYZEWITHTRACKING_HH
#define ANALYZEWITHTRACKING_HH

#include "ConfigFileReader.hh"
#include "ScopeTreeInterface.hh"
#include "MedianMap.hh"
#include "MedianHist.hh"
#include "MeanStdDevMap.hh"
#include "MeanStdDevHist.hh"

#include "TH2F.h"
#include "TEfficiency.h"

class AnalyzeWithTracking{
public:
  static const int _nCh = ScopeTreeInterface::_inCh; // use the number of channels from the interface class
  
  AnalyzeWithTracking(const char* scopeAnaName, const char* trackDataName, const char* confFileName);
  ~AnalyzeWithTracking();
  void Analyze();
  void Save();

private:
  ConfigFileReader* _cfg;
  TFile* _pulsePropFile;
  TFile* _hitFile;
  TTree* _pulsePropTree;
  TTree* _hitTree;
  TFile* _outFile;

  void AssociateBranches();
  void InitializePlots();
  template<typename T> void ReadCfgArray(T* parameter, const char* key); // duplicate... check if possible to change
  void ReadTimingPairs(); // duplicate... check if possible to change
  void ReadCfg();
  
  // ============================= variables for the trees ==================
  
  ULong64_t _event; // event number, needed by the tree
  ULong64_t _eventCheck; // event number, saved in the hitTree, to check that the event is present in both trees
  UInt_t _nChTree; // number of channels as needed by the tree (cannot use _nCh directly)

  // pulse properties in tree
  Float_t* _ampli; // amplitudes
  Float_t* _ampliTime; // position of the signal maximum
  Float_t* _baseline; // baseline
  Float_t* _noise; // event noises
  Float_t* _riseTime; // event 20 to 80% risetime
  Float_t* _linRegT0; // event time used to "align" the events while using signal superimposition, could be used for additional cuts (e.g. ampli after t0), this variable is set to 10 if there are not enough points to determine the t0 using a linear regression
  Float_t* _integral; // integral of the signals, in C
  Float_t* _tCFD; // time when the CFD threshold from cfg file is crossed

  Int_t _nTracks; // number of tracks in the event, it is 0 if no hit is reconstructed
  Float_t _hits[500][_nCh][3]; // hit position on the DUT planes (one for each oscilloscope channel)
  Double_t _pars[500][4]; // parameters of the tracks
  Double_t _minFCN[500]; // minimum of the function minimized for track re-fitting, it is strongly correlated to recoTrackChi2, so a cut on the latter variable is sufficient
  Double_t _recoTrackChi2[500]; // track fit chi2 from the reconstruction SW of the beam telescope (not related to pars)
  Int_t _recoNdetsInTrack[500]; // number of planes used in the track reconstruction by telescope SW (2 for each GEM plane, x and y treated separately)
  
  // ============================= variables from conf file =====================

  float* _thr; // array with signal thresholds in V to select events in the analysis
  float* _maxAmpliCut; // array with maximum amplitude cut in V to select events in the analysis, to avoid saturation
  float* _xSliceLow; // array with lower limit of the cut in x
  float* _xSliceHigh; // array with higher limit of the cut in x
  float* _ySliceLow; // array with lower limit of the cut in y
  float* _ySliceHigh; // array with higher limit of the cut in y

  float _trackChi2cut; // maximum value of the chi2 of the tracks reconstructed by the telescope SW
  
  std::vector<std::string> _timingPairs; // pairs of channels for the timing
  int _nPairs; // number of channel pairs for the timing distribution
  int** _pairs; // array of pairs with channel numbers (starting from 0, program notation)

  float* _minRiseTimeCut; // array with minimum risetime cut in s, used for the dt distributions
  float* _maxRiseTimeCut; // array with maximum risetime cut in s, used for the dt distributions
  
  // ============================= plots ========================================
  
  TH2F** _hitMaps; // hitmaps on each plane
  TH2F** _hitMapsWithThr; // hitmaps on each plane with threshold and removed saturation
  TH2F** _ampliVsX; // amplitude as a function of the position in X, in slice selected by the cut in y
  TH2F** _ampliVsY; // amplitude as a function of the position in Y, in slice selected by the cut in x
  MedianHist** _medianAmpliVsX; // median amplitude as a function of the position in X, in slice selected by the cut in y, thresholds and saturation cuts area applied
  MedianHist** _medianAmpliVsY; // median amplitude as a function of the position in Y, in slice selected by the cut in x, thresholds and saturation cuts area applied
  TEfficiency** _effVsX; // efficiency in slices, uses the same treshold as for the selection for the timing, and removes saturation
  TEfficiency** _effVsY; // efficiency in slices, uses the same treshold as for the selection for the timing, and removes saturation
  TH2F** _riseTimeVsX; // risetime in slices, same cuts as the amplitude plots, plus thr and max amplitude cut
  TH2F** _riseTimeVsY; // risetime in slices, same cuts as the amplitude plots, plus thr and max amplitude cut
  MeanStdDevMap** _riseTimeMap; // risetime map, ampli cuts
  TH2F*** _dtCFDVsX; // delta t in slices, CFD, same cuts as the amplitude plots, plus thr and max amplitude cut, two plots for each timing pair, one for each channel
  TH2F*** _dtCFDVsY; // delta t in slices, CFD, same cuts as the amplitude plots, plus thr and max amplitude cut, two plots for each timing pair, one for each channel
  TH2F*** _dtLinReg0VsX; // delta t in slices, linear regression to 0, same cuts as the amplitude plots, plus thr and max amplitude cut, two plots for each timing pair, one for each channel
  TH2F*** _dtLinReg0VsY; // delta t in slices, linear regression to 0, same cuts as the amplitude plots, plus thr and max amplitude cut, two plots for each timing pair, one for each channel
  MeanStdDevHist*** _meanStdDevdtCFDVsX; // same as dtCFDVsX, mean and stdDev are extracted
  MeanStdDevHist*** _meanStdDevdtCFDVsY; // same as dtCFDVsY, mean and stdDev are extracted
  MedianMap** _ampliMap; // 2D median amplitude map, threshold is used, saturation is removed
  TEfficiency** _effMap; // 2D efficiency map, threshold as for timing, saturation is removed
  MeanStdDevMap*** _dtCFDMap; // 2D mean and std dev maps, threshold for timing and removed saturation
  TH1F** _ampliDistr; // amplitude distribution, geometrical cuts for each channel
  TH1F** _riseTimeDistr; // distribution of risetime for each ch, same selection used for dtCFD distribution, but applied to each ch separately
  TH1F** _dtCFDdistr; // distribution of dt for each timing pair, CFD, threshold, max ampli cut, both x and y slices cuts are used
  TH1F** _dtCFDdistrRiseCut; // distribution of dt for each timing pair, CFD, threshold, max ampli cut, both x and y slices cuts are used, also risetime cuts are used
};

#endif //#ifndef ANALYZEWITHTRACKING_HH
