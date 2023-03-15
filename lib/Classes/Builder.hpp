#ifndef BUILDER_H
#define BUILDER_H
#include "../utils.hpp"
#include "Event.hpp"

class Builder
{
public:
  Builder(){}
  ~Builder(){}

  // Getters :
  Bool_t const & isCoincTrigged() const {return coincON;}
  Bool_t isSingle() const {return (this->status() == 0 && m_single_hit.label>0);}
  Hit & singleHit() {return m_single_hit;}
  Event getSingleEvent() {return Event(m_single_hit);}
  unsigned char const & status() const { return m_status; }

  Event* getEvent() const {return m_event;}
  UShort_t size() const {return m_event -> size();}

  virtual Bool_t build(Hit const & _hit) {return false;}
  virtual void reset() {m_event->clear();m_status = 0;}

  // Setters :
  void set_last_hit(Hit const & hit)
  {
    m_last_hit = hit;
  }

protected:
  Event*      m_event  = nullptr;

  Hit m_empty_hit ;
  Hit m_last_hit = m_empty_hit;
  Hit m_single_hit = m_empty_hit;

  Bool_t coincON = false;
  unsigned char m_status = 0;
};

#endif //BUILDER_H
