#ifndef SIGNALPROPERTIES_HH
#define SIGNALPROPERTIES_HH

/*
 * Class to store pulse properties
 */

#include "AnalysisPrototype.hh"

#include "TH2.h"
#include "TProfile.h"

class AnalyzeScopeClass;

class SignalProperties : public AnalysisPrototype{
public:
  SignalProperties(AnalyzeScopeClass* acl, const char* dirName);
  ~SignalProperties();
  void AnalysisAction();
  void Process();
  void Save(TDirectory* parent);

private:
  // arrays containing the pointers to the histograms
  TH1F* _baselineDistr[AnalyzeScopeClass::_nCh];
  TH1F* _noiseSingleEvtDistr[AnalyzeScopeClass::_nCh];
  TH1F* _noiseDistr[AnalyzeScopeClass::_nCh];
  TH1F* _ampliDistr[AnalyzeScopeClass::_nCh];
  TH1F* _ampliTimeDistr[AnalyzeScopeClass::_nCh];
  TH1F* _riseTimeDistr[AnalyzeScopeClass::_nCh];
  TH2I* _supSignal[AnalyzeScopeClass::_nCh];
  TH2I* _supSignalScaled[AnalyzeScopeClass::_nCh];
  TProfile* _profSignalScaled[AnalyzeScopeClass::_nCh];
  TGraph* _signalDerivative[AnalyzeScopeClass::_nCh];
  TGraph* _signalDerFrac[AnalyzeScopeClass::_nCh];

};

#endif //#ifndef SIGNALPROPERTIES_HH
