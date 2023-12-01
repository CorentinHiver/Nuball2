#include <Faster2Root.hpp>
#define NORMAL
// #define TRIGGER


int main(int argc, char ** argv)
{
#ifdef NORMAL
  Faster2Root(argc, argv);
#endif //NORMAL

#ifdef TRIGGER
  int hit_i = 0;
  Faster2Root(argc, argv, [&hit_i](Event const & event)
  {
    // Example of a simple trigger (DSSD trigger in Nuball2 campaign):
    for (hit_i = 0; hit_i<event.mult; hit_i++)
    {
      if (event.labels[hit_i] > 799 && event.labels[hit_i] < 856 )
    }
    // See lib/Classes/Event.hpp for more information on the event class
  });
#endif //TRIGGER

  return 1;
}