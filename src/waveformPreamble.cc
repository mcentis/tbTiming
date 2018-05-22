#include "waveformPreamble.hh"

#include <sstream>

waveformPreamble::waveformPreamble()
{
  _date = new char(20);
  _time = new char(20);
  _frameModel = new char(50);
  
  return;
}

waveformPreamble::~waveformPreamble()
{
  delete _date;
  delete _time;
  delete _frameModel;
  
  return;
}

bool waveformPreamble::readString(std::string line)
{
  std::stringstream sstr;
  sstr.clear();
  sstr.str(line);
  
  sstr >> _format; // data format, ascii, binary, ...
  sstr >> _type; // raw, average, ...
  sstr >> _points; // number of points in the waveform
  sstr >> _count; // number of avrages acquired
  sstr >> _xIncrement; // time increment between points
  sstr >> _xOrigin; // origin of xaxis
  sstr >> _xReference; // always 0 for this oscilloscope
  sstr >> _yIncrement; // voltage increment between points
  sstr >> _yOrigin; // origin of xaxis
  sstr >> _yReference; // always 0 for this oscilloscope
  sstr >> _coupling; // channel coupling
  sstr >> _xDisplayRange; // range shown on scope
  sstr >> _xDisplayOrigin; // time orignin on display
  sstr >> _yDisplayRange; // "dynamic" range shown on scope
  sstr >> _yDisplayOrigin; // voltage orignin on display
  sstr >> _date;
  sstr >> _time; // check if it is the start or the end time of the acquisition
  sstr >> _frameModel; // frame and model number
  sstr >> _acquisitionMode; // real time, segmented, ...
  sstr >> _completion; // percent of completed time buckets
  sstr >> _xUnits; // V, s, dB, ...
  sstr >> _yUnits; // V, s, dB, ...
  sstr >> _maxBWlimit; // upper bandwidth limit
  sstr >> _minBWlimit; // lower bandwidth limit
  sstr >> _segmentCount; // how many waveforms were acquired

  // routine to fill this variable
  //tm* _dateTime; // combine the information from the strings, second precision, unix time (not local)
  
  return false;
}
