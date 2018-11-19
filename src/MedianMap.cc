#include "MedianMap.hh"
#include "AnalysisPrototype.hh"

#include "TString.h"

MedianMap::MedianMap(const char* name, const char* title, float nbinsx, float xlow, float xup, float nbinsy, float ylow, float yup){

  _histo = new TH2F(name, title, nbinsx, xlow, xup, nbinsy, ylow, yup);
  SetHistNameTitle();
  int ncells = _histo->GetNcells();
  _binContent = new std::vector<float>[ncells]; // one vector for each bin
  
  return;
}

MedianMap::~MedianMap(){
  delete[] _binContent;

  return;
}

void MedianMap::Fill(float x, float y, float val){
  int ibin = _histo->FindBin(x,y);
  _binContent[ibin].push_back(val);
  return;
}

void MedianMap::Process(){
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

void MedianMap::Write(TDirectory* dir){
  dir->cd();
  _histo->Write();
  return;
}

void MedianMap::SetHistNameTitle(){
  TString name = _histo->GetName();
  TString add = "median";
  _histo->SetName(add + name);

  TString title = _histo->GetTitle();
  add = "Median ";
  _histo->SetTitle(add + title);
  
  return;
}

