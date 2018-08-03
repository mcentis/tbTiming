#include <iostream>

#include "ConfigFileReader.hh"
#include "ScopeTreeInterface.hh"

#include "TTree.h"

const int nBits = 16; // number of bits in the beam telescope trigger counter

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

// function to calculate the cycle number, cycle start from 0
// returns tue if the cycle number changes
// a cycle is a block of 65536 events, after which the bit pattern of the beam telescope starts again from 0
// put prevTrigNum = trigNum to not augment the cycle if nothing should happen (maybe in first entry of a tree)
bool cycleNumber(long int prevTrigNum, long int trigNum, int& cycle)
{  
  if(prevTrigNum > trigNum){ // restarted counting, or new tracking file in a chain
    cycle++;
    return true;
  }

  if(trigNum % (int) pow(2, nBits) == 0){ // trigger number from the beam telescope tree can be bigger than the 65536 expected from a 16 bit pattern, all numbers should be there for a run
    cycle++;
    return true;
  }
    
  return false;
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
  
  long int nTrigScope; // from the oscilloscope waveform
  long int nTrigScopePrev;
  // long int nTrigTrack; // from the tracking tree
  // long int nTrigTrackPrev;
  int cycleScope = 0; // cycle of the oscilloscope trigger
  // int cycleTrack = 0; // cycle of the tracking tree
  
  std::vector<long int> trigList; // list of trig number of events above threshold for the selected channel

  float bl; // baseline of the channel used for signal
  int blpt; // number of point used in the baseline
  
  unsigned long int nEntries = scopeTree->_wavTree->GetEntries();
  unsigned long int maxEvt = 10000; // number of events used to find the right cycle between the oscilloscope and the tracking
  if(maxEvt > nEntries)
    maxEvt = nEntries;

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
  } // loop on events

  std::cout << "\n Found " << trigList.size() << " events above threshold" << std::endl;
    
  delete scopeTree;
  delete cfg;
    
  return 0;
}
