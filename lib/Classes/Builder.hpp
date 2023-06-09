#ifndef BUILDER_H
#define BUILDER_H
#include "../libCo.hpp"
#include "../libRoot.hpp"
#include "Event.hpp"

/**
 * @brief Base class of event builders (pure virtual class)
 * 
 * The first thing to do is to set the first hit of the file using Builder::set_first_hit(hit);
 * Then add the following hits using Builder::build(hit);
 * 
 * The builder can be in four states : 
 * 
 * Let's start from the first hit. The next is either in or out of the time window.
 * 
 * Out of the time window :
 * 
 * Then the first hit is single. The status is therefore 0.
 * The second hit is then used to determine if the third one is in coincidence with it or not.
 *  
 * In the time window :
 *  
 *  An event is created, extracting the values of the two first hits in the two first rows of the event.
 *  We are in status 1.
 * 
 *  Then we have to handle the third hit. It either is in or out of the time window.
 * 
 *    If the third hit is out of the time window, then the event is made only of the two previous hits. 
 *    We are in status 2. We can perform a trigger and write down the event.
 * 
 *    If the third hit is in the time window, the values of the third hit are extracted 
 *    inside of the third row of the event. We are in status 1 still. 
 *    We then have to handle the next hits and add them to the event until a hit falls out of the time window.
 *
 * Now, you'll have to create your own event builder by deriving it from this base class : 
 * 
 *        class MyEventBuilder : public Builder
 *        {
 *            ....
 *            build(...);
 *        };
 * 
 * You will have to overload the buid method
 * 
*/
class Builder
{
public:
  Builder(){}
  Builder(Event * event){m_event = event;}
  ~Builder(){}

  // Getters :
  bool const & isCoincTrigged() const {return coincON;}


  Hit const & getLastHit() const {return m_last_hit;}

  uchar const & status() const { return m_status; }
  bool isBuilding() const {return (m_status==1);}
  bool isBuilt() const {return (m_status==2);}
  bool isSingle () const {return (m_status == 0 && m_single_hit.label>0);}
  bool hasSingle() const {return (m_status == 0 && m_single_hit.label>0);}
  Hit const & singleHit() const {return m_single_hit;}
  Event getSingleEvent() const {return Event(m_single_hit);}

  Event* getEvent() const {return m_event;}
  UShort_t size() const {return m_event -> size();}

  virtual bool build(Hit const & _hit) = 0; // pure virtual
  virtual void reset() {m_event->clear(); m_status = 0;}

  // Setters :
  void set_last_hit(Hit const & hit)
  {
    m_last_hit = hit;
  }
  void set_first_hit(Hit const & hit)
  {
    m_last_hit = hit;
  }

protected:

  Event*    m_event   = nullptr;

  std::size_t m_raw_Ge  = 0;
  std::size_t m_DSSD    = 0;
  std::size_t m_modules = 0;

  Hit m_last_hit;
  Hit m_single_hit;

  bool coincON = false;
  uchar m_status = 0;
};

#endif //BUILDER_H
