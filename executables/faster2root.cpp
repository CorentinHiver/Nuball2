#include <Faster2Root.hpp>
#define NORMAL
// #define TRIGGER

#ifdef TRIGGER_PC
  #include <CloversV2.hpp>
#endif // TRIGGER_PC

#ifdef TRIGGER_PPC
  #include <CloversV2.hpp>
#endif // TRIGGER_PPC

#ifdef Calibration
  #include <CalibAndScale.hpp">
#endif //Calibration

int main(int argc, char ** argv)
{
#ifdef NORMAL
  Faster2Root(argc, argv);
#endif //NORMAL
#ifdef TRIGGER_PC
  int hit_i = 0;
  thread_local CloversV2 clovers;
  Faster2Root(argc, argv, [&hit_i, &clovers](Event const & event)
  {
    // Example of a simple trigger : DSSD trigger in Nuball2 :
    // for (hit_i = 0; hit_i<event.mult; hit_i++) if (event.labels[hit_i] > 799 && event.labels[hit_i] < 856 ) return true;

    // Example of a clean Germaniums trigger (add-back + compton suppression) and a DSSD
    // Works only with calibrated energies !!
    for (hit_i = 0; hit_i<event.mult; hit_i++)
    {
      auto const & label = event.labels[hit_i];
      if (label > 799 && label < 856 ) 
      {
        clovers = event;
        if (clovers.clean.size() > 0) return true;
      }
    }
    return false;
    #endif //TRIGGER_PPC

    // Example of a front-back DSSD coincidence
    for (hit_i = 0; hit_i<event.mult; hit_i++)
    {
      auto const & label = event.labels[hit_i];
      if (label > 799 && label < 820 ) 
      {
        for (hit_j = hit_i+1; hit_j<event.mult; hit_j++)
        {
          if (label > 819 && label < 856 ) return true;
        }
      }
    }
    return false;

    // See lib/Classes/Event.hpp for more information on the Event class
  });
#endif //TRIGGER_PC
  return 1;
}
