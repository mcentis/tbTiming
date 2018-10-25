#include "FourierAnalysis.hh"

#include "TH1F.h"
#include "TVirtualFFT.h"

#include <iostream>

FourierAnalysis::FourierAnalysis(AnalyzeScopeClass* acl, const char* dirName)
  : AnalysisPrototype(acl, dirName){

  _baselineRegion = new TH1F*[_acl->_nCh];
  _signalRegion = new TH1F*[_acl->_nCh];
  
  evtCounter = 0;
  
  char name[50];
  char title[200];

  // added instance number to names to avoid error of histos with same names
  for(int iCh = 0; iCh < _acl->_nCh; ++iCh){
    sprintf(name, "baselineRegion_inst%d_Ch%d", _instanceNumber , iCh+1);
    sprintf(title, "Baseline region Ch%d;Frequency [Hz];Average magnitude", iCh+1);
    _baselineRegion[iCh] = new TH1F(name, title, 3000, 0, 3e9); // 1 MHz bins

    sprintf(name, "signalRegion_inst%d_Ch%d", _instanceNumber , iCh+1);
    sprintf(title, "Signal region Ch%d;Frequency [Hz];Average magnitude", iCh+1);
    _signalRegion[iCh] = new TH1F(name, title, 3000, 0, 3e9); // 1 MHz bins
  }
    
}

FourierAnalysis::~FourierAnalysis(){
  return;
}

void FourierAnalysis::AnalysisAction(){
  for(int iCh = 0; iCh < _acl->_nCh; ++iCh){
    double dt = _acl->_sigTime[iCh].at(1) - _acl->_sigTime[iCh].at(0); // spacing between points

    DoFFT(_acl->_blPoints[iCh], dt, _baselineRegion[iCh]);
    DoFFT(_acl->_sigPoints[iCh], dt, _signalRegion[iCh]);
  }

  evtCounter++;
  
  return;
}

void FourierAnalysis::DoFFT(std::vector<float>& data, double dt, TH1F* hist){
  int transSize = data.size(); // size of data sample
    
  const int dimIn = 2 * (transSize / 2 + 1); // dimension of the array needed for the transform
  double* inArray = new double[dimIn];
  for(int i = 0; i < dimIn; ++i) // initialize array to 0
    inArray[i] = 0;
    
  for(int i = 0; i < transSize; ++i) // put the data in the array
    inArray[i] = data.at(i);
    
  TVirtualFFT* fft = TVirtualFFT::FFT(1, &transSize, "R2C M"); // fft object
  fft->SetPoints(inArray);
  fft->Transform();
  fft->GetPoints(inArray);

  double re, im;
  for(int i = 0; i < transSize / 2 + 1; ++i){
    fft->GetPointComplex(i, re, im);
    hist->Fill(((double)i)/(transSize*dt), sqrt(pow(re, 2) + pow(im, 2)) / sqrt(transSize));
  }
  
  delete[] inArray; 
  
  return;
}

void FourierAnalysis::Process(){
  for(int iCh = 0; iCh < _acl->_nCh; ++iCh){
    ScaleBins(_baselineRegion[iCh], 1.0/evtCounter);
    ScaleBins(_signalRegion[iCh], 1.0/evtCounter);
  }

  return;
}

void FourierAnalysis::ScaleBins(TH1F* hist, float scale){
  for(int i = 1; i <= hist->GetNbinsX(); ++i){
    hist->SetBinContent(i, hist->GetBinContent(i) * scale);
    hist->SetBinError(i, hist->GetBinError(i) * scale);
  }
  
  return;
}

void FourierAnalysis::Save(TDirectory* parent){
  TDirectory* dir = parent->mkdir(_dirName.c_str());
  dir->cd();

  for(int iCh = 0; iCh < _acl->_nCh; ++iCh){
    _baselineRegion[iCh]->Write();
    _signalRegion[iCh]->Write();
  }
  
  return;
}
