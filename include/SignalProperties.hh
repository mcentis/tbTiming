#ifndef SIGNALPROPERTIES_HH
#define SIGNALPROPERTIES_HH

#include "AnalysisPrototype.hh"

/*
 * Class to store pulse properties
 */

class AnalyzeScopeClass;

class SignalProperties : public AnalysisPrototype{
public:
  SignalProperties(AnalyzeScopeClass* acl, const char* dirName);
  ~SignalProperties();
  void AnalysisAction();
  void Save(TDirectory* parent);

private:
  // arrays containing the pointers to the histograms
  TH1F* _baselineDistr[AnalyzeScopeClass::_nCh];
  TH1F* _noiseSingleEvtDistr[AnalyzeScopeClass::_nCh];
  TH1F* _noiseDistr[AnalyzeScopeClass::_nCh];
  TH1F* _ampliDistr[AnalyzeScopeClass::_nCh];
  TH1F* _ampliTimeDistr[AnalyzeScopeClass::_nCh];
  TH1F* _riseTimeDistr[AnalyzeScopeClass::_nCh];

};

#endif //#ifndef SIGNALPROPERTIES_HH
