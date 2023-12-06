//g++ NearLine2.cpp $(root-config --glibs --cflags --libs) $(pkg-config  --cflags --libs libfasterac) -o NearLine -O2
// NearLine3 v1
#define FASTERAC
// #define FATIMA
// #define LICORNE
#define N_SI_129
// #define N_SI_120
// #define USE_DSSD
// #define PARIS
// #define N_SI_131

#include <NearLine.hpp>

int main(int argc, char **argv)
{
  NearLine app;
  if( ((argc == 2) ? app.loadConfig(argv[1]) : app.loadConfig()) )
  {
    app.launch();
    std::cout << "NearLine stopped correctly" << std::endl;
    return 1;
  }
  else
  {
    std::cout << "Abort" << std::endl << "Problem ! Check configuration file "<< std::endl;
    return -1;
  }
}

/*
NearLine treats FASTER
An ID file must be provided linking the name of the detectors and their label
*/
