#include "ThresholdStudy.hh"
#include "TimingFixedFraction.hh"

#include <sstream>
#include <iostream>

#include "TH1F.h"
#include "TH2F.h"

ThresholdStudy::ThresholdStudy(AnalyzeScopeClass* acl, const char* dirName)
  : AnalysisPrototype(acl, dirName){

  _nPairs = _acl->_timingPairs.size();
  _pairs = new int*[_nPairs];

  for(int i = 0; i < _nPairs; ++i)
    _pairs[i] = TimingFixedFraction::GetPair(_acl->_timingPairs[i], _acl->_nCh);
  
  // create histograms
  const int fracBins = 19;
  double fracMin = 0.025;
  double fracMax = 0.975;

  char name[50];
  char title[200];

  // add instance number to names to avoid error of histos with same names
  for(int i = 0; i < _nPairs; ++i){
    sprintf(name, "timeDiffMean_inst%d_Ch%d_Ch%d", _instanceNumber , _pairs[i][0]+1, _pairs[i][1]+1);
    sprintf(title, "Mean #Delta t CFD Ch%d-Ch%d;CFD frac Ch%d;CFD frac Ch%d;Mean #Delta t [s]", _pairs[i][0]+1, _pairs[i][1]+1, _pairs[i][0]+1, _pairs[i][1]+1);
    _timeDiffMean.push_back(new TH2F(name, title, fracBins, fracMin, fracMax, fracBins, fracMin, fracMax));

    sprintf(name, "timeDiffStdDev_inst%d_Ch%d_Ch%d", _instanceNumber , _pairs[i][0]+1, _pairs[i][1]+1);
    sprintf(title, "Std. Dev. #Delta t CFD Ch%d-Ch%d;CFD frac Ch%d;CFD frac Ch%d;Std. Dev. #Delta t [s]", _pairs[i][0]+1, _pairs[i][1]+1, _pairs[i][0]+1, _pairs[i][1]+1);
    _timeDiffStdDev.push_back(new TH2F(name, title, fracBins, fracMin, fracMax, fracBins, fracMin, fracMax));

    sprintf(name, "bestTimeDiffDistrCFD_inst%d_Ch%d_Ch%d", _instanceNumber , _pairs[i][0]+1, _pairs[i][1]+1);
    sprintf(title, "#Delta t CFD Ch%d-Ch%d;#Delta t [s];Entries" , _pairs[i][0]+1, _pairs[i][1]+1);
    _bestTimeDiffDistr.push_back(new TH1F(name, title, 2500, -10e-9, 10e-9)); // 8 ps bins 
  }

  for(int i = 0; i < _timeDiffMean[0]->GetNbinsX(); ++i) // threshold vector (same for all pairs)
    _cfdThr.push_back(_timeDiffMean[0]->GetXaxis()->GetBinCenter(i+1)); // i+1 to have the bin numbering of the histo

  // create the matrices holding the time differences for the different thresholds
  for(int iPair = 0; iPair < _nPairs; ++iPair){
    _timeDiff.push_back(new std::vector<float>*[fracBins]); // no underflow and overflow bins
    for(int iThr = 0; iThr < fracBins; ++iThr)
      _timeDiff[iPair][iThr] = new std::vector<float>[fracBins]; // no underflow and overflow bins
  }
  
  return;
}

ThresholdStudy::~ThresholdStudy(){
  for(int i = 0; i < _nPairs; ++i){
    delete[] _pairs[i];
    
    for(unsigned int iThr = 0; iThr < _timeDiff[i][0]->size(); ++iThr) // exploit that the thr matrix is square _timeDiff[i][0]->size()
      for(unsigned int jThr = 0; jThr < _timeDiff[i][0]->size(); ++jThr) // exploit that the thr matrix is square _timeDiff[i][0]->size()
	_timeDiff[i][iThr][jThr].clear();
  }
  
  return;
}

void ThresholdStudy::AnalysisAction(){
  float t1, t2;
  int iCh;
  
  for(int iPair = 0; iPair < _nPairs; ++iPair)
    for(unsigned int i = 0; i < _cfdThr.size(); ++i){
      iCh = _pairs[iPair][0];
      t1 = CalcTimeThrLinear2pt(_acl->_sigPoints[iCh], _acl->_sigTime[iCh], _cfdThr[i] * _acl->_ampli[iCh], _acl->_baseline[iCh]);

      for(unsigned int j = 0; j < _cfdThr.size(); ++j){	
	iCh = _pairs[iPair][1];
	t2 = CalcTimeThrLinear2pt(_acl->_sigPoints[iCh], _acl->_sigTime[iCh], _cfdThr[j] * _acl->_ampli[iCh], _acl->_baseline[iCh]);

	_timeDiff[iPair][i][j].push_back(t1 - t2); // t1 - t2 to have the same convention as the TimingFixedThreshold
      }
    }
  
  return;
}

void ThresholdStudy::Process(){
  float mean, stdDev, Emean, EstdDev;
  float minStdDev;
  int bestThrPos1, bestThrPos2;
  char title[200];
  
  for(int iPair = 0; iPair < _nPairs; ++iPair){
    minStdDev = 1e6;

    for(unsigned int i = 0; i < _cfdThr.size(); ++i)
      for(unsigned int j = 0; j < _cfdThr.size(); ++j){	
	CalcMeanStdDev(_timeDiff[iPair][i][j], mean, stdDev, Emean, EstdDev);

  	_timeDiffMean[iPair]->SetBinContent(i+1, j+1, mean);
  	_timeDiffStdDev[iPair]->SetBinContent(i+1, j+1, stdDev);

	if(stdDev < minStdDev){
	  minStdDev = stdDev;
	  bestThrPos1 = i;
	  bestThrPos2 = j;
	}
      }

    // prepare histo with best std dev distr
    CalcMeanStdDev(_timeDiff[iPair][bestThrPos1][bestThrPos2], mean, stdDev, Emean, EstdDev);
    sprintf(title, "#Delta t CFD Ch%d-Ch%d: Thr1 %.2F, Thr2 %.2F, #sigma = %.2F #pm %.2F ps, %i events", _pairs[iPair][0]+1, _pairs[iPair][1]+1, _cfdThr[bestThrPos1], _cfdThr[bestThrPos2], stdDev * 1e12, EstdDev * 1e12, (int) _timeDiff[iPair][bestThrPos1][bestThrPos2].size());
  _bestTimeDiffDistr[iPair]->SetTitle(title);
  for(std::vector<float>::iterator it = _timeDiff[iPair][bestThrPos1][bestThrPos2].begin(); it != _timeDiff[iPair][bestThrPos1][bestThrPos2].end(); ++it)
    _bestTimeDiffDistr[iPair]->Fill(*it);

  }
    
  return;
}

void ThresholdStudy::Save(TDirectory* parent){
  TDirectory* dir = parent->mkdir(_dirName.c_str());
  dir->cd();

  for(int iPair = 0; iPair < _nPairs; ++iPair){
    _timeDiffMean[iPair]->Write();
    _timeDiffStdDev[iPair]->Write();
    _bestTimeDiffDistr[iPair]->Write();
  }
  
  return;
}
