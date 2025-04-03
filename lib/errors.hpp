#ifndef ERROR_HPP
#define ERROR_HPP

#include "print.hpp"

class OverwriteError
{
public:
  OverwriteError(std::string const & message) noexcept : m_message(message) {error(message);}
  auto const & what() const noexcept {return m_message;}
  auto what() {return m_message;}
private:
  std::string m_message;
};

#endif //ERROR_HPP