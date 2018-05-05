from ROOT import *

import sys

gStyle.SetLabelSize(0.05, 'xy')
gStyle.SetTitleSize(0.05, 'xy')
gStyle.SetTitleOffset(0.9, 'x')
gStyle.SetTitleOffset(0.95, 'y')

if len(sys.argv) != 2:
    print '\tUsage: python plotTemperature logFile'
    exit(1)

tempGr = TGraph(sys.argv[1])
tempGr.SetFillColor(kWhite)
tempGr.SetLineWidth(2)

can = TCanvas()
can.SetGrid()
tempGr.Draw('apl')
tempGr.GetXaxis().SetTimeDisplay(1)
tempGr.GetXaxis().SetTitle('Time')
tempGr.GetYaxis().SetTitle('Temperature [C]')

raw_input('...')
