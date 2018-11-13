#ifndef LINEFIT_HH
#define LINEFIT_HH

// structure to be minimized to fit the tracks
// contains a pointer to the points of the tracks to be fitted and implements the operator () to be called by the fitter
// the class itself cannot contain the points
// assume that the telescope planes are parallel to eachother and perpendicular to z
// the distance between the measured points and the parametrization is calculated on the planes of the telescope

#include <vector>

class lineFit{
private:
  std::vector<std::vector<double>>* points; // container for the points to be fitted, must be a pointer for it to retain its contents when the operator () is called... not clear why. If it is not a pointer, its size appears to be 0 in the () operator.

public:
  lineFit(std::vector<std::vector<double>>*); // the container of points cannot be included in the class, otherwise it will be destroyed from root after the minimization
  ~lineFit();
  
  void clear(); // remove the points stored
  void addPoint(std::vector<double> p); // add a point to the container
  unsigned int nPoints(); // return number of points in the container
  
  // parametrization of a straight line, giving x and y for a given z
  // the line is not parallel to the xy plane
  void line(double z, const double* par, double* out);

  double operator() (const double* par); // used for minimization
  
};

#endif //#ifndef LINEFIT_HH
