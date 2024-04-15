#ifndef COPROGRESSBAR_HPP
#define COPROGRESSBAR_HPP

#include "../libCo.hpp"
#include "Timer.hpp"

/**
 * @brief Displays an auto-updating progress
 * @example
 * 
 *        int i = 0;
 *        int max = 1000000;
 *        CoProgressBar prog(&i, max);
 *        for (;i<max; i++)
 *        {
 *          prog.show();
 *        }
 * 
 * @note 
 * 
 * The various show methods go back to the beginning of the line at the end in
 * order to update at next iteration. If you wich to display something else in between, 
 * please consider adding an extra end-line character (std::endl, '\\n') ...
 */
template<class T>
class CoProgressBar
{
public:

  // Constructors :
  CoProgressBar() noexcept = default;
  CoProgressBar(T * value, float const & value_max) : m_value(value), m_value_max(value_max) {}
  CoProgressBar(T * value, float const & value_max, int width) : m_value(value), m_value_max(value_max), m_width(width) {}

  // Settings methods :
  bool setValue(T * value) {return (m_value = value);}
  bool setValueMax(float const & value_max) {return (m_value_max = value_max);}

  // Show methods :
  void show(std::string const & message = "");
  void showFast();

private:

  int m_width = 50; // Width of the progress bar in pixels

  T* m_value = nullptr;
  T m_value_max = 0;
  T m_last_value = 0;

  Timer timer;

  bool first_time = true;
};

template<class T>
void CoProgressBar<T>::showFast()
{
  if (!m_value)              throw_error("in CoProgressBar<T>::show() : the value has not been set !!");
  else if (m_value_max == 0) throw_error("in CoProgressBar<T>::show() : the maximum value has not been set !!");

  auto const & real_percentage = float_cast(*m_value)/m_value_max;
  auto const & nb_chars = int_cast(real_percentage*m_width);

  std::cout << "|";
  for (int i = 0; i<m_width; i++) 
  {
    if (i<nb_chars) std::cout << ".";
    else            std::cout << " ";
  }
  std::cout << "| : " << int_cast(real_percentage*100) << "% " << std::endl << "\033[F";
  std::cout.flush();
}

template<class T>
void CoProgressBar<T>::show(std::string const & message)
{
  if (!m_value)              throw_error("in CoProgressBar<T>::show() : the value has not been set !!");
  else if (m_value_max == 0) throw_error("in CoProgressBar<T>::show() : the maximum value has not been set !!");

  float speed = 0.f;
  if (first_time) {timer.Restart(); speed = 0; first_time = false;}
  else {speed = 100.*(float_cast(*m_value)-float_cast(m_last_value))/(timer.TimeSec()*float_cast(m_value_max));}

  auto const & real_percentage = float_cast(*m_value)/float_cast(m_value_max);
  auto const & nb_chars = int_cast(real_percentage*float_cast(m_width));

  std::cout << "|";
  for (int i = 0; i<m_width; i++) 
  {
    if (i<nb_chars) std::cout << ".";
    else            std::cout << " ";
  }
  std::cout << "| : " << int_cast(real_percentage*100) << "% (" << std::setprecision(3) << speed << " %/s)";
  if (message!="") std::cout << message ;
  std::cout << std::endl << "\033[F";// This code flushes the previous line
  std::cout.flush();
  timer.Restart();
  m_last_value = *m_value;
}


#endif //COPROGRESSBAR_HPP