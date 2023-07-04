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
  std::size_t pos = 0;
  std::vector<std::string> ret;
  while((pos = string.find(sep) ) != -1ul) // ul stands for unsigned long int
  {
    if (pos==0) 
    {// If the character is at front position then skip it;
      string.erase(0,1);
      continue; 
    }
    else if (pos == string.size())
    {// If the character is at the end of the string then we have reached and can terminate the loop
      ret.push_back(string.substr(0,pos));
      break;
    }
    else
    {// If the character is at the middle of the string then remove the part before and push it in the vector
      ret.push_back(string.substr(0,pos));
      string.erase(0,pos+1);    
    }
  }
  // If the string does not finish with the character then we must take the last part of it
  if (string.size() > 0) ret.push_back(string);
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