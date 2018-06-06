from ROOT import *

import sys

def findMax(wav, pol, base):
    mw = -5e3;
    for w in wav:
        a = (w - base) * pol 
        if a > mw:
            mw = a
    return mw

if len(sys.argv) != 2:
    print "\tUsage: python plotAmpliDistr.py fileName"
    exit(1)

inFile = TFile.Open(sys.argv[1])

pol = [-1, -1, -1, -1]

for p in pol:
    p /= abs(p) # normalize

hampli1 = TH1F("hAmpli1", "Ch1;Amplitude [V];Entries", 240, -0.2, 1) # 5 mV binning
hampli2 = TH1F("hAmpli2", "Ch2;Amplitude [V];Entries", 240, -0.2, 1) # 5 mV binning
hampli3 = TH1F("hAmpli3", "Ch3;Amplitude [V];Entries", 240, -0.2, 1) # 5 mV binning
hampli4 = TH1F("hAmpli4", "Ch4;Amplitude [V];Entries", 240, -0.2, 1) # 5 mV binning

nbase = 100. # number of points used for baseline estimation

nevt = inFile.waves.GetEntries()

for event in inFile.waves:
    if event.event % 1000 == 0:
        sys.stdout.write('\rEvent %d/%d                ' %(event.event, nevt))
        sys.stdout.flush()


    base = [0, 0, 0, 0]
    for i in range(int(nbase)): # baseline
            base[0] += event.ch1[i] /nbase
            base[1] += event.ch2[i] /nbase
            base[2] += event.ch3[i] /nbase
            base[3] += event.ch4[i] /nbase
        
    ampli1 = findMax(event.ch1, pol[0], base[0])
    ampli2 = findMax(event.ch2, pol[1], base[1])
    ampli3 = findMax(event.ch3, pol[2], base[2])
    ampli4 = findMax(event.ch4, pol[3], base[3])
    
    hampli1.Fill(ampli1)
    hampli2.Fill(ampli2)
    hampli3.Fill(ampli3)
    hampli4.Fill(ampli4)

can1 = TCanvas()
can1.SetLogy()
hampli1.Draw()

can2 = TCanvas()
can2.SetLogy()
hampli2.Draw()

can3 = TCanvas()
can3.SetLogy()
hampli3.Draw()

can4 = TCanvas()
can4.SetLogy()
hampli4.Draw()

print '\n'
raw_input('...')
