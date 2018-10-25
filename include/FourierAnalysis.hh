#ifndef FOURIERANALYSIS_HH
#define FOURIERANALYSIS_HH

/*
 * Class to perform and store results of Fourier analysis of signal
 */

#include "AnalysisPrototype.hh"

class AnalyzeScopeClass;

class FourierAnalysis : public AnalysisPrototype{
public:
  FourierAnalysis(AnalyzeScopeClass* acl, const char* dirName);
  ~FourierAnalysis();
  void AnalysisAction();
  void Process();
  void Save(TDirectory* parent);

private:
  // arrays containing the pointers to the histograms
  TH1F** _baselineRegion;
  TH1F** _signalRegion;

  long int evtCounter;
  
  void DoFFT(std::vector<float>& data, double dt, TH1F* hist); // do the FFT of data, that is spaced by dt between points, and put the magnitude in the histogram
  void ScaleBins(TH1F* hist, float scale); // multiply the bin content of each bin by scale

};

#endif //#ifndef FOURIERANALYSIS_HH
