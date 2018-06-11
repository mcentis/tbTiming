#ifndef ANALYZESCOPECLASS_HH
#define ANALYZESCOPECLASS_HH

#include "ScopeTreeInterface.hh"
#include "ConfigFileReader.hh"

class AnalyzeScopeClass{
public:
  AnalyzeScopeClass(const char* inFileName, const char* confFileName);
  ~AnalyzeScopeClass();

private:
  void RootBeautySettings(); // style for plotting
  void GetCfgValues(); // extract values from cfg file
  template<typename T> void ReadCfgArray(T* parameter, const char* key); // read a string separated by , to assign the values to the parameters used in the analysis

  static const int _nCh = ScopeTreeInterface::_inCh; // use the number of channels from the interface class
    
  ScopeTreeInterface* _scopeTreeInter;
  ConfigFileReader* _cfg;

  int* _pol; // array with the signal polarity for each channel
  float* _thr; // array with signal thresholds in V to select events in the analysis
  float* _constFrac; // array with CFD thresholds for ananlysis with fixed thresholds

  float* _blStart; // start of interval used to calculate baseline and noise
  float* _blStop; // stop of interval used to calculate baseline and noise

  float* _sigStart; // start of interval used to calculate signal properties: amplitude, risetime, etc, used to exclude multiple signals in the same event
  float* _sigStop;  // stop of interval used to calculate signal properties: amplitude, risetime, etc, used to exclude multiple signals in the same event

  float* _ampli; // amplitudes
  float* _ampliTime; // position of the signal maximum
  float* _base; // baseline
  float* _noise; // event noises
  
};

#endif //#ifndef ANALYZESCOPECLASS_HH
