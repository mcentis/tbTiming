Collection of small programs and scripts for beam test of timing detectors.
The programs were developed for a setup comprising:
- Oscilloscope Agilent DSO9254A
- HV power supply Iseg SHQ 222M
- Keithley 2000 multimeter

The programs were developed and run on a Linux (Ubuntu) machine.
Usually programs and scripts need some arguments.
If they are launched without arguments (or with the wrong number of arguments), they output what the expected arguments are and terminate.

The communication with the HV power supply and multimiter is through an RS232 interface, while the oscilloscope communication is done through LAN.

--> scopeRun
    This program is used to take data using the oscilloscope.
    It takes as arguments the scope network name (or IP address) and the path to the folder where the data will be saved.
    The output of the program are two files: a text file and a binary file containing the data.
    The data is stored in the binary file, with the text file containing the information to decode the data.
    The file name used for the files contains the date and time when the measurement was launched.
    For the program to work correctly, the oscilloscope must be set in the conditions in which the data will be acquired.
    After checking the conditions, the oscilloscope must be set in segmented mode, with a sensible number of segments to be acquired.
    If there are segments already acquired in the scope, these will be downloaded and written in the files.
    The program connects to the scope, acquires the selected number of segments, and downloads them to the computer where it is run.
    The operation is repeated until a ctrl-C command is issued
    When the ctrl-C is received, the program waits for the current acquisition of segments to finish, and then it terminates.
    It is necessary to ensure that the oscilloscope receives triggers for the data acquisition.
    If the scope stops receiving triggers durig the aquisition, a reset of the oscilloscope is necessary.
    No operations can be made on the oscilloscope while the data acquisition with this program is running.

--> scopeRun_progressiveRunNum
    This program does the same operations as the scopeRun program.
    Only the name of the output files is different, as a progressive run number is used.
    This program looks into the folder where the data will be saved, and if there are already files written using the program, the file name will have a run number higher (by 1) than the highest present in the folder.

--> binToRoot
    This program takes as argument the run name (without the .dat or .txt extension) of the files produced by one of the scopeRun programs and puts the information in root trees.
    There is one tree named "preamble" dedicated to the information about the transfer of the data from the scope to the computer.
    Variables:
    - transfer: number of the "transfer" of the acquired segments in this run, counting from 0
    - nSeg: number of events (segments) moved in each transfer
    - dateTime: date and time counter for the transfer
    The waveform data is contained in the tree "waves".
    Variables:
    - npt: number of points in the waveforms
    - ch1[npt], ..., ch4[npt]: arrays with voltage values in volts of the waveforms. The numbering does not necessarily reflect the channel number of the oscilloscope.
      If less than 4 channels are used, these variables are used starting from ch1, and filled with the data from different channels of the scope, preserving the order.
      E.g. if channels 2 and 3 of the scope are used in the measurements, only the variables ch1 and ch2 of the tree will contain data.
       This is due to the fact that no information about the channel number is stored by the scopeRun programs.
    - time[npt]: array with the time values in seconds for the waveforms, equal for all channels.
    - transfer: see "preamble" tree, this variable can be used to sync the trees when making them friends
    - event: event counter for the run

--> filterWavesLowPass
    This program can be used to apply a low pass filter to the data contained in the root file produced by binToRoot.
    It creates a root file with the same trees as binToRoot, with the filtered data.
    To work, the program needs the cutoff frequency of the low pass filter and the number of waveform points used for applying the filter (the filter is like a weighted running average).

--> filterWavesRunAvg
    Same as "filterWavesLowPass" but it applies a running average filter.
    To work it needs the number of points used for the running average for each channel.

--> analyzeScopeData
    This program analyzes the data from binToRoot (or the filtered data).
    To work it needs a config file, with the structure of "confFileAnalyzeScopeData.cfg".
    The meaning of the parameters is explained in the config file.
    The program performs an automated analysis and also creates a tree with the parameters of the signal for each event.
    The numbering of the channels must be consistent with the one of the binToRoot output file.
    When not all channels are active, the automated analysis fails.
    The following description points out which are the parameters necessary to obtain a good determination of the signal parameters.
    The parameters are extracted using a two step selection:
    * First, baseline, noise, and signal peak time are determined using fixed regions of the waveforms, determined by baseStart, baseStop, signalStart, signalStop.
    * The second step determines the parameters (noise, baseline, amplitude, ...) that are stored in the tree.
      The region of the waveform used for this second step are determined with respect to the time where the maximum of the waveform is found.
      The regions are determined using blDuration, prePeak, postPeak
    All the variables listed above should be set to 0 for the channels that contain no waveform data.

    For the pulse parameters determination, the needed variables of the config file are:
    - termination: oscilloscope termination
    - polarity: signal polarity
    - baseStart, baseStop: extremes for baseline and noise calculation, in seconds and absolute time within the waveform
    - signalStart, signalStop: extremes for finding the signal, in seconds and absolute time within the waveform
    The next three parameters are used to extract signal properties using the part of the waveform around the peak (second step of the parameter extraction from above).
    The time considered will be |blDuration|prePeak|postPeak|.
    - blDuration: time in seconds used to determine the noise and baseline.
    - prePeak: time in seconds used to extract signal properties. The total signal time will be the sum of prePeak and postPeak
    - postPeak: time in seconds used to extract signal properties. The total signal time will be the sum of prePeak and postPeak
    - fractionThr: fraction threshold of the CFD algorithm.
      The time of crossing is determined by selecting the leading edge of the pulse and applying a linear interpolation between the point above and the point beow threshold.

    The variables stored in the tree "evtPropTree" are:
    - event: event counter
    - nCh: number of channels, usually set to 4
    - ampli[nCh]: array with the signal amplitude in volts
    - ampliTime[nCh]: time in seconds where the peak is found
    - baseline[nCh]: baseline value in volts, it is defined as the average of the waveform points in the blDuration interval
    - noise[nCh]: in volts, it is defined as the standard deviation of the waveform points in the blDuration interval around the baseline
    - riseTime[nCh]: 20 to 80% rise time in seconds
    - integral[nCh]: integral of the pulse in Coulomb for the time interval being the sum of prePeak and postPeak
    - linRegT0[nCh]: time of arrival determined using a linear interpolation of the leading edge of the pulse to 0, in seconds
    - tCFD[nCh]: threshold crossing time determined using a CFD algoritm, more details are given in the fractionThr explanation above
    When parameters can not be extracted, usually a value of 0 or 10 is stored in the tree.

--> other programs are used to merge the data streams of oscilloscope, beam telescope, and temperature and voltage logs.
    No detail will be given here, since it is quite unlikely that there will be another beam test with the same beam telescope any time soon.

--> macros/logTemperature.py
    This macro logs the temperature measured by a Pt1000 sensor connected to a Keithley 2000 multimeter as a function of time.
    The macro sets the instrument to measure resistance, and converts the measured resistance in temperature.
    To stop the macro use ctrl-C.

--> macros/logVoltageCurrent.py
    The macro logs the current and voltage supplied by the Iseg SHQ 222M as a function of time.
    To stop the macro use ctrl-C.

--> macros/plotTemperature.py
    Draws the temperature vs time from the file produced by logTemperature.py

--> macros/plotVoltageCurrent.py
    Draws the voltage and curent as a function of time from the file produced by logVoltageCurrent.py

--> macros/printTemperature.py
    Same as logTemperature.py, but it does not write a file.

--> macros/printVoltageCurrent.py
    Same as logVoltageCurrent.py, but it does not write a file.
