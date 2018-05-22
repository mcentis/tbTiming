#include "waveformPreamble.hh"

#include <sstream>
//#include <ctime>

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
  sstr >> _time;
  sstr >> _frameModel; // frame and model number
  sstr >> _acquisitionMode; // real time, segmented, ...
  sstr >> _completion; // percent of completed time buckets
  sstr >> _xUnits; // V, s, dB, ...
  sstr >> _yUnits; // V, s, dB, ...
  sstr >> _maxBWlimit; // upper bandwidth limit
  sstr >> _minBWlimit; // lower bandwidth limit
  sstr >> _segmentCount; // how many waveforms were acquired

  // calculate the unix time stamp check the last part of this block with data
  std::string date = std::string(_date);
  std::string time = std::string(_time);
  tm* timeStruct = NULL;
  strptime((date+" , "+time).c_str(), "%d %b %Y , %H:%M:%S", timeStruct);
  time_t tmpTime = mktime(timeStruct);
  timeStruct = gmtime(&tmpTime);
  _dateTime = mktime(timeStruct);
  
  return false;
}
//"5 MAY 2018 , 19:42:36:46"
