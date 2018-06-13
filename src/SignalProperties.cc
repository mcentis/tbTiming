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

    sprintf(name, "noiseDistr_inst%d_Ch%d",_instanceNumber, iCh+1);
    sprintf(title, "Noise Distribution Ch%d;Noise [V];Entries", iCh+1);
    _noiseDistr[iCh] = new TH1F(name, title, 400, -0.05, 0.05); // 0.25 mV bins 

    sprintf(name, "ampliDistr_inst%d_Ch%d",_instanceNumber, iCh+1);
    sprintf(title, "Amplitude Ch%d;Amplitude [V];Entries", iCh+1);
    _ampliDistr[iCh] = new TH1F(name, title, 550, -0.1, 1); // 2 mV bins 

    sprintf(name, "ampliTimeDistr_inst%d_Ch%d",_instanceNumber, iCh+1);
    sprintf(title, "Time of Max Ch%d;Time of Max [s];Entries", iCh+1);
    _ampliTimeDistr[iCh] = new TH1F(name, title, 3001, -0.1e-9, 300.1e-9); // 100 ps bins

    sprintf(name, "riseTime2080Distr_inst%d_Ch%d",_instanceNumber, iCh+1);
    sprintf(title, "20%% to 80%% rise time Ch%d;Rise time [s];Entries", iCh+1);
    _riseTimeDistr[iCh] = new TH1F(name, title, 400, 0, 2e-9); // 5 ps bins

    sprintf(name, "supSignal_inst%d_Ch%d",_instanceNumber, iCh+1);
    sprintf(title, "Signal Ch%d;Time [s];Voltage [V]", iCh+1);
    _supSignal[iCh] = new TH2I(name, title, 121, -2.1e-9, 10.1e-9, 550, -0.1, 1); // 100 ps *  2 mV bins

    sprintf(name, "supSignalScaled_inst%d_Ch%d",_instanceNumber, iCh+1);
    sprintf(title, "Signal normalized amplitude Ch%d;Time [s];Signal fraction", iCh+1);
    _supSignalScaled[iCh] = new TH2I(name, title, 121, -2.1e-9, 10.1e-9, 560, -0.2, 1.2); // 100 ps *  0.25 % bins
}
  
  return;
}

SignalProperties::~SignalProperties(){
  for(int iCh = 0; iCh < _acl->_nCh; ++iCh){
    delete _baselineDistr[iCh];
    delete _noiseSingleEvtDistr[iCh];
    delete _noiseDistr[iCh];
    delete _ampliDistr[iCh];
    delete _ampliTimeDistr[iCh];
    delete _riseTimeDistr[iCh];
    delete _supSignal[iCh];
    delete _supSignalScaled[iCh];
  }
  
  return;
}

void SignalProperties::AnalysisAction(){
  float t1, t2; // used for risetime
  
  for(int iCh = 0; iCh < _acl->_nCh; ++iCh){
    _baselineDistr[iCh]->Fill(_acl->_baseline[iCh]);
    _noiseSingleEvtDistr[iCh]->Fill(_acl->_noise[iCh]);
    _ampliDistr[iCh]->Fill(_acl->_ampli[iCh]);
    _ampliTimeDistr[iCh]->Fill(_acl->_ampliTime[iCh]);

    for(std::vector<float>::iterator it = _acl->_blPoints[iCh].begin(); it != _acl->_blPoints[iCh].end(); ++it)
      _noiseDistr[iCh]->Fill(*it - _acl->_baseline[iCh]);

    t1 = CalcTimeThrLinear2pt(_acl->_sigPoints[iCh], _acl->_sigTime[iCh], 0.2 * _acl->_ampli[iCh], _acl->_baseline[iCh]);
    t2 = CalcTimeThrLinear2pt(_acl->_sigPoints[iCh], _acl->_sigTime[iCh], 0.8 * _acl->_ampli[iCh], _acl->_baseline[iCh]);
    _riseTimeDistr[iCh]->Fill(t2 - t1);

    t1 = CalcTimeThrLinear2pt(_acl->_sigPoints[iCh], _acl->_sigTime[iCh], _acl->_constFrac[iCh] * _acl->_ampli[iCh], _acl->_baseline[iCh]); // use the CFD threhsold of the analysis to "align" the waveforms
    for(unsigned int ipt = 0; ipt < _acl->_sigPoints[iCh].size(); ++ipt){
      _supSignal[iCh]->Fill(_acl->_sigTime[iCh][ipt] - t1, _acl->_sigPoints[iCh][ipt] - _acl->_baseline[iCh]);
      _supSignalScaled[iCh]->Fill(_acl->_sigTime[iCh][ipt] - t1, (_acl->_sigPoints[iCh][ipt] - _acl->_baseline[iCh]) / _acl->_ampli[iCh]);
    }
  }
  
  return;
}

void SignalProperties::Save(TDirectory* parent){
  TDirectory* dir = parent->mkdir(_dirName.c_str());
  dir->cd();
  
  for(int iCh = 0; iCh < _acl->_nCh; ++iCh){
    _baselineDistr[iCh]->Write();
    _noiseSingleEvtDistr[iCh]->Write();
    _noiseDistr[iCh]->Write();
    _ampliDistr[iCh]->Write();
    _ampliTimeDistr[iCh]->Write();
    _riseTimeDistr[iCh]->Write();
    _supSignal[iCh]->Write();
    _supSignalScaled[iCh]->Write();
  }

  return;
}
