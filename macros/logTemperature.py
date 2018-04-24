import serial
import time
import datetime

import numpy as np
from scipy.optimize import minimize

import sys

if len(sys.argv) != 3:
    print '\tUsage python logTemperature port dataFolder'
    exit(-1)

port = sys.argv[1]
ser = serial.Serial(
    port=port,
    baudrate=9600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS
)

if ser.isOpen() == False:
    print 'Error: Could not open ' + port
    exit(1)
waitTime = 0.2 #[s] time for the instrument to receive (and process) the instruction
    
def write(command):
    ser.write(command + '\r\n')
    time.sleep(waitTime) # time for the instrument to receive (and process) the instruction
        
def read():
    out = ''
    while ser.inWaiting() > 0:
        out += ser.read(1)
    return out

def resPt1000(temp):
    # coefficients and equation from http://www.thermometricscorp.com/pt1000
    r0 = 1000
    a = 3.9083e-3
    b = -5.775e-7
    if(temp >= 0):
        c = 0
    else:
        c = -4.183e-12

    r = r0 * (1 + a * temp + b * pow(temp, 2) + c * (temp - 100) * pow(temp, 3))
    
    return r

def tempPt1000(res):
    tomin = lambda temp: pow(res - resPt1000(temp), 2)
    t0 = 0
    t = minimize(tomin, t0)
    return t.x[0]

write('*IDN?')
print read()

write('*RST')
write('FUNC \'RES\'')
write('RES:MODE AUTO')
write('RES:RANG 2e3')
write(':SYSTem:RSENse OFF') # 2 wires readout, set to ON for 4 wires readout
write(':FORM:ELEM RES')
write(':SOUR:CLE:AUTO ON')
write(':OUTput ON')

# write('*RST')
# write(':SENS:FUNC \'RES\'')
# write(':SENS:RES:NPLC 1')
# write(':SENS:RES:MODE MAN')
# write(':SYSTem:RSENse OFF') # 2 wires readout, set to ON for 4 wires readout
# write(':SOUR:FUNC CURR')
# write(':SOUR:CURR 0.001')
# write(':SOUR:CLE:AUTO ON')
# write(':SENS:VOLT:PROT 2.5')
# write(':TRIG:COUN 1')
# write(':FORM:ELEM RES')

# setup output file
startStr = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
filename = sys.argv[2]+'/'+startStr+'_logTemperature.txt'
print 'Logfile: '+filename

logFile = open(filename, 'w')

print 'Current reading:'

try:
    while(True):
        write(':READ?')
        res = float(read())
        temp = tempPt1000(res)
        tstamp = datetime.datetime.now().strftime("%s") # unix time stamp, GMT time
        logFile.write('%s\t%.2f\n' %(tstamp, temp))
        sys.stdout.write('\r%s\t%.2f' %(datetime.datetime.now().strftime("%Y-%m-%d %H-%M-%S"), temp))
        sys.stdout.flush()
        time.sleep(2)
except:
    write(':SOURce:CLEar')
    write(":OUTput?")
    print '\nOutuput state: %s' % read()
    logFile.close()
    print 'Bye!'
