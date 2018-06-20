#ifndef THRESHOLDSTUDY_HH
#define THRESHOLDSTUDY_HH


/*
 * Class to study and determine the best CFD threshold
 */

#include "AnalysisPrototype.hh"

#include <vector>

class AnalyzeScopeClass;
class TH1F;
class TH2F;

class ThresholdStudy : public AnalysisPrototype{
public:
  ThresholdStudy(AnalyzeScopeClass* acl, const char* dirName);
  ~ThresholdStudy();
  void AnalysisAction();
  void Process();
  void Save(TDirectory* parent);

private:
  int _nPairs; // number of channel pairs for the timing distribution
  int** _pairs; // array of pairs with channel numbers (starting from 0, program notation)
  
  std::vector<TH2F*> _timeDiffMean; // histograms containing the mean time difference distribution for each timing pair
  std::vector<TH2F*> _timeDiffStdDev; // histograms containing the standard deviation of the time difference distribution for each timing pair
  std::vector<float> _cfdThr; // vector of thresholds used in the study (same for each timing pairs)
  std::vector<std::vector<float>**> _timeDiff; // one vector of time differences for each thr combination for each timing pair (3 dimesions needed: pair, thr1, thr2)
  std::vector<TH1F*> _bestTimeDiffDistr; // histograms of time difference distributions obtained using the best threshold combination, one for each timing pair
};

#endif//#ifndef THRESHOLDSTUDY_HH
