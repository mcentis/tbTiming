#include <iostream>

#include "AnalyzeScopeClass.hh"

int main(int argc, char* argv[])
{
  if(argc != 3){
    std::cout << "\tUsage analyzeScopeData rootFile confFile" << std::endl;
    return 1;
  }

  AnalyzeScopeClass* analysisCl = new AnalyzeScopeClass(argv[1], argv[2]);
  
  return 0;
}
