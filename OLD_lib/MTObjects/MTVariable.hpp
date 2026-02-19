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

    if (std::is_arithmetic<T>::value) total = static_cast<T>(0);

    // MTObject::nbThreadsChanged.connect
    // (
    //   [this](int nb_threads)
    //   {
    //     resize(nb_threads);
    //   }
    // );
  }

  void resize(size_t size) 
  {
    lock_mutex lock(MTObject::mutex);
    m_variables.clear();
    m_variables.resize(size);
  }

  T const & fuse()
  {
    static_assert(std::is_arithmetic<T>::value, "in MTVariable<T>::fuse() : T must be a numerical !!");
    total = static_cast<T>(0);
    for (auto const & e : m_variables) e+=m_variables;
  }



private:
  T total;
  std::vector<T> m_variables;
};

#endif // MTVARIABLE_HPP