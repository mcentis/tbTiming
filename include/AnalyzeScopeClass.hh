#ifndef ANALYZESCOPECLASS_HH
#define ANALYZESCOPECLASS_HH

#include "ScopeTreeInterface.hh"
#include "ConfigFileReader.hh"

#include "TFile.h"

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
  
  ConfigFileReader* _cfg;

  TFile* _outFile;

  std::vector<AnalysisPrototype*> _analysisWithoutCuts; // analysis objects without cuts
  std::vector<AnalysisPrototype*> _analysisWCuts; // analysis objects with cuts
  
public: // made these public since they are needed by the analysis objects
  static const int _nCh = ScopeTreeInterface::_inCh; // use the number of channels from the interface class

  static void CalcMeanStdDev(std::vector<float> vec, float& mean, float& stdDev, float& Emean, float& EstdDev); // could be needed by the analysis objects
  
  ScopeTreeInterface* _scopeTreeInter;
  
  int* _pol; // array with the signal polarity for each channel
  float* _thr; // array with signal thresholds in V to select events in the analysis
  float* _constFrac; // array with CFD thresholds for ananlysis with fixed thresholds

  float* _blStart; // start of interval used to calculate baseline and noise
  float* _blStop; // stop of interval used to calculate baseline and noise

  float* _sigStart; // start of interval used to calculate signal properties: amplitude, risetime, etc, used to exclude multiple signals in the same event
  float* _sigStop;  // stop of interval used to calculate signal properties: amplitude, risetime, etc, used to exclude multiple signals in the same event

  float* _ampli; // amplitudes
  float* _ampliTime; // position of the signal maximum
  float* _baseline; // baseline
  float* _noise; // event noises

  // containers for baseline, signal points and signal time
  std::vector<float>* _blPoints;
  std::vector<float>* _sigPoints;
  std::vector<float>* _sigTime;
  
  
};

#endif //#ifndef ANALYZESCOPECLASS_HH
