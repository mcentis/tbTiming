#include "MeanStdDevMap.hh"
#include "AnalysisPrototype.hh"

MeanStdDevMap::MeanStdDevMap(const char* name, const char* title, float nbinsx, float xlow, float xup, float nbinsy, float ylow, float yup)
  : MedianMap(name, title, nbinsx, xlow, xup, nbinsy, ylow, yup)
{
  _histStdDev = new TH2F(name, title, nbinsx, xlow, xup, nbinsy, ylow, yup);
  SetHistNameTitle();
  
  return;
}

MeanStdDevMap::~MeanStdDevMap(){
  return;
}

void MeanStdDevMap::Process(){
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
  
void MeanStdDevMap::SetHistNameTitle(){
  TString name = _histo->GetName();
  name.ReplaceAll("median", "mean");
  _histo->SetName(name);

  TString title = _histo->GetTitle();
  title.ReplaceAll("Median", "Mean");
  _histo->SetTitle(title);

  title = _histo->GetZaxis()->GetTitle();
  TString add = "Mean ";
  _histo->GetZaxis()->SetTitle(add + title);
  
  name = _histStdDev->GetName();
  add = "stdDev";
  _histStdDev->SetName(add + name);

  title = _histStdDev->GetTitle();
  add = "Std Dev ";
  _histStdDev->SetTitle(add + title);

  title = _histStdDev->GetZaxis()->GetTitle();
  _histStdDev->GetZaxis()->SetTitle(add + title);

  
  return;
}
  
void MeanStdDevMap::Write(TDirectory* dir){
  dir->cd();
  _histo->Write();
  _histStdDev->Write();
  return;
}
