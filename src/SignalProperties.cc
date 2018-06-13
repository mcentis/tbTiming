#include "SignalProperties.hh"

#include "TH1F.h"

#include <iostream>

SignalProperties::SignalProperties(AnalyzeScopeClass* acl, const char* dirName)
  : AnalysisPrototype(acl, dirName){

  char name[50];
  char title[200];

  // added instance number to names to avoid error of histos with same names
  for(int iCh = 0; iCh < _acl->_nCh; ++iCh){
    sprintf(name, "baselineDistr_inst%d_Ch%d",_instanceNumber , iCh+1);
    sprintf(title, "Baseline Ch%d;Baseline [V];Entries", iCh+1);
    _baselineDistr[iCh] = new TH1F(name, title, 400, -0.1, 0.1); // 0.5 mV bins 

    sprintf(name, "noiseSingleEvtDistr_inst%d_Ch%d",_instanceNumber, iCh+1);
    sprintf(title, "Single Event Noise Ch%d;Noise [V];Entries", iCh+1);
    _noiseSingleEvtDistr[iCh] = new TH1F(name, title, 400, -0.05, 0.05); // 0.25 mV bins 

    sprintf(name, "ampliDistr_inst%d_Ch%d",_instanceNumber, iCh+1);
    sprintf(title, "Amplitude Ch%d;Amplitude [V];Entries", iCh+1);
    _ampliDistr[iCh] = new TH1F(name, title, 550, -0.1, 1); // 2 mV bins 

    sprintf(name, "ampliTimeDistr_inst%d_Ch%d",_instanceNumber, iCh+1);
    sprintf(title, "Time of Max Ch%d;Time of Max [s];Entries", iCh+1);
    _ampliTimeDistr[iCh] = new TH1F(name, title, 3001, -0.1e-9, 300.1e-9); // 100 ps bins
  }
  
  return;
}

SignalProperties::~SignalProperties(){
  for(int iCh = 0; iCh < _acl->_nCh; ++iCh){
    delete _baselineDistr[iCh];
    delete _noiseSingleEvtDistr[iCh];
    delete _ampliDistr[iCh];
    delete _ampliTimeDistr[iCh];
  }
  
  return;
}

void SignalProperties::AnalysisAction(){
  for(int iCh = 0; iCh < _acl->_nCh; ++iCh){
    _baselineDistr[iCh]->Fill(_acl->_baseline[iCh]);
    _noiseSingleEvtDistr[iCh]->Fill(_acl->_noise[iCh]);
    _ampliDistr[iCh]->Fill(_acl->_ampli[iCh]);
    _ampliTimeDistr[iCh]->Fill(_acl->_ampliTime[iCh]);
  }
  
  return;
}

void SignalProperties::Save(TDirectory* parent){
  TDirectory* dir = parent->mkdir(_dirName.c_str());
  dir->cd();
  
  for(int iCh = 0; iCh < _acl->_nCh; ++iCh){
    _baselineDistr[iCh]->Write();
    _noiseSingleEvtDistr[iCh]->Write();
    _ampliDistr[iCh]->Write();
    _ampliTimeDistr[iCh]->Write();
  }

  return;
}
