#ifndef STRING_FUNCTIONS_HPP
#define STRING_FUNCTIONS_HPP

#include <vector>
#include <string>

std::string firstPart       (std::string const & string, char const & sep) { return (string.substr(0, string.find_first_of(sep) ));  }
std::string lastPart        (std::string const & string, char const & sep) { return (string.substr(   string.find_last_of(sep)+1));  }
std::string removeFirstPart (std::string const & string, char const & sep) { return (string.substr(   string.find_first_of(sep) ));  }
std::string removeLastPart  (std::string const & string, char const & sep) { return (string.substr(0, string.find_last_of(sep)  ));  }

/**
 * 
 * @brief Cuts a string into pieces separated by the given separator like ';' or ' ' or ',' ...
 * 
 * @details Understand the removeVoids parameter : for instance, we have string = ";1;2;3;;5"
 * 
 * without removeVoids this function returns {"1", "2", "3", "5"}
 * with removeVoids this function returns {"", "1", "2", "3", "", "5"}
 * 
*/
std::vector<std::string> getList(std::string string, char const & separator, bool const & removeVoids = false)
{
  size_t pos = 0;
  std::vector<std::string> ret;
  while((pos = string.find(separator) ) != -1ul)
  {
    if (pos==0) 
    {
      if (!removeVoids) ret.push_back("");
      string.erase(0,1);
      continue; 
    }
    else if (pos == string.size())
    {// If the separator is at the end of the string then we have completed the list and can terminate the loop
      ret.push_back(string.substr(0,pos));
      break;
    }
    else
    {// If the separator is at the middle of the string then remove the part before and push it into the vector
      ret.push_back(string.substr(0,pos));
      string.erase(0,pos+1);
    }
  }
  // If the string does not finish with the character then we must take the last part of it
  if (string.size() > 0) ret.push_back(string);
  return ret;
}

/// @brief Remove all the blank space in a string
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
 * @brief Replaces all the instances of one character with another
 * @details 
 * For instance : 
 *        std::string istring = "je_suis_ton_pere";
 *        std::string ostring = replaceCharacter(istring, '_', ' ');
 *        print(ostring);
 *        // output : "je suis ton pere"
 * 
*/
std::string replaceCharacter(std::string const & istring, char const & ichar, char const & ochar)
{
  auto list = getList(istring, ichar);
  std::string ostring;

  for (auto const & string : list)
  {
    ostring+=(string+ochar);
  }
  return ostring;
}

/**
 * @brief Removes the first character of a string
 * 
 * @attention Careful, time complexity makes it really heavy on big string
 */
std::string & pop_front(std::string & string) {if (string.size() > 0) string.erase(0,1); return string;}


/// @brief Replace all the commas of a std::string with dots
std::string rpCommaWDots(std::string str)
{
	int pos = 0;
	while ( ( pos = static_cast<int>(str.find(",")) ) != -1)
	{
		str = str.substr(0,pos) + "." + str.substr(pos+1,str.size()-pos-1);
	}
	return str;
}


/// @brief Returns true if all its characters are digits
bool isNumber(std::string const & string)
{
  for (auto const & c : string)
  {
    if (!(isdigit(c) || "E")) return false;
  }
  return true;
}

bool found(std::string const & string, std::string const & substr)
{
  return (string.find(substr) != std::string::npos);
}

bool remove(std::string & string, std::string const & substr)
{
  auto pos = string.find(substr);
  if (pos!=std::string::npos)
  {
    auto first_str = string.substr(0, pos);
    string = string.substr(0, pos)+string.substr(pos+substr.size());
    return true;
  }
  else return false;
}

/// @brief Convert any type into string
template<class T>
std::string my_to_string(const T& value)
{
  std::ostringstream oss;
  oss << value;
  return oss.str();
}

/// @brief Concatenate a series of arguments into a big string
/// @example print(concatenate(1, " ", argv[2], " test"));
template<class... Args>
std::string concatenate(Args&&... args)
{
  std::ostringstream oss;
  (oss << ... << my_to_string(std::forward<Args>(args)));
  return oss.str();
}

#endif //STRING_FUNCTIONS_HPP