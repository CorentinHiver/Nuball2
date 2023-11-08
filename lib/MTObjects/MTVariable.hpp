#ifndef MTVARIABLE_HPP
#define MTVARIABLE_HPP

#include "MTObject.hpp"

/**
 * @brief UNUSED
 */
template<class T>
class MTVariable
{
public:
  MTVariable()
  {
    m_variables.resize(MTObject::getThreadsNb());

    MTObject::nbThreadsChanged.connect
    (
      [this](int nb_threads)
      {
        resize(nb_threads);
      }
    );
  }

  void resize(size_t size) 
  {
    lock_mutex(MTObject::mutex);
    m_variables.clear();
    m_variables.resize(size);
  }

private:
  std::vector<T> m_variables;
};

#endif // MTVARIABLE_HPP