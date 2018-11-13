#include "lineFit.hh"

#include "math.h"

lineFit::lineFit(std::vector<std::vector<double>>* poVec){
  points = poVec;
  
  return;
}

lineFit::~lineFit(){
  return;
}

void lineFit::clear(){ // remove the points stored
  points->clear();
  return;
}

unsigned int lineFit::nPoints(){
  return points->size();
}

void lineFit::addPoint(std::vector<double> p){ // add a point to the container
  points->push_back(p);
  return;
}
  
// parametrization of a straight line, giving x and y for a given z
// the line is not parallel to the xy plane
void lineFit::line(double z, const double* par, double* out){
  out[0] = par[0] + par[1] * z; // x
  out[1] = par[2] + par[3] * z; // y
  out[2] = z; // z
    
  return;
}

double lineFit::operator() (const double* par){
  double ret = 0;
  double calcPoint[3];

  for(unsigned int ipt = 0; ipt < points->size(); ++ipt){
    line(points->at(ipt)[2], par, calcPoint); // calculate the point from parametrization
    for(int j = 0; j < 3; ++j)
      ret += pow(calcPoint[j] - points->at(ipt)[j], 2); // sum distances from points
  }
  
  return ret;
}
