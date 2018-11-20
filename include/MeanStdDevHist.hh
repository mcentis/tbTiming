#ifndef MEANSTDDEVHIST_HH
#define MEANSTDDEVHIST_HH

#include "MedianHist.hh"

class MeanStdDevHist : public MedianHist
{
public:
  MeanStdDevHist(const char* name, const char* title, float nbinsx, float xlow, float xup); // same as for TH1
  ~MeanStdDevHist();
  void Process(); // calculate mean and std dev for each bin
  void Write(TDirectory* dir);
  
private:
  TH1F* _histStdDev;
  void SetHistNameTitle();
};

#endif //#ifndef MEANSTDDEVHIST_HH
