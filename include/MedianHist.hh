#ifndef MEDIANHIST_HH
#define MEDIANHIST_HH

#include "TH1F.h"

#include <vector>

class MedianHist
{
public:
  MedianHist(const char* name, const char* title, float nbinsx, float xlow, float xup); // same as for TH1
  ~MedianHist();
  void Fill(float x, float val); // add a value to a vector
  virtual void Process(); // calculate the median for each bin
  void Write(TDirectory* dir); // 

protected:
  std::vector<float>* _binContent;
  TH1F* _histo;
  virtual void SetHistNameTitle(); // adds "median" to name and title
};

#endif //#ifndef MEDIANHIST_HH
