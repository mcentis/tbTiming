#include "MedianHist.hh"
#include "AnalysisPrototype.hh"

#include "TString.h"

MedianHist::MedianHist(const char* name, const char* title, float nbinsx, float xlow, float xup){

  _histo = new TH1F(name, title, nbinsx, xlow, xup);
  SetHistNameTitle();
  int ncells = _histo->GetNcells();
  _binContent = new std::vector<float>[ncells]; // one vector for each bin
  
  return;
}

MedianHist::~MedianHist(){
  delete[] _binContent;

  return;
}

void MedianHist::Fill(float x, float val){
  int ibin = _histo->FindBin(x);
  _binContent[ibin].push_back(val);
  return;
}

void MedianHist::Process(){
  float median;
  float eMedianHigh;
  float eMedianLow;

  for(int iBin = 0; iBin < _histo->GetNcells(); ++iBin){
    AnalysisPrototype::CalcMedian(_binContent[iBin], median, eMedianLow, eMedianHigh);
    _histo->SetBinContent(iBin, median);
    _histo->SetBinError(iBin, (eMedianLow+eMedianHigh)/2); // mean of the two errors is used
  }
  
  return;
}

void MedianHist::Write(TDirectory* dir){
  dir->cd();
  _histo->Write();
  return;
}

void MedianHist::SetHistNameTitle(){
  TString name = _histo->GetName();
  TString add = "median";
  _histo->SetName(add + name);

  TString title = _histo->GetTitle();
  add = "Median ";
  _histo->SetTitle(add + title);
  
  return;
}

