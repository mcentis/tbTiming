#include "AnalyzeScopeClass.hh"
#include "AnalysisPrototype.hh"
#include "SignalProperties.hh"
#include "TimingFixedFraction.hh"
#include "ThresholdStudy.hh"

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
  _minRiseTimeCut = new float[_nCh];
  _maxRiseTimeCut = new float[_nCh];
  _constFrac = new float[_nCh];

  _termination = new float[_nCh];
  
  _ampli = new float[_nCh];
  _integral = new float[_nCh];
  _ampliTime = new float[_nCh];
  _baseline = new float[_nCh];
  _noise = new float[_nCh];
  _riseTime = new float[_nCh];
  _risePoints = new int[_nCh];
  _linRegT0 = new float[_nCh];
  _tCFD = new float[_nCh];
  
  _blStart = new float[_nCh];
  _blStop = new float[_nCh];
  
  _sigStart = new float[_nCh];
  _sigStop = new float[_nCh];

  _inteStart = new float[_nCh];
  _inteStop = new float[_nCh];

  _blPoints = new std::vector<float>[_nCh];
  _sigPoints = new std::vector<float>[_nCh];
  _sigTime = new std::vector<float>[_nCh];
  
  GetCfgValues();

  RootBeautySettings();
  
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

  // tree with properties of the event
  _evtPropTree = new TTree("evtPropTree", "signal properties of the event");
  _evtPropTree->Branch("event", &_event, "event/l");
  _evtPropTree->Branch("nCh", &_nChTree, "nCh/i");
  _nChTree = _nCh;
  _evtPropTree->Branch("ampli", _ampli, "ampli[nCh]/F");
  _evtPropTree->Branch("ampliTime", _ampliTime, "ampliTime[nCh]/F");
  _evtPropTree->Branch("baseline", _baseline, "baseline[nCh]/F");
  _evtPropTree->Branch("noise", _noise, "noise[nCh]/F");
  _evtPropTree->Branch("riseTime", _riseTime, "riseTime[nCh]/F");
  _evtPropTree->Branch("integral", _integral, "integral[nCh]/F");
  _evtPropTree->Branch("linRegT0", _linRegT0, "linRegT0[nCh]/F");
  _evtPropTree->Branch("tCFD", _tCFD, "tCFD[nCh]/F");
    
  // create analysis objects without cuts
  _analysisWithoutCuts.push_back(new SignalProperties(this, "SignalPropertiesNoCuts"));

  // create analysis objects with cuts
  _analysisWCuts.push_back(new SignalProperties(this, "SignalProperties"));
  _analysisWCuts.push_back(new TimingFixedFraction(this, "TimingFixedFraction"));
  _analysisWCuts.push_back(new ThresholdStudy(this, "ThresholdStudy"));

  return;
}

AnalyzeScopeClass::~AnalyzeScopeClass(){
  _outFile->Close(); // should delete all root objects associated to the file
  
  delete[] _pol;
  delete[] _thr;
  delete[] _maxAmpliCut;
  delete[] _minRiseTimeCut;
  delete[] _maxRiseTimeCut;
  delete[] _termination;
  delete[] _constFrac;
  delete[] _ampli;
  delete[] _integral;
  delete[] _ampliTime;
  delete[] _baseline;
  delete[] _noise;
  delete[] _riseTime;
  delete[] _risePoints;
  delete[] _linRegT0;
  delete[] _tCFD;
  delete[] _blStart;
  delete[] _blStop;
  delete[] _sigStart;
  delete[] _sigStop;
  delete[] _inteStart;
  delete[] _inteStop;

  // delete analysis without cuts objects
  for(std::vector<AnalysisPrototype*>::iterator it = _analysisWithoutCuts.begin(); it != _analysisWithoutCuts.end(); it++)
    delete *it;

  // delete analysis with cuts objects
  for(std::vector<AnalysisPrototype*>::iterator it = _analysisWCuts.begin(); it != _analysisWCuts.end(); it++)
    delete *it;

  return;
}

void AnalyzeScopeClass::GetCfgValues(){
  // _cfg->DumpConfMap();

  ReadCfgArray(_pol, "polarity");
  for(int i = 0; i < _nCh; ++i) // make sure that polarity is normalized
    _pol[i] /= fabs(_pol[i]); 
  
  ReadCfgArray(_thr, "threshold");
  ReadCfgArray(_maxAmpliCut, "maxAmpli");
  ReadCfgArray(_minRiseTimeCut, "minRiseTime");
  ReadCfgArray(_maxRiseTimeCut, "maxRiseTime");
  ReadCfgArray(_constFrac, "fractionThr");
  ReadCfgArray(_blStart, "baseStart");
  ReadCfgArray(_blStop, "baseStop");
  ReadCfgArray(_sigStart, "signalStart");
  ReadCfgArray(_sigStop, "signalStop");
  ReadCfgArray(_termination, "termination");
  ReadCfgArray(_inteStart, "inteStart");
  ReadCfgArray(_inteStop, "inteStop");

  ReadTimingPairs();

  _minEvent = atoi(_cfg->GetValue("minEvent").c_str());
  _maxEvent = atoi(_cfg->GetValue("maxEvent").c_str());
  
  return;
}

void AnalyzeScopeClass::ReadTimingPairs(){
  std::string valStr = _cfg->GetValue("timingPairs");
  std::stringstream strstr(valStr);
  std::string sub;
  
  while(strstr.good()){
    getline(strstr, sub, ',');
    _timingPairs.push_back(sub);
  }

  //for(std::vector<std::string>::iterator it = _timingPairs.begin(); it != _timingPairs.end(); ++it)
  //  std::cout << *it << std::endl;
  
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
  unsigned long int evtToProcess = nEntries;
  
  if(_maxEvent != 0 && nEntries > _maxEvent)
    evtToProcess = _maxEvent;

  std::cout << "Events from " << _minEvent << " to " << evtToProcess << " will be processed." << std::endl;
  std::cout << "Below is shown the total number of events in the run" << std::endl;
  
  for(unsigned long int i = _minEvent; i < evtToProcess; ++i){
    _scopeTreeInter->_wavTree->GetEntry(i);

    if((i+1) % 1000 == 0 || (i+1) == nEntries)
      std::cout << " Processing event " << i+1 << " / " << nEntries << "                             \r" << std::flush;

    // select signal and baseline regions and apply polarity
    SelectPoints();
    
    // calculate pulses properties
    CalcBaselineNoise();
    CalcAmpliTime();
    CalcRiseTimeT0();
    CalcIntegral();
    CalcTcfd();
    
    _event = _scopeTreeInter->_event;
    _evtPropTree->Fill(); // fill event properties
    
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

  _outFile->cd();
  _evtPropTree->Write();
  
  return;
}

bool AnalyzeScopeClass::ProcessEvent(){
  bool ret = true;

  for(int iCh = 0; iCh < _nCh; ++iCh){ // amplitude cut, threshold and maximum amplitude
    ret = ret && _ampli[iCh] >= _thr[iCh];
    ret = ret && _ampli[iCh] <= _maxAmpliCut[iCh];
    ret = ret && _riseTime[iCh] >= _minRiseTimeCut[iCh];
    ret = ret && _riseTime[iCh] <= _maxRiseTimeCut[iCh];
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
    AnalysisPrototype::CalcMeanStdDev(_blPoints[iCh], _baseline[iCh], _noise[iCh], blErr, noiseErr);

  return;
}

void AnalyzeScopeClass::CalcAmpliTime(){ // CalcBaselineNoise() should be called before this function to define the baseline
  
  for(int iCh = 0; iCh < _nCh; ++iCh){ // loop on channels
    _ampli[iCh] = -5; // start value
    _ampliTime[iCh] = -5; // start value

    //int ptPos = -1;
    
    for(unsigned int ipt = 0; ipt < _sigPoints[iCh].size(); ++ipt)
      if(_sigPoints[iCh][ipt] > _ampli[iCh]){
	_ampli[iCh] = _sigPoints[iCh][ipt];
	_ampliTime[iCh] = _sigTime[iCh][ipt];
	//ptPos = ipt;
      }

    // // =================================================
    // // interpolation using points near maximum
    // // function y = a x**2 + b x + c
    // float y1 = _sigPoints[iCh][ptPos - 1];
    // float y2 = _sigPoints[iCh][ptPos];
    // float y3 = _sigPoints[iCh][ptPos + 1];
    // float x1 = _sigTime[iCh][ptPos - 1];
    // float x2 = _sigTime[iCh][ptPos];
    // float x3 = _sigTime[iCh][ptPos + 1];
    
    // float a = ( y3-y1 - (y2-y1)*(x3-x1)/(x2-x1) ) / ( pow(x3,2)-pow(x1,2) - (pow(x2,2)-pow(x1,2))*(x3-x1)/(x2-x1) );
    // float b = ( y2-y1 - a*(pow(x2,2)-pow(x1,2)) ) / (x2-x1);
    // float c = y1 - a*pow(x1,2) - b*x1;
    
    // _ampli[iCh] = - pow(b,2)/(4*a) + c;

    // _ampliTime[iCh] = -b/(2*a);

    // //======================================================
    
    _ampli[iCh] -= _baseline[iCh];
  }  
  
  return;
}

void AnalyzeScopeClass::CalcRiseTimeT0(){ // CalcBaselineNoise() and CalcAmpliTime() should be called before this function to define neede quantities
  float t1, t2; // used for risetime
  float a, b; // used for t0 with linear regression
  std::vector<float> x, y; // used for t0 with linear regression

  for(int iCh = 0; iCh < _nCh; ++iCh){
    t1 = AnalysisPrototype::CalcTimeThrLinear2pt(_sigPoints[iCh], _sigTime[iCh], 0.2 * _ampli[iCh], _baseline[iCh]);
    t2 = AnalysisPrototype::CalcTimeThrLinear2pt(_sigPoints[iCh], _sigTime[iCh], 0.8 * _ampli[iCh], _baseline[iCh]);

    _riseTime[iCh] = t2 -t1;
    
    x.clear();
    y.clear();
    std::vector<float>::iterator itTime = _sigTime[iCh].begin();
    std::vector<float>::iterator itVolt = _sigPoints[iCh].begin();
    for(; itTime != _sigTime[iCh].end() && itVolt != _sigPoints[iCh].end(); ++itTime, ++itVolt){
      if(*itTime > t2)
	break;
      if(*itTime < t1)
	continue;
      
      x.push_back(*itTime);
      y.push_back(*itVolt - _baseline[iCh]);
    }
    
    _risePoints[iCh] = x.size();
    
    if(_risePoints[iCh] >= 2){
      AnalysisPrototype::LinearReg(x, y, a, b); // y = ax + b
      _linRegT0[iCh] = -b/a; // 0 crossing time
    }
    else
      _linRegT0[iCh] = 10; // if not enough points for linear regression
    
  }
  
  return;
}

void AnalyzeScopeClass::CalcTcfd(){
  for(int iCh = 0; iCh < _nCh; ++iCh)
    _tCFD[iCh] = AnalysisPrototype::CalcTimeThrLinear2pt(_sigPoints[iCh], _sigTime[iCh], _constFrac[iCh] * _ampli[iCh], _baseline[iCh]);
  
  return;
}

void AnalyzeScopeClass::CalcIntegral(){
  for(int iCh = 0; iCh < _nCh; ++iCh){
    if(_linRegT0[iCh] == 10){ // the 0 crossing time is not determined
      _integral[iCh] = 10;
      continue;
    }

    _integral[iCh] = AnalysisPrototype::Integrate(_sigPoints[iCh], _sigTime[iCh], _linRegT0[iCh] + _inteStart[iCh], _linRegT0[iCh] + _inteStop[iCh], _baseline[iCh]);
    _integral[iCh] /= _termination[iCh]; // from [Vs] to [C]
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
