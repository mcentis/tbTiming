import time
import datetime
import sys
sys.path.insert(0, '/home/daq/IsegSHQ222M')

import IsegSHQ222M

if len(sys.argv) != 3:
    print 'Usage: python logVoltageCurr.py port dataFolder'
    exit(1)

hv = IsegSHQ222M.IsegSHQ222M(sys.argv[1])

print hv.GetModuleID()

# setup output file
startStr = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
filename = sys.argv[2]+'/'+startStr+'_logVoltageCurrent.txt'
print 'Logfile: '+filename

logFile = open(filename, 'w')

print 'Current reading:'

try:
    while(True):
        v1 = hv.GetMeasVoltage(1)
        i1 = hv.GetMeasCurrent(1)
        v2 = hv.GetMeasVoltage(2)
        i2 = hv.GetMeasCurrent(2)
        tstamp = datetime.datetime.now().strftime("%s") # unix time stamp, GMT time
        logFile.write('%s\t%f\t%f\t\t%f\t%f\n' %(tstamp, v1, i1, v2, i2))
        sys.stdout.write('\r%s Ch1: %f\t%f\t\tCh2: %f\t%f' %(datetime.datetime.now().strftime("%Y-%m-%d %H-%M-%S"), v1, i1, v2, i2))
        sys.stdout.flush()
        time.sleep(2)

except:
    logFile.close()
    print 'Bye!'
