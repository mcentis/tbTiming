#ifndef SCOPETREEINTERFACE_HH
#define SCOPETREEINTERFACE_HH

#include "TTree.h"

class ScopeTreeInterface{
public:
  ScopeTreeInterface(const char* fileName);
  ~ScopeTreeInterface();

  static const int _inCh = 4; // number of channels in the root file, to be matched with binToRoot outCh
  
private:
  TFile* _inFile; // input file
  
  TTree* _wavTree; // tree with waveform data
  UInt_t _npt; // number of points in each waveform
  Float_t* _channels[_inCh]; // data from the scope channels (vertical)
  Float_t* _time; // time axis of the data
  UInt_t _transfer; // "transfer" number from the scope
  ULong64_t _event; // event number

  TTree* _preTree; // tree with waveform preamble data
  ULong64_t _dateTime; // timestamp of segments acquisition
  UInt_t _nSeg; // number of segments in a "transfer"
  
};

#endif//#ifndef SCOPETREEINTERFACE_HH
