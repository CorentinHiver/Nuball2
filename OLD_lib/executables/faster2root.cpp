#include "Faster2Root.hpp"
#define NORMAL
// #define TRIGGER

#ifdef TRIGGER_PC
  #include "CloversV2.hpp"
#endif // TRIGGER_PC

#ifdef TRIGGER_PPC
  #include "CloversV2.hpp"
#endif // TRIGGER_PPC

#ifdef Calibration
  #include "CalibAndScale.hpp"
#endif //Calibration

int main(int argc, char ** argv)
{
#ifdef NORMAL
  Faster2Root(argc, argv);
#endif //NORMAL
#ifdef TRIGGER_PC
  thread_local CloversV2 clovers; // Thread local is really important in case multithreading is activated (option '-m' > 1)
  Faster2Root(argc, argv, [&clovers](Event const & event)
  {
    // See lib/Classes/Event.hpp for more information on the Event class

    // Example of a simple trigger : DSSD trigger in Nuball2 :
    // for (int hit_i = 0; hit_i<event.mult; hit_i++) if (event.labels[hit_i] > 799 && event.labels[hit_i] < 856 ) return true;

    // Example of a clean Germaniums trigger (add-back + compton suppression) and a DSSD
    // Works only with calibrated energies !!
    for (int hit_i = 0; hit_i<event.mult; hit_i++)
    {
      auto const & label = event.labels[hit_i];
      if (label > 799 && label < 856)
      {
        clovers = event;
        if (clovers.clean.size() > 0) return true;
      }
    }
    return false;
  });
    #endif //TRIGGER_PC

    #ifdef TRIGGER_PPC
  thread_local CloversV2 clovers;
  Faster2Root(argc, argv, [](Event const & event)
  {
    // Example of a front-back DSSD coincidence
    for (int hit_i = 0; hit_i<event.mult; hit_i++)
    {
      auto const & label0 = event.labels[hit_i];
      if (label0 > 799 && label0 < 820 ) // If there is a ring
      {
        for (int hit_j = 0; hit_j<event.mult; hit_j++)
        {
          auto const & label1 = event.labels[hit_j];
          if (label1 > 819 && label1 < 856 ) // If there is a sector
          {
            clovers.loadRaw(event);
            if (clovers.clean.size() > 0) return true; // IF there is at least a clean Ge
          }
        }
      }
    }
    return false;

  });
#endif //TRIGGER_PC
  return 1;
}

// Copy the following command line to compile the converter. Note that you can change the path to the library by changing libPath at the beggining of the line
// libPath=../lib g++ -o faster2root faster2root.cpp $(root-config --cflags) $(pkg-config --cflags libfasterac) -I$libPath/Modules -I$libPath -I$libPath/Classes -I$libPath/Analyse $(root-config --libs) $(pkg-config --libs libfasterac)