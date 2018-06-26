#include "AnalysisPrototype.hh"

#include <iostream>

#include "TCanvas.h"
#include "TAxis.h"

// initialize the instance counter, -1 to have counter start from 0
int AnalysisPrototype::_totInstanceNumber = -1;

AnalysisPrototype::AnalysisPrototype(AnalyzeScopeClass* acl, const char* dirName)
{
  _acl = acl;
  _dirName = std::string(dirName);

  _totInstanceNumber++;
  _instanceNumber = _totInstanceNumber;
  
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

void AnalysisPrototype::Process()
{
 return;
}

float AnalysisPrototype::CalcTimeThrLinear2pt(const std::vector<float>& tra, const std::vector<float>& tim, float thr, float offset)
{
  unsigned int i = 0;
  for(; i < tra.size(); ++i)
    if(tra[i] - offset > thr) break;

  if(i == 0) // intercept i == 0 to avoid problems below
    return tim[0];

  // linear interpolation between points below and above thr
  // y = a x + b
  float y1 = tra[i] - offset;
  float y2 = tra[i - 1] - offset;
  float x1 = tim[i];
  float x2 = tim[i - 1];

  float a = (y2-y1)/(x2-x1);
  float b = y1 -a*x1;

  return (thr - b)/a;
}

void AnalysisPrototype::LinearReg(const std::vector<float>& x, const std::vector<float>& y, float& a, float& b)
{
  if(x.size() != y.size() || x.size() < 2 || y.size() < 2){
    a = -999;
    b = -999;
    std::cout << "[Warning] AnalysisPrototype::LinearReg Problem with the input vectors" << std::endl;
    return;
  }
  
  // calculate linear regression y = a x + b
  float xmean = 0; // mean x
  float ymean = 0; // mean y
  float x2mean = 0; // mean x*x
  float xymean = 0; // mean x*y

  std::vector<float>::const_iterator itx = x.begin();
  std::vector<float>::const_iterator ity = y.begin();
  for(; itx != x.end() && ity != y.end(); ++itx, ++ity){
    xmean += *itx;
    ymean += *ity;
    x2mean += *itx * *itx;
    xymean += *itx * *ity;

    //std::cout << *itx << "   " << *ity << std::endl;
  }

  xmean /= x.size();
  ymean /= x.size();
  x2mean /= x.size();
  xymean /= x.size();

  a = (xymean - xmean * ymean) / (x2mean - xmean * xmean);
  b = (ymean * x2mean - xymean * xmean) / (x2mean - xmean * xmean);

  //std::cout << x.size() << std::endl;
  //std::cout << xmean << "   " << ymean << "   " << x2mean << "   " << xymean << std::endl;  
  //std::cout << a << "   " << b << std::endl;
  
  return;
}

void AnalysisPrototype::CalcMeanStdDev(const std::vector<float>& vec, float& mean, float& stdDev, float& Emean, float& EstdDev)
{
  if(vec.size() == 0){
    std::cout << "[Warning] AnalyzeScopeClass::CalcMeanStdDev: Too few entries to calculate anything." << std::endl;
    mean = 0;
    stdDev = 0;
    Emean = 0;
    EstdDev = 0;
    return;
  }

  int N = 0;
  float sum = 0;
  float sumD2 = 0;
  float sumD4 = 0;
  
  for(std::vector<float>::const_iterator it = vec.begin(); it != vec.end(); ++it){
    if(*it != *it) // protect from nan
      continue;
    
    sum += *it;
    N++;
  }
  
  if(N == 0){
    std::cout << "[Warning] AnalyzeScopeClass::CalcMeanStdDev: Too few entries to calculate anything due to NAN." << std::endl;
    mean = 0;
    stdDev = 0;
    Emean = 0;
    EstdDev = 0;
    return;
  }

  mean = sum / N; // mean

  if(N < 4){ // to avoid strange results in error calculation
    std::cout << "[Warning] AnalyzeScopeClass::CalcMeanStdDev: Too few entries to calculate uncertainties. Mean " << mean << std::endl;
    stdDev = 0;
    Emean = 0;
    EstdDev = 0;
    return;
  }

  
  for(std::vector<float>::const_iterator it = vec.begin(); it != vec.end(); ++it){
    if(*it != *it) // protect from nan
      continue;

    sumD2 += pow(*it - mean, 2);
    sumD4 += pow(*it - mean, 4);
  }

  float mu2 = sumD2 / (N - 1); // second central moment (variance)
  
  stdDev = sqrt(mu2); // std dev

  Emean = sqrt(mu2/N);

  float mu4 = ( pow(N, 2)*sumD4/(N-1) - 3*(2*N-3)*pow(mu2, 2) ) / (pow(N, 2)-3*N+3); // fourth central moment from http://mathworld.wolfram.com/SampleCentralMoment.html

  float Emu2 = sqrt((pow(N-1, 2) * mu4 - (N-1) * (N-3) * pow(mu2, 2)) / pow(N, 3)); // std dev of mu2 distribution from http://mathworld.wolfram.com/SampleVarianceDistribution.html
    
  EstdDev = 0.5 * Emu2 / sqrt(mu2);

  return;
}

float AnalysisPrototype::Integrate(const std::vector<float>& tra, const std::vector<float>& tim, float start, float stop, float offset)
{
  float integral = 0;
  float dt;

  std::vector<float>::const_iterator itVolt = tra.begin();
  std::vector<float>::const_iterator itTime = tim.begin();
  for(; itVolt != tra.end() && itTime != tim.end(); ++itVolt, ++itTime)
    if(*itTime >= start && *itTime <= stop){
      dt = *itTime - *(itTime - 1);
      integral += 0.5 * dt * (*(itVolt - 1) + *itVolt - 2 * offset);
    }

  return integral;
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
