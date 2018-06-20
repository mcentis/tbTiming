#include "TimingFixedFraction.hh"

#include <sstream>
#include <iostream>

#include "TH1F.h"

TimingFixedFraction::TimingFixedFraction(AnalyzeScopeClass* acl, const char* dirName)
  : AnalysisPrototype(acl, dirName){

  _nPairs = _acl->_timingPairs.size();
  _pairs = new int*[_nPairs];

  for(int i = 0; i < _nPairs; ++i)
    _pairs[i] = GetPair(_acl->_timingPairs[i], _acl->_nCh);
  
  // create histograms  
  char name[50];
  char title[200];

  // add instance number to names to avoid error of histos with same names
  for(int i = 0; i < _nPairs; ++i){
    sprintf(name, "deltaTDistrCFD_inst%d_Ch%d_Ch%d",_instanceNumber , _pairs[i][0]+1, _pairs[i][1]+1);
    sprintf(title, "#Delta t CFD Ch%d-Ch%d;#Delta t [s];Entries" , _pairs[i][0]+1, _pairs[i][1]+1);
    _timeDiffCFD.push_back(new TH1F(name, title, 2500, -10e-9, 10e-9)); // 8 ps bins 

    sprintf(name, "deltaTDistrLinReg_inst%d_Ch%d_Ch%d",_instanceNumber , _pairs[i][0]+1, _pairs[i][1]+1);
    sprintf(title, "#Delta t Linear Regression Ch%d-Ch%d;#Delta t [s];Entries" , _pairs[i][0]+1, _pairs[i][1]+1);
    _timeDiffLinReg.push_back(new TH1F(name, title, 2500, -10e-9, 10e-9)); // 8 ps bins 
}

  _tCFD = new float[_acl->_nCh];
  
  return;
}

TimingFixedFraction::~TimingFixedFraction(){
  for(int i = 0; i < _nPairs; ++i)
    delete[] _pairs[i];

  delete[] _tCFD;
  
  return;
}

int* TimingFixedFraction::GetPair(std::string pairstr, int maxChNum){
  int* pair = new int[2];

  std::stringstream strstr(pairstr);
  std::string sub;
  
  getline(strstr, sub, '-');
  pair[0] = atoi(sub.c_str()) - 1; // subtract 1 to have ch starting from 0
  getline(strstr, sub, '-');
  pair[1] = atoi(sub.c_str()) - 1; // subtract 1 to have ch starting from 0

  if(pair[0] >= maxChNum || pair[1] >= maxChNum){
    std::cout << "[Error] TimingFixedFraction::GetPair Channel number out of range for " << pairstr << std::endl;
    exit(1);
  }
  
  return pair;
}

void TimingFixedFraction::AnalysisAction(){
  // get the time of threshold crossing
  for(int iCh = 0; iCh < _acl->_nCh; ++iCh)
    _tCFD[iCh] = CalcTimeThrLinear2pt(_acl->_sigPoints[iCh], _acl->_sigTime[iCh], _acl->_constFrac[iCh] * _acl->_ampli[iCh], _acl->_baseline[iCh]);
    
  // fill the histograms
  for(int i = 0; i < _nPairs; ++i){
    _timeDiffCFD[i]->Fill(_tCFD[_pairs[i][0]] - _tCFD[_pairs[i][1]]);
    _timeDiffLinReg[i]->Fill(_acl->_linRegT0[_pairs[i][0]] - _acl->_linRegT0[_pairs[i][1]]);
  }
  
  return;
}

void TimingFixedFraction::Save(TDirectory* parent){
  TDirectory* dir = parent->mkdir(_dirName.c_str());
  dir->cd();

  for(int i = 0; i < _nPairs; ++i)
    _timeDiffCFD[i]->Write();

  for(int i = 0; i < _nPairs; ++i)
    _timeDiffLinReg[i]->Write();

    
  return;
}
    
