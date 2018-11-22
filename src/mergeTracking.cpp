#include <iostream>
#include <vector>
#include <sstream>

#include "ConfigFileReader.hh"
#include "ScopeTreeInterface.hh"
#include "lineFit.hh" // class for the fit of the tracks

#include "TTree.h"
#include "TFile.h"
#include "TH2I.h"
#include "Math/Functor.h"
#include "Fit/Fitter.h"

const int nBits = 16; // number of bits in the beam telescope trigger counter
const int nCh = 4; // number of oscilloscope channels

// function to interpret the NIM coded event counter of the telescope used for the test beam of RD51
// the pattern consists of 1 bit always on (to trigger) and 16 bits MSB first that constitute the counter
// each bit lasts 25 ns
long int trigNum(Float_t* volt, int npt, float dt) // vector with the voltages, dt between points (sampling )
{
  float logicThr = -0.4; // [V] V < thr => 1; V > thr => 0
  float period = 25e-9; // duration of each bit [s]
  int periodPts = period / dt;

  // calculate baseline
  int blpt = 10; // number of points used for baseline
  float bl = 0;
  for(int i = 0 ; i < blpt; ++i)
    bl += volt[i];
  bl /= blpt;
  
  int start = -1;
  for(int i = 0; i < npt; ++i){
    if(volt[i] - bl > logicThr) continue;
    start = i;
    break;
  }

  if(start == -1)// no signal found
    return -1;

  start += 1.5 * periodPts; // start should be in the middle of the first (MSB) bit

  long int nTrig = 0;
  int pos = 0;
  
  for(int i = 0; i < nBits; ++i){
    pos = start + i * periodPts;
    if(pos > npt)
      return -2;

    if(volt[pos] - bl < logicThr){
      nTrig += pow(2, nBits - i - 1);
    }
  }

  return nTrig;
}

// function to calculate the cycle number, cycle should start from 0
// returns true if the cycle number changes
// a cycle is a block of 65536 events, after which the bit pattern of the beam telescope starts again from 0
bool cycleNumber(long int prevTrigNum, long int trigNum, int& cycle)
{  
  if(prevTrigNum > trigNum){ // restarted counting, or new tracking file in a chain
    cycle++;
    return true;
  }
  
  // trigger number from the beam telescope tree can be bigger than the 65536 expected from a 16 bit pattern, all numbers should be there for a run
  if(trigNum !=0 && trigNum % (int) pow(2, nBits) == 0){ // added trigNum != 0 to avoid problems on the first iteration, in case of a chain of tracking data, the previous if should take care of it
    cycle++;
    return true;
  }
    
  return false;
}

// function to read different parameters from the config file, modified from AnalyzeScopeClass
template<typename T> void ReadCfgArray(T* parameter, const char* key, ConfigFileReader* cfg, int length){
  std::string valStr = cfg->GetValue(key);
  std::stringstream strstr(valStr);
  std::string sub;
  
  for(int i = 0; i < length; ++i){
    if(strstr.good() == false){
      std::cout << "Error while reading config file for parameter " << key << std::endl;
      exit(1);
    }
    getline(strstr, sub, ',');
    parameter[i] = atof(sub.c_str());
  }
  
  return;
}

int main(int argc, char* argv[])
{
  if(argc != 4){
    std::cout << "Usage: mergeTracking oscilloscopeRootFile trackingRootFile configFile" << std::endl;
    return 1;
  }

  // std::string scopeFileName(argv[1]);
  // std::string ivtFileName = inFileName.substr(0, inFileName.size() - 5); // assume that the root file ends in .root and ivt file is in same directory
  // ivtFileName.append("_IVTtree.root");

  ConfigFileReader* cfg = new ConfigFileReader(argv[3]);
  ScopeTreeInterface* scopeTree = new ScopeTreeInterface(argv[1]);

  int trigCh = atoi(cfg->GetValue("trigCh").c_str()) - 1; // oscilloscope channel containing the bit pattern
  int sigCh = atoi(cfg->GetValue("sigCh").c_str()) - 1; // oscilloscope channel containing the signal to be used for the alignment
  int pol = atoi(cfg->GetValue("pol").c_str()); // signal polarity
  pol /= abs(pol);
  float thr = atof(cfg->GetValue("thr").c_str()); // signal threshold
  float blStart = atof(cfg->GetValue("blStart").c_str()); // start of baseline for signal channel
  float blStop = atof(cfg->GetValue("blStop").c_str()); // end of baseline for signal channel
  float sigStart = atof(cfg->GetValue("sigStart").c_str()); // start of signal region for signal channel
  float sigStop = atof(cfg->GetValue("sigStop").c_str()); // end of signal region for signal channel
  int nRecPlanes = atoi(cfg->GetValue("nRecPlanes").c_str()); // number of telescope planes used in the (re)fitting of the tracks
  int* recPlanes = new int[nRecPlanes];
  ReadCfgArray(recPlanes, "recPlanes", cfg, nRecPlanes);
  float* dutPos = new float[nCh];
  ReadCfgArray(dutPos, "dutPos", cfg, nCh);
  
  long int nTrigScope; // from the oscilloscope waveform
  long int nTrigScopePrev; // trig number of previous entry
  int cycleScope = 0; // cycle of the oscilloscope trigger
  
  std::vector<long int> trigList; // list of trig number of events above threshold for the selected channel

  float bl; // baseline of the channel used for signal
  int blpt; // number of point used in the baseline
  
  unsigned long int nEntriesScope = scopeTree->_wavTree->GetEntries();
  unsigned long int maxEvt = 10000; // number of events used to find the right cycle between the oscilloscope and the tracking
  if(maxEvt > nEntriesScope)
    maxEvt = nEntriesScope;

  std::cout << " Finding events for trees synchronization" << std::endl;
  
  for(unsigned long int i = 0; i < maxEvt; ++i){ // first cycle on oscilloscope data to find the events above threshold
    scopeTree->_wavTree->GetEntry(i);
    
    if((i+1) % 1000 == 0 || (i+1) == maxEvt)
      std::cout << " Processing event " << i+1 << " / " << maxEvt << "                             \r" << std::flush;
    
    nTrigScope = trigNum(scopeTree->_channels[trigCh], scopeTree->_npt, scopeTree->_time[1] - scopeTree->_time[0]);

    if(i == 0)
      nTrigScopePrev = nTrigScope;

    if(cycleNumber(nTrigScopePrev, nTrigScope, cycleScope))// if there is a new cycle break, all the trigger numbers for alignment between trees must be in the same cycle
	break;

    bl = 0;
    blpt = 0;
    for(unsigned int ipt = 0; ipt < scopeTree->_npt; ++ipt){ // calculate baseline
      if(scopeTree->_time[ipt] >= blStop)
	break;
      
      if(scopeTree->_time[ipt] > blStart){
	bl += scopeTree->_channels[sigCh][ipt];
	blpt++;
      }
    }

    bl /= blpt;
    bl *= pol;
    
    for(unsigned int ipt = 0; ipt < scopeTree->_npt; ++ipt){ // find if signal is above threshold
      if(scopeTree->_time[ipt] >= sigStop)
	break;

      if(scopeTree->_time[ipt] > sigStart)
	if(scopeTree->_channels[sigCh][ipt] * pol - bl > thr){
	  trigList.push_back(nTrigScope);
	  break;
	}
    }

    nTrigScopePrev = nTrigScope;
  } // loop on events in oscilloscope tree, first cycle

  std::cout << "\n Found " << trigList.size() << " events above threshold" << std::endl;

  // output file
  std::string scopeFileName(argv[1]);
  std::string outFileName = scopeFileName.substr(0, scopeFileName.size() - 5); // assume that the root file ends in .root
  outFileName.append("_trackInfo.root");
  TFile* outFile = new TFile(outFileName.c_str(), "RECREATE");
  
  // prepare to read the track tree
  TFile* trackFile = TFile::Open(argv[2]);
  TTree* trackTree = (TTree*) trackFile->Get("tracks");
  Int_t srstriggerctr;
  Int_t srstimestamp;
  Int_t ntracks;
  Int_t tracknumber;
  Double_t trackchi2;
  Int_t ndetsintrack;
  std::vector<std::vector<double> >* hits = 0;
  std::vector<double>* distnextcluster = 0;
  std::vector<double>* totchanextcluster = 0;
  
  // Set branch addresses
  trackTree->SetBranchAddress("srstriggerctr",&srstriggerctr);
  trackTree->SetBranchAddress("srstimestamp",&srstimestamp);
  trackTree->SetBranchAddress("ntracks",&ntracks);
  trackTree->SetBranchAddress("tracknumber",&tracknumber);
  trackTree->SetBranchAddress("trackchi2",&trackchi2);
  trackTree->SetBranchAddress("ndetsintrack",&ndetsintrack);
  trackTree->SetBranchAddress("hits",&hits);
  trackTree->SetBranchAddress("distnextcluster",&distnextcluster);
  trackTree->SetBranchAddress("totchanextcluster",&totchanextcluster);

  long int nTrigTrack; // from the tracking tree
  long int nTrigTrackPrev; // trig number of previous event
  int cycleTrack = -1; // cycle of the tracking tree
  long int nTrigTrackCorr; // trigger number going from 0 to 2^nBits, to synchronize with the oscilloscope
  
  unsigned long int nEntriesTracks = trackTree->GetEntries();

  char name[50];
  char title[200];
  
  TH2I* hitmap;
  std::vector<TH2I*> hitVec;
  float x,y;
  bool match = false; // set to true if the trigger numbers match
  
  for(unsigned long int i = 0; i < nEntriesTracks; ++i){ // loop on the track tree
    trackTree->GetEntry(i);

    nTrigTrack = srstriggerctr;
    
    if(i == 0)
      nTrigTrackPrev = nTrigTrack + 1; // use this initialization to have a new cycle in the if below and initialize the hitmap

    if(cycleNumber(nTrigTrackPrev, nTrigTrack, cycleTrack)){ // if there is a new cycle, make a new hitmap
      sprintf(name, "hitmap_trackCycle%d", cycleTrack);
      sprintf(title, "Hitmap track cycle %d;X [mm];Y[mm];Entries", cycleTrack);
      hitmap = new TH2I(name, title, 1000, 0, 100, 1000, -50, 50);
      hitVec.push_back(hitmap);
      match = false;
    }
    
    nTrigTrackCorr = nTrigTrack % (int) pow(2, nBits);

    if(match && nTrigTrackPrev != nTrigTrack){ // if in the previous entry there was a match and the trigger number changed, the track was the only one of the event
      hitmap->Fill(x, y);
      match = false;
    }
    
    if(ntracks == 1 && distnextcluster->at(0) != 2000) // use only the first track of one event, it is checked in the next iteration that the track is the only one of the event, the distnextcluster is 2000 when the track is not reconstructed on the plane of interest
      for(unsigned int iTrigScope = 0; iTrigScope < trigList.size(); iTrigScope++) // check if the event is in the oscilloscope trigger list
	if(nTrigTrackCorr == trigList[iTrigScope]){ // find event with same number
	  x = hits->at(0)[0]; // use hits on plane 0, first plane of the telescope
	  y = hits->at(0)[1];
	  match = true;
	  break;
	}
    
    nTrigTrackPrev = nTrigTrack;
  } // loop on events of track tree


  // find the matching cycle between the   
  float minFig = 1e6;
  int nCycle;
  float fig; // variable to be minimized to associate the cycle 0 of the oscilloscope data to the cycle of the tracking
  for(unsigned int iCycle = 0; iCycle < hitVec.size(); ++iCycle){ // per construction the cycle number of the tracking corresponds to the position in the vector
    if(hitVec[iCycle]->GetStdDev(1) == 0 || hitVec[iCycle]->GetStdDev(2) == 0)
      continue;

    fig = hitVec[iCycle]->GetStdDev(1) + hitVec[iCycle]->GetStdDev(2);
    if(fig < minFig){
      minFig = fig;
      nCycle = iCycle;
    }
  }

  std::cout << " The tracking cycle corresponding to the 0 cycle of the oscilloscope data is " << nCycle << std::endl;
  
  outFile->cd();
  for(unsigned int i = 0; i < hitVec.size(); ++i)
    hitVec[i]->Write();

  // produce 2 trees, one with the track number of the oscilloscope, the other with the all the tracking info ordered with the trig number and cycle, so that they can be synched as friends
  // first tree
  TTree* trackHitTree = new TTree("trackHitTree_tmp", "");
  Int_t nTracks = 500; // number of tracks in the event, it is 0 if no hit is reconstructed
  Float_t DUThits[nTracks][nCh][3]; // hit position on the DUT planes (one for each oscilloscope channel)
  Double_t pars[nTracks][4]; // parameters of the tracks
  Double_t minFCN[nTracks]; // minimum of FCN for track refitting
  Double_t recoTrackChi2[nTracks]; // track fit chi2 from the reconstruction SW of the beam telescope (not related to pars)
  Int_t recoNdetsInTrack[nTracks]; // number of planes used in the track reconstruction (2 for each GEM plane, x and y treated separately)
  trackHitTree->Branch("cycle", &cycleTrack, "cycle/I");
  trackHitTree->Branch("nTrig", &nTrigTrackCorr, "nTrig/l");
  trackHitTree->Branch("nTracks", &nTracks, "nTracks/I");
  char prop[50];
  sprintf(prop, "hits[nTracks][%d][3]/F", nCh);
  trackHitTree->Branch("hits", DUThits, prop);
  trackHitTree->Branch("trackPar", pars, "trackPar[nTracks][4]/D");  
  trackHitTree->Branch("minFCN", minFCN, "minFCN[nTracks]/D");  
  
  trackHitTree->Branch("recoTrackChi2", recoTrackChi2, "recoTrackChi2[nTracks]/D");
  trackHitTree->Branch("recoNdetsInTrack", recoNdetsInTrack, "recoNdetsInTrack[nTracks]/I");
  
  cycleTrack = -nCycle; // so that cycleTrack 0 in the tree corresponds to cycleScope 0 in the next tree

  // fitting of 3d tracks from $ROOTSYS/tutorials/fit/line3Dfit.C
  // set up objects for fitting the tracks
  std::vector<std::vector<double>> pointVec;
  lineFit lineObj(&pointVec);
  ROOT::Fit::Fitter fitter;
  ROOT::Math::Functor fcn(lineObj,4);
  
  std::cout << " Creating temp tree with hits" << std::endl;
  
  for(unsigned long int i = 0; i < nEntriesTracks; ++i){ // loop on the track tree
    trackTree->GetEntry(i);

    nTrigTrack = srstriggerctr;
    
    if(i == 0)
      nTrigTrackPrev = nTrigTrack;

    cycleNumber(nTrigTrackPrev, nTrigTrack, cycleTrack);

    nTrigTrackCorr = nTrigTrack % (int) pow(2, nBits);

    nTracks = 0;
    
    do{ // group together all tracks in one event and extrapolate them to the DUT planes

      // refit the track to find parameters
      lineObj.clear(); // empty the container
      for(int iPlane = 0; iPlane < nRecPlanes; ++iPlane) // set the points for the fit
	lineObj.addPoint(hits->at(recPlanes[iPlane]));

      // use hits on first plane (z=0) to set offsets and set slopes to 0
      pars[nTracks][0] = hits->at(0)[0];
      pars[nTracks][1] = 0;
      pars[nTracks][2] = hits->at(0)[1];
      pars[nTracks][3] = 0;

      fitter.SetFCN(fcn, pars[nTracks]);

      if(fitter.FitFCN() != true) // do the fitting
	std::cout << "Problem in track fitting!!!" << std::endl;

      minFCN[nTracks] = fitter.Result().MinFcnValue();
      
      const double* res = fitter.Result().GetParams(); // retrieve the parameters
      for(int j = 0; j < 4; ++j)
	pars[nTracks][j] = res[j];
      
      for(int iCh = 0; iCh < nCh; ++iCh){ // calculate hits on the planes
	DUThits[nTracks][iCh][0] = pars[nTracks][0] + pars[nTracks][1] * dutPos[iCh];
	DUThits[nTracks][iCh][1] = pars[nTracks][2] + pars[nTracks][3] * dutPos[iCh];
	DUThits[nTracks][iCh][2] = dutPos[iCh];

	//std::cout << DUThits[nTracks][iCh][0] << "  " << DUThits[nTracks][iCh][1] << "  " << DUThits[nTracks][iCh][2] <<std::endl;
      }

      recoTrackChi2[nTracks] = trackchi2;
      recoNdetsInTrack[nTracks] = ndetsintrack;
      
      nTracks++;
      i++;
      if(i == nEntriesTracks)
	break;
      trackTree->GetEntry(i);
    }while(nTrigTrack == srstriggerctr);

    i--; // go back one event as the counter went one too far in the while loop
    trackTree->GetEntry(i);

    trackHitTree->Fill();
    
    nTrigTrackPrev = nTrigTrack;
  } // loop on events of track tree

  trackHitTree->BuildIndex("cycle", "nTrig");
  trackHitTree->Write();

  // second tree
  TTree* scopeTrigNumTree = new TTree("scopeTrigNumTree_tmp", "");
  ULong64_t event; // event number (same as in oscilloscope tree, used for synchronization)    
  scopeTrigNumTree->Branch("cycle", &cycleScope, "cycle/I");
  scopeTrigNumTree->Branch("nTrig", &nTrigScope, "nTrig/l");
  scopeTrigNumTree->Branch("event", &event, "event/l");

  std::cout << " Creating temp tree with trig numbers from oscilloscope" << std::endl;

  cycleScope = 0;
  
  for(unsigned long int i = 0; i < nEntriesScope; ++i){
    scopeTree->_wavTree->GetEntry(i);

    if((i+1) % 1000 == 0 || (i+1) == nEntriesScope)
      std::cout << " Processing event " << i+1 << " / " << nEntriesScope << "                             \r" << std::flush;
    
    nTrigScope = trigNum(scopeTree->_channels[trigCh], scopeTree->_npt, scopeTree->_time[1] - scopeTree->_time[0]);

    if(i == 0)
      nTrigScopePrev = nTrigScope;

    cycleNumber(nTrigScopePrev, nTrigScope, cycleScope);
    
    event = scopeTree->_event;

    scopeTrigNumTree->Fill();
    
    nTrigScopePrev = nTrigScope;
  }

  scopeTrigNumTree->Write();

  std::cout << std::endl;

  scopeTrigNumTree->AddFriend(trackHitTree);
  
  // produce the tree with the hits for each event of the oscilloscope
  TTree* hitTree = new TTree("hitTree", "hits on the detector planes");
  hitTree->Branch("event", &event, "event/l");
  hitTree->Branch("nTracks", &nTracks, "nTracks/I");
  sprintf(prop, "hits[nTracks][%d][3]/F", nCh);
  hitTree->Branch("hits", DUThits, prop);
  hitTree->Branch("trackPar", pars, "trackPar[nTracks][4]/D");  
  hitTree->Branch("minFCN", minFCN, "minFCN[nTracks]/D");  
  hitTree->Branch("recoTrackChi2", recoTrackChi2, "recoTrackChi2[nTracks]/D");
  hitTree->Branch("recoNdetsInTrack", recoNdetsInTrack, "recoNdetsInTrack[nTracks]/I");

  scopeTrigNumTree->SetBranchAddress("event", &event);
  trackHitTree->SetBranchAddress("nTracks", &nTracks);
  trackHitTree->SetBranchAddress("hits", DUThits);
  trackHitTree->SetBranchAddress("trackPar", pars);  
  trackHitTree->SetBranchAddress("minFCN", minFCN);  
  trackHitTree->SetBranchAddress("recoTrackChi2", recoTrackChi2);
  trackHitTree->SetBranchAddress("recoNdetsInTrack", recoNdetsInTrack);

  std::cout << " Creating tree with tracking data ordered per oscilloscope event" << std::endl;
  
  for(unsigned long int i = 0; i < nEntriesScope; ++i){
   scopeTrigNumTree->GetEntry(i);

    if((i+1) % 1000 == 0 || (i+1) == nEntriesScope)
      std::cout << " Processing event " << i+1 << " / " << nEntriesScope << "                             \r" << std::flush;

    hitTree->Fill();
  }
 
  std::cout << std::endl;
  
  hitTree->BuildIndex("event"); // to sync it with the oscilloscope wawe tree
  hitTree->Write();

  delete scopeTree;
  delete cfg;

  trackFile->Close();
  outFile->Close();
  
  return 0;
}
