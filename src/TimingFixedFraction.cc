#include "TimingFixedFraction.hh"

#include <sstream>
#include <iostream>

TimingFixedFraction::TimingFixedFraction(AnalyzeScopeClass* acl, const char* dirName)
  : AnalysisPrototype(acl, dirName){

  _nPairs = _acl->_timingPairs.size();
  _pairs = new int*[_nPairs];

  for(int i = 0; i < _nPairs; ++i)
    _pairs[i] = GetPair(_acl->_timingPairs[i]);
  
  return;
}

TimingFixedFraction::~TimingFixedFraction(){

  for(int i = 0; i < _nPairs; ++i){
    delete[] _pairs[i];
  }

  return;
}

int* TimingFixedFraction::GetPair(std::string pairstr){
  int* pair = new int[2];

  std::stringstream strstr(pairstr);
  std::string sub;
  
  getline(strstr, sub, '-');
  pair[0] = atoi(sub.c_str()) - 1; // subtract 1 to have ch starting from 0
  getline(strstr, sub, '-');
  pair[1] = atoi(sub.c_str()) - 1; // subtract 1 to have ch starting from 0

  if(pair[0] >= AnalyzeScopeClass::_nCh || pair[1] >= AnalyzeScopeClass::_nCh){
    std::cout << "[Error] TimingFixedFraction::GetPair Channel number out of range for " << pairstr << std::endl;
    exit(1);
  }
  
  return pair;
}

void TimingFixedFraction::AnalysisAction(){
  return;
}

void TimingFixedFraction::Save(TDirectory* parent){
  TDirectory* dir = parent->mkdir(_dirName.c_str());
  dir->cd();

  return;
}
    
