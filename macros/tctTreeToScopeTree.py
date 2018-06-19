from ROOT import *
import sys
from array import array

if len(sys.argv) != 2:
    print '\tUsage: tctTreeToScopeTree tctRootFile'
    exit(1)

inFile = TFile.Open(sys.argv[1])

# 4 channels will be created, ch1 and ch2 will contain the data

npt = array('I', [int(1e6)])

ch1 = array('f', npt[0] * [-999])
ch2 = array('f', npt[0] * [-999])
ch3 = array('f', npt[0] * [-999])
ch4 = array('f', npt[0] * [-999])

time = array('f', npt[0] * [-999])

event = array('L', [0])
transfer = array('I', [0])

dateTime = array('L', [0])
nSeg = array('I', [0])

outFile = TFile.Open('outScopeTree.root', 'RECREATE')
wavTree = TTree('waves', 'Waveform data')
preTree = TTree('preamble', 'Waveform preamble data')

wavTree.Branch('npt', npt, 'npt/i')
wavTree.Branch('ch1', ch1, 'ch1[npt]/F')
wavTree.Branch('ch2', ch2, 'ch2[npt]/F')
wavTree.Branch('ch3', ch3, 'ch3[npt]/F')
wavTree.Branch('ch4', ch4, 'ch4[npt]/F')
wavTree.Branch('time', time, 'time[npt]/F')
wavTree.Branch('transfer', transfer, 'transfer/i')
wavTree.Branch('event', event, 'event/l')

preTree.Branch('transfer', transfer, 'transfer/i')
preTree.Branch('nSeg', nSeg, 'nSeg/i')
preTree.Branch('dateTime', dateTime, 'dateTime/l')

for evt in inFile.TCTtree:
    if evt.set != 0: # copy only one set, the number 0
        break
    
    event[0] = evt.event
    npt[0] = 0
    for i in range(evt.npt):
        # if i % 2 == 0: # to copy only every other point
        #     continue
        npt[0] += 1
        time[i] = evt.time1[i]
        ch1[i] = evt.trace1[i]
        ch2[i] = evt.trace2[i]

    wavTree.Fill()
    preTree.Fill()

preTree.BuildIndex('transfer')
    
outFile.Write()
outFile.Close()
