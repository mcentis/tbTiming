#ifndef WAVEFORMPREAMBLE_HH
#define WAVEFORMPREAMBLE_HH

#include <ctime>
#include <string>

/**
 * Classs containing the preamble information for one channel
 * For more information see the oscilloscope programmer's guide
 * command: :WAVeform:PREamble?
 **/

class waveformPreamble{
public:
  waveformPreamble();
  bool readString(std::string line);
  ~waveformPreamble();
  
private:
  int _format; // data format, ascii, binary, ...
  int _type; // raw, average, ...
  int _points; // number of points in the waveform
  int _count; // number of avrages acquired
  float _xIncrement; // time increment between points
  float _xOrigin; // origin of xaxis
  float _xReference; // always 0 for this oscilloscope
  float _yIncrement; // voltage increment between points
  float _yOrigin; // origin of xaxis
  float _yReference; // always 0 for this oscilloscope
  int _coupling; // channel coupling
  float _xDisplayRange; // range shown on scope
  float _xDisplayOrigin; // time orignin on display
  float _yDisplayRange; // "dynamic" range shown on scope
  float _yDisplayOrigin; // voltage orignin on display
  char* _date;
  char* _time; // check if it is the start or the end time of the acquisition
  char* _frameModel; // frame and model number
  tm* _dateTime; // combine the information from the strings, second precision, unix time (not local)
  int _acquisitionMode; // real time, segmented, ...
  int _completion; // percent of completed time buckets
  int _xUnits; // V, s, dB, ...
  int _yUnits; // V, s, dB, ...
  float _maxBWlimit; // upper bandwidth limit
  float _minBWlimit; // lower bandwidth limit
  int _segmentCount; // how many waveforms were acquired
  
};

#endif//#ifndef WAVEFORMPREAMBLE_HH

