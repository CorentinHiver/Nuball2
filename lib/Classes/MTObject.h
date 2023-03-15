#ifndef MTOBJECT_H
#define MTOBJECT_H
/*
  * This object is meant to be a base class for any other multithreaded object
  * It's only point is to manage this nb_threads static member variable
  * That is, all classes inheriting from MTObject will share this variable
*/
class MTObject
{
public:
  static unsigned short nb_threads;
};
// As it is a static variable we have to declare it outside of the class
// Also, I think it is better to initialise it at 1 to avoid unitialisation issue
unsigned short MTObject::nb_threads = 1;

#endif //MTOBJECT_H
