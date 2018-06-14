#include "AnalyzeScopeClass.hh"
#include "AnalysisPrototype.hh"
#include "SignalProperties.hh"

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
  _maxAmpliCut = new float[_nCh];
  _constFrac = new float[_nCh];
  
  _ampli = new float[_nCh];
  _ampliTime = new float[_nCh];
  _baseline = new float[_nCh];
  _noise = new float[_nCh];
  
  _blStart = new float[_nCh];
  _blStop = new float[_nCh];
  
  _sigStart = new float[_nCh];
  _sigStop = new float[_nCh];

  _blPoints = new std::vector<float>[_nCh];
  _sigPoints = new std::vector<float>[_nCh];
  _sigTime = new std::vector<float>[_nCh];
  
  GetCfgValues();
  
  // open root file
  std::string outFileName = inFileName;
  std::string rep = std::string("/"); // to be replaced
  std::string add = std::string("/analyzed"); // replacement
  std::string::size_type i = outFileName.find_last_of(rep);
  
  if(i != std::string::npos)
    outFileName.replace(i, rep.length(), add);
  else
    outFileName.insert(0, add);
  
  _outFile = TFile::Open(outFileName.c_str(), "RECREATE");
  
  // create analysis objects without cuts
  _analysisWithoutCuts.push_back(new SignalProperties(this, "SignalPropertiesNoCuts"));

  // create analysis objects with cuts
  _analysisWCuts.push_back(new SignalProperties(this, "SignalProperties"));

  return;
}

AnalyzeScopeClass::~AnalyzeScopeClass(){
  _outFile->Close();

  delete[] _pol;
  delete[] _thr;
  delete[] _maxAmpliCut;
  delete[] _constFrac;
  delete[] _ampli;
  delete[] _ampliTime;
  delete[] _baseline;
  delete[] _noise;
  delete[] _blStart;
  delete[] _blStop;
  delete[] _sigStart;
  delete[] _sigStop;

  // delete analysis without cuts objects
  for(std::vector<AnalysisPrototype*>::iterator it = _analysisWithoutCuts.begin(); it != _analysisWithoutCuts.end(); it++)
    delete *it;

  // delete analysis with cuts objects
  for(std::vector<AnalysisPrototype*>::iterator it = _analysisWCuts.begin(); it != _analysisWCuts.end(); it++)
    delete *it;

  return;
}

void AnalyzeScopeClass::GetCfgValues(){
  //_cfg->DumpConfMap();

  ReadCfgArray(_pol, "polarity");
  for(int i = 0; i < _nCh; ++i) // make sure that polarity is normalized
    _pol[i] /= fabs(_pol[i]); 
  
  ReadCfgArray(_thr, "threshold");
  ReadCfgArray(_maxAmpliCut, "maxAmpli");
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

void AnalyzeScopeClass::Analyze(){
  unsigned long int nEntries = _scopeTreeInter->_wavTree->GetEntries();

  for(unsigned long int i = 0; i < nEntries; ++i){
    _scopeTreeInter->_wavTree->GetEntry(i);

    if((i+1) % 1000 == 0 || (i+1) == nEntries)
      std::cout << " Processing event " << i+1 << " / " << nEntries << "                             \r" << std::flush;

    // select signal and baseline regions and apply polarity
    SelectPoints();
    
    // calculate pulses properties
    CalcBaselineNoise();
    CalcAmpliTime();

    // analysis without selection
    for(std::vector<AnalysisPrototype*>::iterator it = _analysisWithoutCuts.begin(); it != _analysisWithoutCuts.end(); it++)
      (*it)->AnalysisAction();
    
    // apply cuts
    if(ProcessEvent() == false)
      continue;

    // analysis on selected events
    for(std::vector<AnalysisPrototype*>::iterator it = _analysisWCuts.begin(); it != _analysisWCuts.end(); it++)
      (*it)->AnalysisAction();
    
  }// loop on the events

  std::cout << std::endl;

  std::cout << "Processing analysis objects" << std::endl;
  
  // process analysis without selection
  for(std::vector<AnalysisPrototype*>::iterator it = _analysisWithoutCuts.begin(); it != _analysisWithoutCuts.end(); it++)
    (*it)->Process();

  // processa nalysis on selected events
  for(std::vector<AnalysisPrototype*>::iterator it = _analysisWCuts.begin(); it != _analysisWCuts.end(); it++)
    (*it)->Process();
  
  return;
}

void AnalyzeScopeClass::Save(){
  // analysis without cuts
  for(std::vector<AnalysisPrototype*>::iterator it = _analysisWithoutCuts.begin(); it != _analysisWithoutCuts.end(); it++)
    (*it)->Save(_outFile);

  // analysis with cuts
  for(std::vector<AnalysisPrototype*>::iterator it = _analysisWCuts.begin(); it != _analysisWCuts.end(); it++)
    (*it)->Save(_outFile);

  return;
}

bool AnalyzeScopeClass::ProcessEvent(){
  bool ret = true;

  for(int iCh = 0; iCh < _nCh; ++iCh){ // amplitude cut, threshold and maximum amplitude
    ret = ret && _ampli[iCh] >= _thr[iCh];
    ret = ret && _ampli[iCh] <= _maxAmpliCut[iCh];
  }

  return ret;
}

void AnalyzeScopeClass::SelectPoints(){
  for(int iCh = 0; iCh < _nCh; ++iCh){ // empty the vectors
    _blPoints[iCh].clear();
    _sigPoints[iCh].clear();
    _sigTime[iCh].clear();
  }

  for(int iCh = 0; iCh < _nCh; ++iCh){ // fill baseline vectors
    for(unsigned int ipt = 0; ipt < _scopeTreeInter->_npt; ++ipt){ // select the points for baseline calculation
      if(_scopeTreeInter->_time[ipt] > _blStop[iCh]) // interrupt the loop for points after the end of the region for the calculation
	break;
      if(_scopeTreeInter->_time[ipt] >= _blStart[iCh])
	_blPoints[iCh].push_back(_scopeTreeInter->_channels[iCh][ipt] * _pol[iCh]);
    }
  }

  for(int iCh = 0; iCh < _nCh; ++iCh){ // fill signal vectors
    for(unsigned int ipt = 0; ipt < _scopeTreeInter->_npt; ++ipt){ // select the signal points
      if(_scopeTreeInter->_time[ipt] > _sigStop[iCh]) // interrupt the loop for points after the end of the signal region
	break;
      if(_scopeTreeInter->_time[ipt] >= _sigStart[iCh]){
	_sigPoints[iCh].push_back(_scopeTreeInter->_channels[iCh][ipt] * _pol[iCh]);
	_sigTime[iCh].push_back(_scopeTreeInter->_time[ipt]);
      }
    }
  }
    
  return;
}

void AnalyzeScopeClass::CalcBaselineNoise(){
  float blErr;
  float noiseErr;
  
  for(int iCh = 0; iCh < _nCh; ++iCh) // loop on channels
    CalcMeanStdDev(_blPoints[iCh], _baseline[iCh], _noise[iCh], blErr, noiseErr);

  return;
}

void AnalyzeScopeClass::CalcAmpliTime(){ // CalcBaselineNoise() should be called before this function to define the baseline
  
  for(int iCh = 0; iCh < _nCh; ++iCh){ // loop on channels
    _ampli[iCh] = -5; // start value
    _ampliTime[iCh] = -5; // start value

    for(unsigned int ipt = 0; ipt < _sigPoints[iCh].size(); ++ipt)
      if(_sigPoints[iCh][ipt] > _ampli[iCh]){
	_ampli[iCh] = _sigPoints[iCh][ipt];
	_ampliTime[iCh] = _sigTime[iCh][ipt];
      }

    _ampli[iCh] -= _baseline[iCh];
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

void AnalyzeScopeClass::CalcMeanStdDev(const std::vector<float>& vec, float& mean, float& stdDev, float& Emean, float& EstdDev)
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
