#ifndef MEDIANMAP_HH
#define MEDIANMAP_HH

#include "TH2F.h"

#include <vector>

class MedianMap
{
public:
  MedianMap(const char* name, const char* title, float nbinsx, float xlow, float xup, float nbinsy, float ylow, float yup); // same as for TH2
  ~MedianMap();
  void Fill(float x, float y, float val); // add a value to a vector
  virtual void Process(); // calculate the median for each bin
  virtual void Write(TDirectory* dir); // 

protected:
  std::vector<float>* _binContent;
  TH2F* _histo;
  virtual void SetHistNameTitle(); // adds "median" to name and title
};

#endif //#ifndef MEDIANMAP_HH
