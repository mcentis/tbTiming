#ifndef ANALYSISWITHTRACKING_HH
#define ANALYSISWITHTRACKING_HH

#include "ConfigFileReader.hh"
#include "ScopeTreeInterface.hh"

class AnalysisWithTracking{
public:
  static const int _nCh = ScopeTreeInterface::_inCh; // use the number of channels from the interface class
  
  AnalysisWithTracking(const char* scopeAnaName, const char* trackDataName, const char* confFileName);
  ~AnalysisWithTracking();
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

  ULong64_t _event; // event number, needed by the tree
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
  Float_t*** _hits; // hit position on the DUT planes (one for each oscilloscope channel)
  Double_t* _pars; // parameters of the tracks

};

#endif //#ifndef ANALYSISWITHTRACKING_HH
