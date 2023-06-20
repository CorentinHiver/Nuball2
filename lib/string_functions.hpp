#ifndef STRING_FUNCTIONS_HPP
#define STRING_FUNCTIONS_HPP

#include <vector>
#include <string>

std::string firstPart       (std::string const & string, char const & sep) { return (string.substr(0, string.find_first_of(sep) ));  }
std::string lastPart        (std::string const & string, char const & sep) { return (string.substr(   string.find_last_of(sep)+1));  }
std::string removeFirstPart (std::string const & string, char const & sep) { return (string.substr(   string.find_first_of(sep) ));  }
std::string removeLastPart  (std::string const & string, char const & sep) { return (string.substr(0, string.find_last_of(sep)  ));  }

std::vector<std::string> getList(std::string string, char const & sep)
{
  int pos = 0;
  std::vector<std::string> ret;
  while((pos = static_cast<int>(string.find(sep)) ) != -1)
  {
    ret.push_back(string.substr(0,pos));
    string.erase(0,pos+1);    
    string = string.substr(pos+1);
  }
  return ret;
}

std::string removeBlankSpace(std::string str)
{
	int pos = 0;
  while ( (pos = static_cast<int>(str.find(' ')) ) != -1)
  {
    str = str.substr(0,pos) + str.substr(pos+1,str.size()-pos-1);
  }
  return str;
}

/**
 * @brief Removes the first character of a string
 * 
 * Careful, time complexity makes it really heavy on big string
 * 
 */
std::string & pop_front(std::string & string) {if (string.size() > 0) string.erase(0,1); return string;}

/**
 * @brief Replace all the commas of a std::string with dots
 */
std::string rpCommaWDots(std::string str)
{
	int pos = 0;
	while ( ( pos = static_cast<int>(str.find(",")) ) != -1)
	{
		str = str.substr(0,pos) + "." + str.substr(pos+1,str.size()-pos-1);
	}
	return str;
}

/**
 * @brief Returns true if all its characters are digits
 */
bool isNumber(std::string const & string)
{
  for (auto const & c : string)
  {
    if (!(isdigit(c) || "E")) return false;
  }
  return true;
}

#endif //STRING_FUNCTIONS_HPP