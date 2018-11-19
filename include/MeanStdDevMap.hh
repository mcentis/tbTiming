#ifndef MEANSTDDEVMAP_HH
#define MEANSTDDEVMAP_HH

#include "MedianMap.hh"

class MeanStdDevMap : public MedianMap
{
public:
  MeanStdDevMap(const char* name, const char* title, float nbinsx, float xlow, float xup, float nbinsy, float ylow, float yup); // same as for TH2
  ~MeanStdDevMap();
  void Process(); // calculate mean and std dev for each bin
  void Write(TDirectory* dir);
  
private:
  TH2F* _histStdDev;
  void SetHistNameTitle();
};

#endif //#ifndef MEANSTDDEVMAP_HH
