#include <TTree.h>
#include <TFile.h>
#include <iomanip>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdio.h>
#include <string>

#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bitset>

#define MAX_SEQ 1000

static FILE* fd_txt = NULL;
static FILE *fd_dat = NULL;
static short int* fileBuf;
static int nch;
static int nevt_seq[MAX_SEQ];
static int nevt_tot;
static int nseq;
static float yscale[4];
static int npts_evt;
static float rate;
unsigned long int size;

int scope_data_init(std::string asciiFile);
int getAdcData(int evtnum, int chnum, float* buf, int maxpts);

int scope_data_decode_macro(std::string fname)
{
    int i;
    int ier;
    int ns;
    float buf[100000]; // big buffer in case of long acquisition
    ier = scope_data_init(fname);
    if (ier != 0) return 1;   
    printf("ATTENTION - All 4 channels are always filled, if in reality nch < 4, value 1000. is filled\n"); 
    printf("No. of Channels = %d nevt = %d\n", nch, nevt_tot);
    
    // create the ntuple
    // fname = fname.substr(fname.find_last_of("/\\") + 1);
    TString filename = fname + ".root";
    TFile ofile(filename.Data(),"RECREATE");
    TTree *tree = new TTree("wfm","Time part of the LGAD time measurement");
    
    // loop over channels
    int nsamples  = getAdcData(1, 2, buf, sizeof(buf)/sizeof(float));

    // with current oscilloscope, 4 is the maximal number of channels we can read out
    vector<float> chan1(nsamples);
    vector<float> chan2(nsamples);
    vector<float> chan3(nsamples);
    vector<float> chan4(nsamples);
    vector<float> time(nsamples);    
    for (int i = 0; i < nsamples; i++) time.at(i) = ((float)i) * rate; // 25 picoseconds per data point
    int ntrig = 0;
    int nsamp = 0;
    int evnr = 0;
    tree->Branch("Ch1",&chan1);
    tree->Branch("Ch2",&chan2);
    tree->Branch("Ch3",&chan3);
    tree->Branch("Ch4",&chan4);  
    tree->Branch("time", &time);
    tree->Branch("ntrig",&ntrig,"ntrig/I");
    tree->Branch("nsamp",&nsamp,"nsamp/I");
    tree->Branch("evnr",&evnr,"evnr/I");

    printf("Starting loop %d over events, number of samples per channel and event: %d \n",nevt_tot,nsamples);
    ntrig = nevt_tot;
    for (int i = 0; i < ntrig;i++)
        { 
         if (i%1000 == 0) printf("Processing event number: %d \n",i);
         // initialize branch each event
         for (int ij = 0; ij < nsamples; ij++)
             {
              chan1.at(ij) = 1000.;
              chan2.at(ij) = 1000.;
              chan3.at(ij) = 1000.;
              chan4.at(ij) = 1000.;
             }
         nsamp = nsamples;
         evnr = i;       
         for (int j = 0; j < nch; j++)
             {       
              ns = getAdcData(i, j, buf, sizeof(buf)/sizeof(float));           
              for (int ii=0; ii<ns; ii++)
                  {
                   if (j == 0) chan1.at(ii) = buf[ii];
                   if (j == 1) chan2.at(ii) = buf[ii];
                   if (j == 2) chan3.at(ii) = buf[ii];
                   if (j == 3) chan4.at(ii) = buf[ii];
                  }
             }      
         tree->Fill(); // fill every event
        }
    
    tree->Write();
    ofile.Close();
    //delete[]fileBuf;
    return 0;
}

int scope_data_init(std::string asciiFile)
{
    int i;
    long int lCurPos;
    char line[1024];
    char dt1[1024];
    char dt0[1024];
    int  nrec;
    asciiFile = asciiFile + ".txt";
   
    if (fd_txt != NULL) fclose(fd_txt);
    
    if (size > 0)
       {
	 //assert(adc_data != NULL);
	 //munmap(adc_data, size);
        size = 0;
       }
    
    nch = 0;
    for (i=0; i<MAX_SEQ; i++) nevt_seq[i] = 0;
    nevt_tot = 0;
    dt0[0] = '\0';
    npts_evt = 0;
    
    fd_txt = fopen(asciiFile.c_str(), "r");
    if (fd_txt == NULL)
       {
        perror(asciiFile.c_str());
        return 1;
       }
        
    nrec = 0;
    while (fgets(line, sizeof(line), fd_txt))
          {
           int nel, npts, nev;
           float yinc;
           nel = sscanf(line, "%*d,%*d,%d,%*d,%f,%*lf,%*f,%f,%*f,%*f,%*d,%*f,%*f,%*f,%*f,%*[^,],%[^,],%*[^,],%*d,%*d,%*d,%*d,%*f,%*f,%d",&npts, &rate, &yinc, dt1, &nev);
           if (npts_evt == 0) npts_evt = npts;
           assert(nel == 5 && npts > 0 && nev > 0 && npts == npts_evt);
           /* Figure out number of channels if not known */
           if (nch == 0)
              {
               if (dt0[0] == '\0')
                  {
                   memcpy(dt0, dt1, 1024);
                   nevt_seq[0] = nev;
                  }
               if (strncmp(dt0, dt1, 1024) == 0)
                  {
                   assert(nrec < 4);
                   yscale[nrec] = yinc;
                  }
               else nch = nrec;
              }
           else {
                 assert(nrec < nch * MAX_SEQ);
                 nevt_seq[nrec/nch] = nev;
                }
           nrec++;
          }

    if (nch == 0) nch = nrec;
    nseq = nrec/nch;
    assert(nrec == nch * nseq);
    for(i=0; i<nseq; i++) nevt_tot += nevt_seq[i];
    
    /* prep to access real wfm data */
     char *s;
     strncpy(line, asciiFile.c_str(), sizeof(line));
     s = strstr(line, ".txt");
     assert(s != NULL);
     strncpy(s, ".dat", 4);

    if ((fd_dat = fopen(std::string(line).c_str(), "rb")) == NULL) cout << "Could not open file " << std::string(line) << endl;
    else cout << "Oppening binary file: " << std::string(line) << endl;
    lCurPos = ftell(fd_dat);
    fseek(fd_dat, 0, 2);
    size = ftell(fd_dat);
    fseek(fd_dat, lCurPos, 0);
    if (size <= 0)
       {
        perror(line);
        return 1;
       }
    assert(size == npts_evt * nch * nevt_tot * 2 + nevt_tot * 8);
    // std::cout << size << std::endl;
    fileBuf = (short *) malloc(size);
    fread(fileBuf, size, 1, fd_dat);
    if (fileBuf == NULL)
       {
        perror("Buffer not correctly filled");
        return 1;
       }
    fclose(fd_dat);
    return 0;
}

int getAdcData(int evtnum, int chnum, float* buf, int maxpts)
{
  int iseq;
  int offst;
  int i;
  int npts_filled;
  if (evtnum < 0 || evtnum >= nevt_tot || chnum < 0 || chnum >= nch) return -1;
  offst = 0;
  for (iseq = 0; evtnum >= nevt_seq[iseq]; iseq++)
      {
       offst += (npts_evt * nch + 4) * nevt_seq[iseq];
       evtnum -= nevt_seq[iseq];
      }
  offst += npts_evt * (chnum * nevt_seq[iseq] + evtnum);
  // std::cout << npts_evt << " " << maxpts << std::endl;
  for (i = 0; i < npts_evt && i < maxpts; i++) 
      {
       buf[i] = fileBuf[offst + i] * yscale[chnum];
       // std::cout << fileBuf[offst + i] << std::endl;
      }
  npts_filled = i;
  return npts_filled;
}
