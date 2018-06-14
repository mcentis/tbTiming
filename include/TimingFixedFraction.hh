#ifndef TIMINGFIXEDFRACTION_HH
#define TIMINGFIXEDFRACTION_HH

/*
 * Class to store time difference distributions obtained using a fixed CFD threshold
 */

#include "AnalysisPrototype.hh"

#include <vector>

class AnalyzeScopeClass;

class TimingFixedFraction : public AnalysisPrototype{
public:
  TimingFixedFraction(AnalyzeScopeClass* acl, const char* dirName);
  ~TimingFixedFraction();
  void AnalysisAction();
  //  void Process();
  void Save(TDirectory* parent);
  
private:
  int* GetPair(std::string pairstr); // interpret the strings containing the pairs
  
  int _nPairs; // number of channel pairs for the timing distribution
  int** _pairs; // array of pairs with channel numbers (starting from 0, program notation)

  // arrays containing the pointers to the histograms
  std::vector<TH1F*> _timeDiff;
  
};

#endif //#ifndef TIMINGFIXEDFRACTION_HH
