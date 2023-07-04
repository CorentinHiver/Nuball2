#ifndef PERFORMANCES_HPP
#define PERFORMANCES_HPP

#include "../libCo.hpp"
#include "../MTObjects/MTCounter.hpp"

class Performances
{
public:
    Performances(){}

    MTCounter nb_hits;
    MTCounter nb_events;
    MTCounter nb_raw_events;
    MTCounter nb_kept_events;

    MTCounter nb_total_hits;
    MTCounter nb_total_events;
    MTCounter nb_total_raw_events;
    MTCounter nb_total_kept_events;
};



#endif //PERFORMANCES_HPP