#ifndef MTOBJECT_H
#define MTOBJECT_H
/*
  * This object is meant to be a base class for any other multithreaded object
  * It's only point is to manage this nb_threads static member variable
  * That is, all classes inheriting from MTObject will share this variable
*/
#include "TThread.h"
#include <thread>
#include <mutex>

class MTObject
{
public:
  static unsigned short nb_threads;
  static std::mutex mutex;
};
// As it is a static variable we have to declare it outside of the class
// Also, I think it is better to initialise it at 1, in order to avoid unitialisation issue
unsigned short MTObject::nb_threads = 1;
#endif //MTOBJECT_H

#ifndef MTCOUNTER_H
#define MTCOUNTER_H

template <class T>
struct MTCounter : public MTObject
{
public:
  operator T() {return m_counter;}
  T & operator[] (int i) {return m_collection[i];}
  void operator++() {m_counter++;}
  void Merge() {for (auto const & e : m_collection) m_counter+=e;}
  void init() {m_collection.resize(MTObject::nb_threads,0);}
private:
  std::vector<T> m_collection;
  T m_counter = 0;
};

#endif //MTCOUNTER_H
