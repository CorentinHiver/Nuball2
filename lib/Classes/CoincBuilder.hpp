#ifndef COINCBUILDER_H
#define COINCBUILDER_H
#include "../utils.hpp"
#include "Builder.hpp"

/*
  Makes coincidence building easier
*/

class CoincBuilder : public Builder
{
public:
  // Constructors :
  explicit CoincBuilder()
  {
    m_buffer = new Buffer;
    m_buffer->resize(2);
  }
  explicit CoincBuilder(Int_t const & i)
  {
    m_buffer = new Buffer;
    m_buffer->resize(2);
  }
  explicit CoincBuilder(Buffer * _event)                            : m_buffer(_event){}
  explicit CoincBuilder(Buffer * _event, Int_t const & _timeWindow) : m_buffer(_event), m_time_window(_timeWindow){}

  // Methods :
  // Add Hits  Outputs  0: single | 1: begin of coincidence | 2: coincidence complete
  Bool_t build(Hit const & _hit);

  void   flush() {m_buffer -> resize(0); m_buffer -> clear(); m_status = 0;}
  Bool_t coincidence(Hit const & hit) {return ((hit.time - m_last_hit.time) < m_time_window);}

  // Getters :
  Buffer* getBuffer() const {return m_buffer;}
  Buffer  getSingleBuffer() {return Buffer(1,m_single_hit);}
  Bool_t const & isCoincTrigged() const {return coincON;}
  UShort_t size() const {return m_buffer -> size();}

  // Setters :
  void setTimeWindow(Int_t const & _timeWindow) {m_time_window = _timeWindow;}

  // Printers :
  void printBuffer();

  // Public members :
  unsigned char mult = 1;
  Int_t         n_evt = 0;

private:
  // Attributes :
  Buffer* m_buffer = nullptr;
  Time    m_time_window = 500000ull; // 500 000 ps by default (ull = unsigned long long)
};

Bool_t CoincBuilder::build(Hit const & hit)
{//return true when a coincidence is ready to be processed
  if (m_status == 2 || mult>254) this->flush(); //because nb_evts is on unsigned char so < 255
  if (!coincON)
  {// No coincidence going on. If there is, go ...
    if (coincidence(hit))
    {// Bingo, coincidence detected ! Next call, go ...
      m_buffer -> push_back( m_last_hit );
      m_buffer -> push_back( hit );
      mult = 2;
      coincON = true;
      m_status = 1;
    }
    else
    {// No coincidence detected...
      m_single_hit = m_last_hit;
      m_last_hit = hit; //store the hit for next loop
      mult = 1;
      n_evt++;
      m_status = 0;
    }
  }
  else
  {// ... there ! Coincidence already detected, check if it ends
    if (coincidence(hit))
    {// Hit in coincidence with the previous hits
      mult++;
      m_buffer -> push_back(hit);
    }
    else
    {// Hit out of coincidence. Next call, go back to first condition.
      n_evt++;
      coincON = false;
      m_last_hit = hit;
      m_status = 2;
      return true;
    }
  }
  return false;
}

void CoincBuilder::printBuffer()
{
  std::cout << " | ";
  for (auto const & hit : *m_buffer)
  {
    std::cout << hit.label << " | " ;
  }
  std::cout << std::endl;
}


#endif //COINCBUILDER_H
