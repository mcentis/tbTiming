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
  TH1F** _baselineDistr;
  TH1F** _noiseSingleEvtDistr;
  TH1F** _noiseDistr;
  TH1F** _ampliDistr;
  TH1F** _ampliTimeDistr;
  TH1F** _integralDistr;
  TH2I** _inteAmpli;
  TH1F** _riseTimeDistr;
  TH1I** _risePointsDistr;
  TH2I** _riseTimeAmpli;
  TH2I** _supSignal;
  TH2I** _supSignalScaled;
  TProfile** _profSignalScaled;
  // TGraph** _signalDerivative;
  // TGraph** _signalDerFrac;

};

#endif //#ifndef SIGNALPROPERTIES_HH
