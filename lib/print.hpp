#ifndef PRINT_HPP
#define PRINT_HPP


// Generic print :

template <class T> void print()
{
  std::cout << std::endl;
}

template <class T> void print(T const & t)
{
  std::cout << t << std::endl;
}

template <class... T> void print(T const &... t)
{
  ((print(t)),...);
}

template <class T, class... T2> void print(T const & t, T2 const &... t2)
{
  std::cout << t << " ";
  print(t2...);
}

//Prints containers :

template <class E> void print (std::vector<E> const & t)
{
  for (auto const & e : t) print(e);
}

template <class E, class... T> void print (std::vector<E> const & v, T const &... t2)
{
  print(v);
  print(t2...);
}

template <class K, class V> void print (std::map<K,V> const & m)
{
  print();
  print("{");
  print();
  for (auto const & pair : m)
  {
    print("key : ");
    print(pair.first);
    // print();
    print("value :");
    print(pair.second);
    print();
  }
  print("}");
  print();
}

template <class K, class V, class... T> void print (std::map<K,V> const & m, T const & ... t2)
{
  print(m);
  print(t2...);
}

// Class Debug, based on template print() :
class Debug
{
public:
  template <class... T> Debug(T... t) { (*this)(t...); }

  template <class... T> void operator () (T... t)
  {
    if (sizeof...(t) == 0)
    {
      print("coucou ", i);
      i++;
      return;
    }
    print(t...);
    std::cout << std::endl;
  }

  void operator () (int _i)
  {
    std::cout << "coucou " << _i << std::endl;
    i = _i+1;
  }
private:
  int i = 0;
};

#endif //PRINT_HPP