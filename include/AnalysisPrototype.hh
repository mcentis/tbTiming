#ifndef ANALYSISPROTOTYPE_H
#define ANALYSISPROTOTYPE_H

#include "AnalyzeScopeClass.hh"

#include "string"

#include "TGraph.h"

/*
 * Base analysis class from which the analysis objects inherit 
 * This class provides the interface to handle the analysis objects from the AnalyzeScopeClass etc.
 */

class AnalysisPrototype
{
 public:
  AnalysisPrototype(AnalyzeScopeClass* acl, const char* dirName);
  virtual ~AnalysisPrototype();
  virtual void AnalysisAction();
  virtual void Save(TDirectory* parent);
  // virtual void Process();
  //  virtual void NewSet();
  
 protected:
  AnalyzeScopeClass* _acl;
  std::string _dirName;
  static int _instanceNumber; // number of the instance, used to avoid naming problems in root (e.g. histos with same names etc)
  
  float CalcTimeThrLinear2pt(Float_t* tra, Float_t* tim, Int_t n, int pol, float thr, float offset); // thr and offset must have already the right polarity
  
  void PutAxisLabels(TGraph* gr, const char* xtitle, const char* ytitle);

};


#endif //#ifndef ANALYSISPROTOTYPE_H
