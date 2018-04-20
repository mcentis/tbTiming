import time
import sys
sys.path.insert(0, '/home/daq/IsegSHQ222M')

import IsegSHQ222M

if len(sys.argv) != 2:
    print 'Usage: python printVoltageCurr.py port'
    exit(1)

hv = IsegSHQ222M.IsegSHQ222M(sys.argv[1])

print hv.GetModuleID()

try:
    while(True):
        print 'Ch1: %f\t%f\t\tCh2: %f\t%f' %(hv.GetMeasVoltage(1), hv.GetMeasCurrent(1), hv.GetMeasVoltage(2), hv.GetMeasCurrent(2))
        time.sleep(2)

except:
    print 'Bye!'
