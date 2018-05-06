from ROOT import *

import sys

gStyle.SetLabelSize(0.05, 'xy')
gStyle.SetTitleSize(0.05, 'xy')
gStyle.SetTitleOffset(0.9, 'x')
gStyle.SetTitleOffset(0.95, 'y')

def removeOverflow(gr):
    n = gr.GetN()
    x = gr.GetX()
    y = gr.GetY()

    # set only the points not in overflow
    ipt = 0
    for i in range(n):
        if y[i] < -10000:
            continue
        gr.SetPoint(ipt, x[i], y[i])
        ipt += 1
        
    # remove the points after the ones that are set in previous loop
    for i in range(ipt, n):
        gr.RemovePoint(i)
        
    return

if len(sys.argv) != 2:
    print '\tUsage: python plotVoltageCurrent logFile'
    exit(1)

v1Gr = TGraph(sys.argv[1], '%lg %lg %*lg %*lg %*lg')
v1Gr.SetTitle('V1')
v1Gr.SetFillColor(kWhite)
v1Gr.SetLineColor(kRed)
v1Gr.SetLineStyle(1)
v1Gr.SetLineWidth(2)
v1Gr.SetMarkerColor(kRed)
removeOverflow(v1Gr)

i1Gr = TGraph(sys.argv[1], '%lg %*lg %lg %*lg %*lg')
i1Gr.SetTitle('I1')
i1Gr.SetFillColor(kWhite)
i1Gr.SetLineColor(kRed)
i1Gr.SetLineStyle(2)
i1Gr.SetLineWidth(2)
i1Gr.SetMarkerColor(kRed)
removeOverflow(i1Gr)

v2Gr = TGraph(sys.argv[1], '%lg %*lg %*lg %lg %*lg')
v2Gr.SetTitle('V2')
v2Gr.SetFillColor(kWhite)
v2Gr.SetLineColor(kBlue)
v2Gr.SetLineStyle(1)
v2Gr.SetLineWidth(2)
v2Gr.SetMarkerColor(kBlue)
removeOverflow(v2Gr)

i2Gr = TGraph(sys.argv[1], '%lg %*lg %*lg %*lg %lg')
i2Gr.SetTitle('I2')
i2Gr.SetFillColor(kWhite)
i2Gr.SetLineColor(kBlue)
i2Gr.SetLineStyle(2)
i2Gr.SetLineWidth(2)
i2Gr.SetMarkerColor(kBlue)
removeOverflow(i2Gr)

vMult = TMultiGraph()
vMult.Add(v1Gr)
vMult.Add(v2Gr)
vMult.SetTitle(sys.argv[1])

iMult = TMultiGraph()
iMult.Add(i1Gr)
iMult.Add(i2Gr)
iMult.SetTitle(sys.argv[1])

vCan = TCanvas()
vCan.SetGrid()
vMult.Draw('apl')
vMult.GetXaxis().SetTimeDisplay(1)
vMult.GetXaxis().SetTitle('Time')
vMult.GetYaxis().SetTitle('Voltage [V]')
vCan.BuildLegend().SetLineColor(kWhite)

iCan = TCanvas()
iCan.SetGrid()
iMult.Draw('apl')
iMult.GetXaxis().SetTimeDisplay(1)
iMult.GetXaxis().SetTitle('Time')
iMult.GetYaxis().SetTitle('Current [A]')
iCan.BuildLegend().SetLineColor(kWhite)


raw_input('...')
