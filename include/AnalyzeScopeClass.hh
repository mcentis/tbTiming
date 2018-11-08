#ifndef ANALYZESCOPECLASS_HH
#define ANALYZESCOPECLASS_HH

#include "ScopeTreeInterface.hh"
#include "ConfigFileReader.hh"

#include "TFile.h"
#include "TTree.h"

class AnalysisPrototype;

class AnalyzeScopeClass{
public:
  AnalyzeScopeClass(const char* inFileName, const char* confFileName);
  ~AnalyzeScopeClass();
  void Analyze(); // loop on the events
  void Save(); // save stuff, necessary to call this function otherwise there will be a core dump... to be investigated
  
private:
  void RootBeautySettings(); // style for plotting
  void GetCfgValues(); // extract values from cfg file
  template<typename T> void ReadCfgArray(T* parameter, const char* key); // read a string separated by , to assign the values to the parameters used in the analysis

  bool ProcessEvent(); // cuts are put in tis function, false is returned if the event does not fullfill the cuts

  void CalcBaselineNoise(); // calculate baseline and noise for an event
  void CalcAmpliTime(); // calculate signal amplitude and time at which the maximum occours
  void SelectPoints(); // fills the vectors containing the waveform points of signal and baseline
  void SelectPointsFromPeak(); // fills the vectors containing the waveform points of signal and baseline, using interval relative to peak
  void CalcRiseTimeT0(); // calculate 20 to 80% rise time and t0 using linear regression of rising edge between 20 and 80 %
  void CalcIntegral(); // integrates the signal, needs the baseline, selected points, and t0
  void CalcTcfd(); // calculation of the CFD threhsold crossing, thresholds from cfg file
  
  ConfigFileReader* _cfg;

  TFile* _outFile;
  TTree* _evtPropTree; // tree with the event properties: amplitude, integral, time of arrival etc...
  ULong64_t _event; // event number, needed by the tree
  UInt_t _nChTree; // number of channels as needed by the tree (cannot use _nCh directly)
  
  std::vector<AnalysisPrototype*> _analysisWithoutCuts; // analysis objects without cuts
  std::vector<AnalysisPrototype*> _analysisWCuts; // analysis objects with cuts
  
public: // made these public since they are needed by the analysis objects
  static const int _nCh = ScopeTreeInterface::_inCh; // use the number of channels from the interface class
  
  ScopeTreeInterface* _scopeTreeInter;
  
  int* _pol; // array with the signal polarity for each channel
  float* _thr; // array with signal thresholds in V to select events in the analysis
  float* _maxAmpliCut; // array with maximum amplitude cut in V to select events in the analysis, to avoid saturation
  float* _minRiseTimeCut; // array with minimum risetime cut in s to select events in the analysis, to study signal properties
  float* _maxRiseTimeCut; // array with maximum risetime cut in s to select events in the analysis, to reject noise
  float* _constFrac; // array with CFD thresholds for ananlysis with fixed thresholds
  float* _ToT; // array with time over threshold to be used in the CFD, riseTime, and rising edge interpolation

  float* _blStart; // start of interval used to calculate baseline and noise
  float* _blStop; // stop of interval used to calculate baseline and noise

  float* _sigStart; // start of interval used to calculate signal properties: amplitude, risetime, etc, used to exclude multiple signals in the same event
  float* _sigStop;  // stop of interval used to calculate signal properties: amplitude, risetime, etc, used to exclude multiple signals in the same event

  float* _termination; // oscilloscope termination, in Ohms

  float* _inteStart; // start time for integration, relative to the _linRegT0
  float* _inteStop; // stop time for integration, relative to the _linRegT0

  // variables for fine selection of the signal region based on the position of the peak of the signal
  // the intervals will be : blDuration, prePeak, (peaking point determined in analysis), postPeak
  float* _blDuration; // baseline interval duration
  float* _prePeak; // interval preceding peak
  float* _postPeak; // interval following the peak
  
  unsigned long int _minEvent; // minimum event number
  unsigned long int _maxEvent; // maximum event number
  
  std::vector<std::string> _timingPairs; // pairs of channels for the timing study
  void ReadTimingPairs(); // function to read the pairs from the cfg file
  
  float* _ampli; // amplitudes
  float* _ampliTime; // position of the signal maximum
  unsigned int* _ampliTimeIndex; // position of the signal maximum in the vectors
  float* _baseline; // baseline
  float* _noise; // event noises
  float* _riseTime; // event 20 to 80% risetime
  int* _risePoints; // number of points in the rising edge between 20 and 80%
  float* _linRegT0; // event time used to "align" the events while using signal superimposition, could be used for additional cuts (e.g. ampli after t0), this variable is set to 10 if there are not enough points to determine the t0 using a linear regression
  float* _integral; // integral of the signals, in C
  float* _tCFD; // time when the CFD threshold from cfg file is crossed
  
  // containers for baseline, signal points and signal time
  std::vector<float>* _blPoints;
  std::vector<float>* _sigPoints;
  std::vector<float>* _sigTime;
  //std::vector<float>* _leadPoints;
  //std::vector<float>* _leadTime;
  
  
};

#endif //#ifndef ANALYZESCOPECLASS_HH
