#ifndef PRINT_HPP
#define PRINT_HPP


// Generic print :

template <class T> void print()
{
  std::cout << std::endl;
}

#ifndef __CINT__
template <class T> void print(T const & t)
{
  std::cout << t << std::endl;
}
#endif //__CINT__

template <class... T> void print(T const &... t)
{
  ((print(t)),...);
}

template <class T, class... T2> void print(T const & t, T2 const &... t2)
{
  std::cout << t << " ";
  print(t2...);
}


// Useful overload of operator<< in std::cout stream :

template <class E>
std::ostream& operator<<(std::ostream& cout, std::vector<E> const & v)
{
  for (auto const & e : v) cout << e << " ";
  return cout;
}

template <class F, class S> 
std::ostream& operator<<(std::ostream& cout, std::pair<F,S> const & p)
{
  cout << " {" << p.first << ", " << p.second << "}" << std::endl;
  return cout;
}

template <class K, class V> 
std::ostream& operator<<(std::ostream& cout, std::map<K,V> const & m)
{
  cout << "{";
  for (auto const & pair : m) cout << pair;
  cout << "}\n";
  return cout;
}

void print_precision(int n = 6) {std::cout << std::setprecision(n);}


// Idea to print any class that has a member called getPrintable that returns an object that can be printed 
// (i.e. a base type or a class for which operator<< have been overloaded like vector or map)
// Raw idea from chatgpt so it needs refinement
//
// template <typename T, typename = void>
// struct HasRequiredFunction : std::false_type {};

// // Partial specialization for types that have the required function
// template <typename T>
// struct HasRequiredFunction<T, std::void_t<decltype(std::declval<T>().getPrintable())>> : std::true_type {};

// template <class T>
// std::ostream& operator<<(std::ostream& cout, T const & t)
// {
//   std::static_assert(HasRequiredFunction<T>::value, "must have a getPrintable() method or overloaded operator<< to be printed.");
//   cout << t.get();
//   return cout;
// }

// template <class E>
//  void print (std::vector<E> const & v)
// {
//   for (auto const & e : v) print(e);
// }
// template <class E, class... T> void print (std::vector<E> const & v, T const &... t2)
// {
//   print(v);
//   print(t2...);
// }

// template <class K, class V> void print (std::map<K,V> const & m)
// {
//   print();
//   print("{");
//   print();
//   for (auto const & pair : m)
//   {
//     print("key : ");
//     print(pair.first);
//     // print();
//     print("value :");
//     print(pair.second);
//     print();
//   }
//   print("}");
//   print();
// }

// template <class K, class V, class... T> void print (std::map<K,V> const & m, T const & ... t2)
// {
//   print(m);
//   print(t2...);
// }

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
  static int i;
};

int Debug::i = 0;

#endif //PRINT_HPP