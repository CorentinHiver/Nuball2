#include <Faster2Root.hpp>
//#define NORMAL
#define TRIGGER

#ifdef TRIGGER
#include <Clovers.hpp>
#endif // TRIGGER

int main(int argc, char ** argv)
{
  #ifdef NORMAL
  Faster2Root(argc, argv);
#endif //NORMAL
#ifdef TRIGGER
  int hit_i = 0;
  Clovers clovers;
  Faster2Root(argc, argv, [&hit_i, &clovers](Event const & event)
  {
    // Example of a simple trigger (DSSD trigger in Nuball2) :
    // for (hit_i = 0; hit_i<event.mult; hit_i++) if (event.labels[hit_i] > 799 && event.labels[hit_i] < 856 ) return true;

    // Example of a trigger DSSD + 2 clean Germaniums (add-back + compton suppression) (Nuball2)
    // Works only with calibrated energies !!
    for (hit_i = 0; hit_i<event.mult; hit_i++)
    {
      auto const & label = event.labels[hit_i];
      if (label > 799 && label < 856 ) 
      {
        lock_mutex lock(MTObject::mutex);
        clovers.SetEvent(event);
        if (clovers.CleanGe.size() > 0) return true;
      }
    }
    return false;

    // See lib/Classes/Event.hpp for more information on the Event class
  });
#endif //TRIGGER
  return 1;
}
