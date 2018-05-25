#include "inetClient.hh"

#include <iostream>
#include <string.h> // strlen
#include <stdio.h> // sprintf
#include <stdlib.h> // atoi
#include <unistd.h> // sleep
#include <sys/stat.h> // open
#include <fcntl.h> // open
#include <signal.h> // stuff to catch ctrl-c
//#include <dirent.h> // dirent
#include <ctime>
#include <string>

// global variables
inetClient* scope;
char* buf;
const int nCh = 4; // four channels
bool chStatus[nCh] = {0}; // active or inactive
char comm[500]; // array to contain commands to scope, when needed
long int events = 0; // total number of events
int fd_txt = 0; // file descriptor text file
int fd_bin = 0; // file descriptor binary file

void initScope(){ // setup the scope for acquisition
  // scope ID
  scope->SendCString("*IDN?\n");
  scope->Receive();
  std::cout << "Scope ID:\n" << buf;

  // deactivate header for query responses
  scope->SendCString(":SYST:HEAD 0\n");

  // waveforms transmitted in bynary, least significant bit first, streaming on, download all segment in one query
  scope->SendCString(":WAV:FORM BIN;BYT LSBF;STR 1;SEGM:ALL 1\n");

  // check active channels
  for(int i = 0; i < nCh; i++){
    sprintf(comm, ":STAT? CHAN%i\n", i+1);
    scope->Send(comm, strlen(comm));
    scope->Receive();
    chStatus[i] = (bool) atoi(buf); // works due to \n after first char
  }

  std::cout << "Active channels:" << std::endl;
  for(int i = 0; i < nCh; i++)
    std::cout << "Ch " << i+1 << ": " << chStatus[i] << std::endl;  
  
  return;
}

void recWriteScopeData(int fd, long int expected){ // file descriptor and expected length
  long int len; // amount of received data
  long int tot = 0;
  int recCount = 0; // count how many times the Receive function must be used to get the data
  int missing = expected;  // amount of data to be received yet
  int headerBytes = 2; // variable since it makes code easyer
  int trailerBytes = 0; // variable since it makes code easyer

  do{
    len = scope->Receive();
    recCount += 1;
    tot += len;
    
    missing -= len;
    if(missing == 0) // set the trailer bytes only at the last iteration
      trailerBytes = 1;
    write(fd, buf + headerBytes, len - headerBytes - trailerBytes); // exclude the header bytes by "moving" the pointer of the second argument, the length is set correspondingly
    headerBytes = 0; // the header is only received once
  }while(tot < expected);
  
  // std::cout << "Bytes received " << tot << std::endl;
  // std::cout << "Bytes expected " << expected << std::endl;
  // std::cout << "Receive calls " << recCount << std::endl;

  if(missing != 0)
    std::cerr << "Error while receiving data!" << std::endl;
  
  return;
};

void acquisition(){
  // start acquisition 
  scope->SendCString(":DIGitize\n");

  do{
    scope->SendCString(":ASTate?\n"); // acquisition state
    scope->Receive();
    //sleep(1);
    //std::cout << '.';
  }while(buf[1] != 'D'); // status ADONE means the acquisition is done

  scope->SendCString(":WAVeform:SEGMented:COUNt?\n"); // get how many waveforms are there
  scope->Receive();
  int nSeg = atol(buf);
  //  std::cout << "\rAcquired " << nSeg << " events                    " << std::flush;
  events += nSeg;

  std::cout << "\rDownloading " << nSeg << " events              " << std::flush;
  
  // download the data
  long int len;// amount of received data
  long int expected; // expected amount of data
  int nPts; // number of points wf
  int format; // wf format (ASCII, BINARY, ...)
  int type; // type of data (RAW, AVERAGED, HISTO, ...)
  for(int i = 0; i < nCh; i++)
    if(chStatus[i]){ // ask only about the active channels
      // select which channel is gonna be downloaded (source) and ask for the status of the channel (pre?)
      sprintf(comm, ":WAVeform:SOURce CHANnel%d;PRE?\n", i+1);
      scope->Send(comm, strlen(comm));
      len = scope->Receive();
      write(fd_txt, buf, len); // write conditions in the txt file
      
      sscanf(buf, "%d,%d,%d", &format, &type, &nPts); // get how many points per wf
      expected = nSeg * nPts * 2 * sizeof(char) + 3; // expected amount of data: number of segments * number of points * 2 bytes + 3

      // download the data
      scope->SendCString(":WAV:DATA?\n");
      recWriteScopeData(fd_bin, expected);
    }

  // get the time tag of each segment (same for all channels)
  scope->SendCString(":WAV:SEGM:XLIS? TTAG\n");
  expected = nSeg * sizeof(double) + 3;
  recWriteScopeData(fd_bin, expected);

  //  std::cout << "\rDownload complete               " << std::flush;
  std::cout << "\rTotal: " << events << " events               " << std::flush;

  return;
}

int main(int argc, char* argv[]){

  if(argc != 3){
    std::cout << "\t Usage: scopeRun scopeAddress runFolder" << std::endl;
    return -1;
  }

  // check that we are ready
  char rep;
  std::cout << "Launch run? (Check list complete?) [y to continue, any other key to abort]" << std::endl;
  std::cin >> rep;
  if(rep != 'y')
    return 0;

  long int bufferSize = 1000000000; // ~ 100 MB to contain all wf data in one go, if needed
  int port = 5025;
  scope = new inetClient(bufferSize);
  buf = scope->GetBufferPointer();
  scope->Connect(argv[1], port);

  initScope();

  // find time for file name
  time_t rawtime;
  tm* timeinfo;
  char timeStr[80];
  time (&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(timeStr,sizeof(timeStr),"%Y-%m-%d_%H-%M-%S",timeinfo);

  std::cout << "Run number: "<< timeStr << std::endl;
  std::cout << '\n';
  
  // open binary file for data
  sprintf(comm, "%s/%s_run.dat", argv[2], timeStr);
  fd_bin = open(comm, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (fd_bin < 0){
    perror(comm);
    return -1;
  }

  // open text file for the conditions (npt for waveform, number of segments, etc)
  sprintf(comm, "%s/%s_run.txt", argv[2], timeStr);
  fd_txt = open(comm, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (fd_txt < 0){
    perror(comm);
    return -1;
  }

  // intercept ctrl-c signal,
  sigset_t signalMask;
  sigemptyset(&signalMask); // initialize signalMask as empty
  sigaddset(&signalMask, SIGINT);  // put the the interrupt (ctrl-c) signal in signalMask
  if(sigprocmask(SIG_BLOCK, &signalMask, NULL) < 0) // block signals in signalMask, they will be pending, if this fails, print error
    perror("sigprocmask()");

  do{
    acquisition();

    if (sigpending(&signalMask) < 0) // put pending signals in signalMask, if this fails, print error
      perror("sigpending()");
    
  }while(sigismember(&signalMask, SIGINT) == 0); // the ctrl-c is received (contained in signalMask, 1 is returned)
  std::cout << '\n';
  
  close(fd_bin); // close binary file
  close(fd_txt); // close text file
  delete scope;
  
  return 0;
}
