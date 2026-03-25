#pragma once

namespace Colib
{
  namespace Terminal
  {
    constexpr char CLEAR_ROW              [] = "\033[2K"  ;
    constexpr char DISABLE_LINES_WRAPPING [] = "\033[?7l" ;
    constexpr char ENABLE_LINES_WRAPPING  [] = "\033[?7h" ;
    constexpr char RETURN                 [] =        "\r";
    constexpr char NEWLINE                [] =        "\n";

    std::ostream& move_up   (int index = 1, std::ostream & out = std::cout) {out << "\033[" << index << "A"; return out;}
    std::ostream& move_down (int index = 1, std::ostream & out = std::cout) {out << "\033[" << index << "B"; return out;}
    std::ostream& new_line  (               std::ostream & out = std::cout) {out << NEWLINE; return out;}
    std::ostream& new_lines (int lines = 1, std::ostream & out = std::cout) {for (int i = 0; i<lines; ++i) out << NEWLINE; return out;}
  } 
}