#include "waveformPreamble.hh"

#include <iostream>
#include <string>
#include <fstream> // std::ifstream

#include "TTree.h"
#include "TFile.h"

int findActiveChannels(std::ifstream& inFile, waveformPreamble* wfPre)
{
  // use time stamp to determine number of the active channels, the active channels start the acquisition at the same time
  int nch = 0;
  char oldTime[100];
  std::string line;
  do{
    std::getline(inFile, line);
    wfPre->readString(line);
    if(nch == 0) // first iteration, fill oldTime for first time
      strcpy(oldTime, wfPre->_time);

    if(strcmp(oldTime, wfPre->_time) != 0)
      break;

    nch++;
    strcpy(oldTime, wfPre->_time);
    
  }while(inFile.good());

  return nch;
}

int main(int argc, char* argv[])
{
  if(argc != 2){
    std::cout << "\tUsage binToRoot runName" << std::endl;
    return -1;
  }

  const int outCh = 4; // number of channels used in producing the root file
  const int ptSize = 2 * sizeof(char); // size of a datapoint in the binary file (2 bytes)
  
  std::cout << "Number of channels filled in tree: " << outCh << std::endl;
  
  char txtFileName[100];
  char binFileName[100];
  char outFileName[100];
  strcpy(txtFileName, argv[1]);
  strcpy(binFileName, argv[1]);
  strcpy(outFileName, argv[1]);

  strcat(txtFileName, ".txt");
  strcat(binFileName, ".dat");
  strcat(outFileName, ".root");

  std::ifstream txtFile(txtFileName, std::ifstream::in); // open ascii file
  if(txtFile.good() == false){
    perror(txtFileName);
    return -1;
  }

  waveformPreamble* testChPre = new waveformPreamble();
  const int nCh = findActiveChannels(txtFile, testChPre); // get number of active channels, also testChPre stores info common to all channels
  std::cout << "Found " << nCh << " active channels" << std::endl;
  txtFile.seekg(0, txtFile.beg); // put the get "cursor" to the beginning of the file

  if(nCh > outCh){
    std::cout << "Found more channels than the ones contained in the output file (" << outCh << ")" << std::endl;
    txtFile.close();
    return -1;
  }

  std::ifstream binFile(binFileName, std::ifstream::in | std::ios::binary); // open binary file
  if(binFile.good() == false){
    perror(binFileName);
    return -1;
  }

  TFile* outFile = TFile::Open(outFileName, "RECREATE");
  TTree* wavTree = new TTree("waves", "");
  
  UInt_t npt = 1e6; // big number to allocate enough space when initializing channels
  Float_t* channels[outCh]; // waveforms y info
  for(int i = 0; i < outCh; ++i){
    channels[i] = new Float_t[npt];
    for(unsigned int ipt = 0; ipt < npt; ++ipt)
      channels[i][ipt] = -999; // initialize to dummy value in case the channel is not used
  }

  Float_t time[npt]; // waveforms x info
  UInt_t transfer = 0; // "transfer" number from oscilloscope
  ULong_t event = 0; // event number

  wavTree->Branch("npt", &npt, "npt/i");
  char brName[100];
  char brNameType[100];
  for(int i = 0; i < outCh; ++i){
    sprintf(brName, "ch%d", i+1);
    sprintf(brNameType, "ch%d[npt]/F", i+1);
    wavTree->Branch(brName, channels[i], brNameType);
    }
  wavTree->Branch("time", time, "time[npt]/F");
  wavTree->Branch("tansfer", &transfer, "transfer/i");
  wavTree->Branch("event", &event, "event/l");
  
  waveformPreamble* chPre[nCh]; // objects used to read txt file and determine quantities: number of points, number of waveforms, ...
  short int* buffer[nCh]; // buffers to store the info from the binary file. each should contain all the info of one channel in one "transfer" from the scope. Use short int since this is the format used by the scope
  int bufSize = (int) testChPre->_points * testChPre->_segmentCount * 1.5;  // 50% "safety factor" ptSize not used here since it corresponds to the size of one short int
  for(int i = 0; i < nCh; ++i){
    chPre[i] = new waveformPreamble();
    buffer[i] = new short int[bufSize];
  }

  bufSize = (int) testChPre->_segmentCount * 1.5; // 50% "safety factor"
  Double_t* bufTtag = new Double_t[bufSize]; // buffer for time stamps of all segments of one transfer
  
  std::string line; // to read txt file
  bool exitLoop = false;
  while(txtFile.good() && binFile.good()){
    for(int i = 0; i < nCh; ++i){ // store the data of one "transfer" in the buffers for all active channels
      std::getline(txtFile, line);
      if(txtFile.good() == false){ // usually here due to end of file
	exitLoop = true;
	break;
      }
      chPre[i]->readString(line);
      binFile.read((char*) buffer[i], chPre[i]->_points * ptSize * chPre[i]->_segmentCount); // put the segments of one channel into its buffer. Cast the pointer to char* to comply with the read function.
      if(binFile.good() == false){
	exitLoop = true;
	break;
      }
    }

    // read the time tags of the segments
    binFile.read((char*) bufTtag, chPre[0]->_segmentCount * sizeof(char) * 8); // buffer the time tags of the segments
    if(binFile.good() == false)
      exitLoop = true;

    if(exitLoop) break; // exit from loop if there was a problem reading one of the files or end of file is reached
    
    npt = chPre[0]->_points;

    for(unsigned int ipt = 0; ipt < npt; ipt++)// time axis same for all segments
      time[ipt] = chPre[0]->_xIncrement * ipt;
    
    for(int iSeg = 0; iSeg < chPre[0]->_segmentCount; ++iSeg){
      for(int iCh = 0; iCh < nCh; ++iCh){// read the data from the buffers
	for(unsigned int ipt = 0; ipt < npt; ipt++){// loop on points of wf
	  channels[iCh][ipt] = buffer[iCh][ipt + iSeg * npt] * chPre[iCh]->_yIncrement + chPre[iCh]->_yOrigin;
	}
      }
      if(event % 1000 == 0)
    	std::cout << " Processing event " << event << "               \r" << std::flush;
      
      wavTree->Fill();
      event++;
    } // loop on segments
    transfer++;
  }
  
  std::cout << std::endl;
  std::cout << "Processed\n";
  std::cout << event << " events\n";
  std::cout << transfer << " transfers\n";

  wavTree->Write();
  outFile->Close();
  
  txtFile.close();
  binFile.close();
  delete testChPre;
  for(int i = 0; i < nCh; ++i)
    delete[] buffer[i];

  for(int i = 0; i < outCh; ++i)
    delete[] channels[i];
    
  return 0;
}
