#include "AnalyzeScopeClass.hh"

#include <string>
#include <sstream>
#include <iostream>

#include "TStyle.h"

AnalyzeScopeClass::AnalyzeScopeClass(const char* inFileName, const char* confFileName){

  _scopeTreeInter = new ScopeTreeInterface(inFileName);
  _cfg = new ConfigFileReader(confFileName);

  // allocate memory for variables
  _pol = new int[_nCh];
  _thr = new float[_nCh];
  _constFrac = new float[_nCh];
  
  _ampli = new float[_nCh];
  _ampliTime = new float[_nCh];
  _base = new float[_nCh];
  _noise = new float[_nCh];
  
  _blStart = new float[_nCh];
  _blStop = new float[_nCh];
  
  _sigStart = new float[_nCh];
  _sigStop = new float[_nCh];

  GetCfgValues();
  
  return;
}

AnalyzeScopeClass::~AnalyzeScopeClass(){
  delete[] _pol;
  delete[] _thr;
  delete[] _constFrac;
  delete[] _ampli;
  delete[] _ampliTime;
  delete[] _base;
  delete[] _noise;
  delete[] _blStart;
  delete[] _blStop;
  delete[] _sigStart;
  delete[] _sigStop;
  
  return;
}

void AnalyzeScopeClass::GetCfgValues(){
  //_cfg->DumpConfMap();

  ReadCfgArray(_pol, "polarity");
  for(int i = 0; i < _nCh; ++i) // make sure that polarity is normalized
    _pol[i] /= _pol[i]; 
  
  ReadCfgArray(_thr, "threshold");
  ReadCfgArray(_constFrac, "fractionThr");
  ReadCfgArray(_blStart, "baseStart");
  ReadCfgArray(_blStop, "baseStop");
  ReadCfgArray(_sigStart, "signalStart");
  ReadCfgArray(_sigStop, "signalStop");

  return;
}

template<typename T> void AnalyzeScopeClass::ReadCfgArray(T* parameter, const char* key){
  std::string valStr = _cfg->GetValue(key);
  std::stringstream strstr(valStr);
  std::string sub;
  
  for(int i = 0; i < _nCh; ++i){
    if(strstr.good() == false){
      std::cout << "Error while reading config file for parameter " << key << std::endl;
      exit(1);
    }
    getline(strstr, sub, ',');
    parameter[i] = atof(sub.c_str());
  }
  
  return;
}

void AnalyzeScopeClass::RootBeautySettings(){
  gStyle->SetOptFit();
  gStyle->SetOptStat(111110);
  //gStyle->SetOptFit(0);
  //gStyle->SetOptStat(0);
  //gStyle->SetOptTitle(0);

  // big axis labels
  gStyle->SetLabelSize(0.05, "x");
  gStyle->SetLabelSize(0.05, "y");
  gStyle->SetLabelSize(0.05, "z");

  gStyle->SetTitleSize(0.05, "x");
  gStyle->SetTitleSize(0.05, "y");
  gStyle->SetTitleSize(0.05, "z");

  gStyle->SetTitleOffset(0.9, "x");
  // gStyle->SetTitleOffset(0.95, "x");
  gStyle->SetTitleOffset(0.95, "y");

  gStyle->SetHistFillColor(kBlue);
  gStyle->SetHistFillStyle(3005);

  gStyle->SetMarkerStyle(20);  
  gStyle->SetMarkerSize(2);
  gStyle->SetLineWidth(2);

  return;
}
