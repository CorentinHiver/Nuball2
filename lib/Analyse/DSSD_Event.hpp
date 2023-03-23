#ifndef DSSD_EVENT_H
#define DSSD_EVENT_H

#include "../Classes/Event.hpp"

class DSSD_Sector
{
public:
  DSSD_Sector()
  {

  }
private:

};

class DSSD_Ring
{
public:
  DSSD_Ring()
  {

  }

private:
  uchar m_label;
  Float_t m_nrj = 0;
  Float_t nrj = 0;
};

class DSSD_Event
{
public:
  DSSD_Event();
  void Initialize();
  void set(Event const & evt, int const & index);

private:
  static std::array<DSSD_Sector, 32> m_Sectors;
  static std::array<DSSD_Ring  , 16> m_Rings;
};

void DSSD_Event::set(Event const & evt, int const & i)
{

}


#endif //DSSD_EVENT_H
