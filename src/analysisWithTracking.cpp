#include <iostream>

#include "AnalysisWithTracking.hh"

int main(int argc, char* argv[])
{
  if(argc != 4){
    std::cout << "\tUsage analysisWithTracking analyzedScopeDataFile trackingFile confFile" << std::endl;
    return 1;
  }

  AnalysisWithTracking* analysisClass = new AnalysisWithTracking(argv[1], argv[2], argv[3]);
  analysisClass->Analyze();
  analysisClass->Save();
  delete analysisClass;
  
  return 0;
}
