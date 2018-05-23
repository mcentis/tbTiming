#include "waveformPreamble.hh"

#include <string.h>
#include <sstream>
#include <algorithm> // std::replace
#include <iostream>

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

void waveformPreamble::readString(std::string line)
{
  std::replace(line.begin(), line.end(), ',', ' '); 
  std::replace(line.begin(), line.end(), '\"', ' '); 
  
  std::stringstream sstr;
  sstr.clear();
  sstr.str(line);

  char day[5], month[20], year[20];
  
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
  sstr >> day;
  sstr >> month;
  sstr >> year;
  strcpy(_date, day);
  strcat(_date, ",");
  strcat(_date, month);
  strcat(_date, ",");
  strcat(_date, year);
  sstr >> _time;
  sstr >> _frameModel; // frame and model number
  sstr >> _acquisitionMode; // real time, segmented, ...
  sstr >> _completion; // percent of completed time buckets
  sstr >> _xUnits; // V, s, dB, ...
  sstr >> _yUnits; // V, s, dB, ...
  sstr >> _maxBWlimit; // upper bandwidth limit
  sstr >> _minBWlimit; // lower bandwidth limit
  sstr >> _segmentCount; // how many waveforms were acquired
  
  // calculate the unix time stamp
  char tmpDateTime[100];
  strcpy(tmpDateTime, _date);
  strcat(tmpDateTime, " , ");
  strcat(tmpDateTime, _time);
  tm* timeStruct = new tm();
  strptime(tmpDateTime, "%d,%b,%Y , %H:%M:%S", timeStruct);
  // next lines to get the right time stamp in GMT time
  time_t tmpTime = mktime(timeStruct); 
  timeStruct = gmtime(&tmpTime);
  _dateTime = mktime(timeStruct);
  
  return;
}

