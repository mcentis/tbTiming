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

  // utilities
  static void CalcMeanStdDev(const std::vector<float>& vec, float& mean, float& stdDev, float& Emean, float& EstdDev);
  static float CalcTimeThrLinear2pt(const std::vector<float>& tra, const std::vector<float>& tim, float thr, float offset); // all arguments (except time) need to be already corrected for polarity
  static void LinearReg(const std::vector<float>& x, const std::vector<float>& y, float& a, float& b); // linear regression y = ax + b
  static float Integrate(const std::vector<float>& tra, const std::vector<float>& tim, float start, float stop, float offset); // integrate between start and stop, all arguments needs to be already corrected for polarity
 protected:
  AnalyzeScopeClass* _acl;
  std::string _dirName;
  static int _totInstanceNumber; // number of the class instance, used to avoid naming problems in root (e.g. histos with same names etc)
  int _instanceNumber; // copy of the instance number, that should not change during the life of an instance, _totInstanceNumber can as other instances are created
      
  void PutAxisLabels(TGraph* gr, const char* xtitle, const char* ytitle);

};


#endif //#ifndef ANALYSISPROTOTYPE_H
