#include "MeanStdDevHist.hh"
#include "AnalysisPrototype.hh"

MeanStdDevHist::MeanStdDevHist(const char* name, const char* title, float nbinsx, float xlow, float xup)
  : MedianHist(name, title, nbinsx, xlow, xup)
{
  _histStdDev = new TH1F(name, title, nbinsx, xlow, xup);
  SetHistNameTitle();
  
  return;
}

MeanStdDevHist::~MeanStdDevHist(){
  return;
}

void MeanStdDevHist::Process(){
  float mean;
  float stdDev;
  float eMean;
  float eStdDev;
  
  for(int iBin = 0; iBin < _histo->GetNcells(); ++iBin){
    AnalysisPrototype::CalcMeanStdDev(_binContent[iBin], mean, stdDev, eMean, eStdDev);
    _histo->SetBinContent(iBin, mean);
    _histo->SetBinError(iBin, eMean);
    _histStdDev->SetBinContent(iBin, stdDev);
    _histStdDev->SetBinError(iBin, eStdDev);
  }

  return;
}
  
void MeanStdDevHist::SetHistNameTitle(){
  TString name = _histo->GetName();
  name.ReplaceAll("median", "mean");
  _histo->SetName(name);

  TString title = _histo->GetTitle();
  title.ReplaceAll("Median", "Mean");
  _histo->SetTitle(title);

  title = _histo->GetYaxis()->GetTitle();
  TString add = "Mean ";
  _histo->GetYaxis()->SetTitle(add + title);
  
  name = _histStdDev->GetName();
  add = "stdDev";
  _histStdDev->SetName(add + name);

  title = _histStdDev->GetTitle();
  add = "Std Dev ";
  _histStdDev->SetTitle(add + title);

  title = _histStdDev->GetZaxis()->GetTitle();
  _histStdDev->GetYaxis()->SetTitle(add + title);

  
  return;
}
  
void MeanStdDevHist::Write(TDirectory* dir){
  dir->cd();
  _histo->Write();
  _histStdDev->Write();
  return;
}
