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
  virtual void Process();
  
 protected:
  AnalyzeScopeClass* _acl;
  std::string _dirName;
  static int _instanceNumber; // number of the class instance, used to avoid naming problems in root (e.g. histos with same names etc)
  
  float CalcTimeThrLinear2pt(const std::vector<float>& tra, const std::vector<float>& tim, float thr, float offset); // all agruments (except time) need to be already corrected for polarity
  void LinearReg(const std::vector<float>& x, const std::vector<float>& y, float& a, float& b); // linear regression
    
  void PutAxisLabels(TGraph* gr, const char* xtitle, const char* ytitle);

};


#endif //#ifndef ANALYSISPROTOTYPE_H
