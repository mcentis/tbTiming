#include "AnalysisPrototype.hh"

#include <iostream>

#include "TCanvas.h"
#include "TAxis.h"

// initialize the instance counter, -1 to have counter start from 0
int AnalysisPrototype::_instanceNumber = -1;

AnalysisPrototype::AnalysisPrototype(AnalyzeScopeClass* acl, const char* dirName)
{
  _acl = acl;
  _dirName = std::string(dirName);

  _instanceNumber++;
  
  return;
}

AnalysisPrototype::~AnalysisPrototype()
{
  return;
}

void AnalysisPrototype::AnalysisAction()
{
  std::cout << "Implement AnalysisAction" << std::endl;
  
  return;
}

void AnalysisPrototype::Save(TDirectory* parent)
{
  std::cout << "Implement Save" << std::endl;
  
  return;
}

// void AnalysisPrototype::Process()
// {
//  return;
// }

// void AnalysisPrototype::NewSet()
// {
//  return;
// }

float AnalysisPrototype::CalcTimeThrLinear2pt(Float_t* tra, Float_t* tim, Int_t n, int pol, float thr, float offset)
{
  int i = 0;
  for(; i < n; ++i)
    if(pol * tra[i] - offset > thr) break;

  // linear interpolation between points below and above thr
  // y = a x + b
  float y1 = pol * tra[i] - offset;
  float y2 = pol * tra[i - 1] - offset;
  float x1 = tim[i];
  float x2 = tim[i - 1];

  float a = (y2-y1)/(x2-x1);
  float b = y1 -a*x1;

  return (thr - b)/a;
}

void AnalysisPrototype::PutAxisLabels(TGraph* gr, const char* xtitle, const char* ytitle)
{
  TCanvas* can = new TCanvas();
  gr->Draw("apl");
  gr->GetXaxis()->SetTitle(xtitle);
  gr->GetYaxis()->SetTitle(ytitle);
  delete can;
  
  return;
}
