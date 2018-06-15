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
    sprintf(title, "Signal Ch%d (at least 2 pt between 20%% and 80%%);Time [s];Voltage [V]", iCh+1);
    _supSignal[iCh] = new TH2I(name, title, 121, -2.1e-9, 10.1e-9, 550, -0.1, 1); // 100 ps *  2 mV bins

    sprintf(name, "supSignalScaled_inst%d_Ch%d",_instanceNumber, iCh+1);
    sprintf(title, "Signal normalized amplitude Ch%d (at least 2 pt between 20%% and 80%%);Time [s];Signal fraction", iCh+1);
    _supSignalScaled[iCh] = new TH2I(name, title, 121, -2.1e-9, 10.1e-9, 560, -0.2, 1.2); // 100 ps *  0.25 % bins

    sprintf(name, "signalDerivative_inst%d_Ch%d",_instanceNumber, iCh+1);
    sprintf(title, "Derivative of normalized signal Ch%d (at least 2 pt between 20%% and 80%%)", iCh+1);
    _signalDerivative[iCh] = new TGraph();
    _signalDerivative[iCh]->SetName(name);
    _signalDerivative[iCh]->SetTitle(title);
    _signalDerivative[iCh]->SetMarkerStyle(20);

    sprintf(name, "signalDerFrac_inst%d_Ch%d",_instanceNumber, iCh+1);
    sprintf(title, "Derivative of normalized signal Ch%d (at least 2 pt between 20%% and 80%%)", iCh+1);
    _signalDerFrac[iCh] = new TGraph();
    _signalDerFrac[iCh]->SetName(name);
    _signalDerFrac[iCh]->SetTitle(title);
    _signalDerFrac[iCh]->SetMarkerStyle(20);

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
    delete _profSignalScaled[iCh];
    delete _signalDerivative[iCh];
    delete _signalDerFrac[iCh];
  }
  
  return;
}

void SignalProperties::AnalysisAction(){
  float t1, t2; // used for risetime
  float a, b; // used for signal superimposition
  std::vector<float> x, y; // used for signal superimposition
  
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

    // signal superimposition, use leading edge interpolation to 0 to "align" the signals, selection of points between t1 and t2
    x.clear();
    y.clear();
    std::vector<float>::iterator itTime = _acl->_sigTime[iCh].begin();
    std::vector<float>::iterator itVolt = _acl->_sigPoints[iCh].begin();
    for(; itTime != _acl->_sigTime[iCh].end() && itVolt != _acl->_sigPoints[iCh].end(); ++itTime, ++itVolt){
      if(*itTime > t2)
	break;
      if(*itTime < t1)
	continue;
      
      x.push_back(*itTime);
      y.push_back(*itVolt - _acl->_baseline[iCh]);
    }

    if(x.size() >= 2){
      LinearReg(x, y, a, b); // y = ax + b
      t1 = -b/a; // 0 crossing time

      itTime = _acl->_sigTime[iCh].begin();
      itVolt = _acl->_sigPoints[iCh].begin();
      for(; itTime != _acl->_sigTime[iCh].end() && itVolt != _acl->_sigPoints[iCh].end(); ++itTime, ++itVolt){

	_supSignal[iCh]->Fill(*itTime - t1, *itVolt - _acl->_baseline[iCh]);
	_supSignalScaled[iCh]->Fill(*itTime - t1, (*itVolt - _acl->_baseline[iCh]) / _acl->_ampli[iCh]);
      }
    }

  }
  
  return;
}

void SignalProperties::Process(){
  char name[50];

  for(int iCh = 0; iCh < _acl->_nCh; ++iCh){ // get the profiles of the signal
    sprintf(name, "profileSignalScaled_inst%d_Ch%d",_instanceNumber, iCh+1);
    
    _profSignalScaled[iCh] = _supSignalScaled[iCh]->ProfileX(name);
    _profSignalScaled[iCh]->GetYaxis()->SetTitle("Signal fraction");
  }

  // derivative of the signal
  float y1, y2, x1, x2, slope;
  for(int iCh = 0; iCh < _acl->_nCh; ++iCh)
    for(int iBin = 1; iBin < _profSignalScaled[iCh]->GetNbinsX(); ++iBin){
      y1 = _profSignalScaled[iCh]->GetBinContent(iBin);
      y2 = _profSignalScaled[iCh]->GetBinContent(iBin + 1);
      x1 = _profSignalScaled[iCh]->GetBinCenter(iBin);
      x2 = _profSignalScaled[iCh]->GetBinCenter(iBin + 1);
      slope = (y2-y1)/(x2-x1);
      _signalDerivative[iCh]->SetPoint(iBin - 1, x1+(x2-x1)/2, slope);

      if(iBin > _profSignalScaled[iCh]->GetMaximumBin()) // select only points before maximum of signal for following graph
	continue;
      
      _signalDerFrac[iCh]->SetPoint(iBin - 1, y1+(y2-y1)/2, slope);

    }
  
  return;
}

void SignalProperties::Save(TDirectory* parent){
  TDirectory* dir = parent->mkdir(_dirName.c_str());
  dir->cd();

  for(int iCh = 0; iCh < _acl->_nCh; ++iCh){
    PutAxisLabels(_signalDerivative[iCh], "Time [s]", "Signal derivative [1/s]");
    PutAxisLabels(_signalDerFrac[iCh], "Signal fraction", "Signal derivative [1/s]");
  }
  
  for(int iCh = 0; iCh < _acl->_nCh; ++iCh){
    _baselineDistr[iCh]->Write();
    _noiseSingleEvtDistr[iCh]->Write();
    _noiseDistr[iCh]->Write();
    _ampliDistr[iCh]->Write();
    _ampliTimeDistr[iCh]->Write();
    _riseTimeDistr[iCh]->Write();
    _supSignal[iCh]->Write();
    _supSignalScaled[iCh]->Write();
    _profSignalScaled[iCh]->Write();
    _signalDerivative[iCh]->Write();
    _signalDerFrac[iCh]->Write();
  }

  return;
}
