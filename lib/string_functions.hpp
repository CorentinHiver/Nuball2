#ifndef STRING_FUNCTIONS_HPP
#define STRING_FUNCTIONS_HPP

#include <vector>
#include <cstring>
#include <string>
#include <sstream>

/// @brief Returns the string to the left of the first occurrence of sep in the string
std::string firstPart       (std::string const & string, std::string const & sep) { return (string.substr(0, string.find_first_of(sep) ));  }
/// @brief Returns the string to the right of the last occurrence of sep in the string
std::string lastPart        (std::string const & string, std::string const & sep) { return (string.substr(   string.find_last_of(sep)+1));  }
/// @brief Returns the string to the right of the first occurrence of sep in the string
std::string removeFirstPart (std::string const & string, std::string const & sep) { return (string.substr(   string.find_first_of(sep) ));  }
/// @brief Returns the string to the left of the last occurrence of sep in the string
std::string removeLastPart  (std::string const & string, std::string const & sep) { return (string.substr(0, string.find_last_of(sep)  ));  }

/// @brief Returns the string to the left of the first occurrence of sep in the string
std::string firstPart       (std::string const & string, char const & sep) { return (string.substr(0, string.find_first_of(sep) ));  }
/// @brief Returns the string to the right of the last occurrence of sep in the string
std::string lastPart        (std::string const & string, char const & sep) { return (string.substr(   string.find_last_of(sep)+1));  }
/// @brief Returns the string to the right of the first occurrence of sep in the string
std::string removeFirstPart (std::string const & string, char const & sep) { return (string.substr(   string.find_first_of(sep) ));  }
/// @brief Returns the string to the left of the last occurrence of sep in the string
std::string removeLastPart  (std::string const & string, char const & sep) { return (string.substr(0, string.find_last_of(sep)  ));  }

/**
 * @brief Cuts a string into pieces separated by the given separator like ';' or ' ' or ','
 * 
 * @param removeVoids: For instance, we have string = ";1;2;3;;5".
 * without removeVoids this function returns {"1", "2", "3", "5"}
 * with removeVoids this function returns {"", "1", "2", "3", "", "5"}
 * 
*/
void fillList(std::vector<std::string> & list, std::string string, std::string const & separator, bool const & removeVoids = false)
{
  size_t pos = 0;
  while((pos = string.find(separator) ) != -1ul)
  {
    if (pos==0) 
    {
      if (!removeVoids) list.push_back("");
      string.erase(0,1);
      continue; 
    }
    else if (pos == string.size())
    {// If the separator is at the end of the string then we have completed the list and can terminate the loop
      list.push_back(string.substr(0,pos));
      return;
    }
    else
    {// If the separator is at the middle of the string then remove the part before and push it into the vector
      list.push_back(string.substr(0,pos));
      string.erase(0,pos+1);
    }
  }
  // If the string does not finish with the character then we must take the last part of it
  if (string.size() > 0) list.push_back(string);
}

std::vector<std::string> getList(std::string string, std::string const & separator, bool const & removeVoids = false)
{
  std::vector<std::string> ret;
  fillList(ret, string, separator, removeVoids);
  return ret;
}

std::vector<std::string> split(std::string string, std::string const & separator, bool const & removeVoids = false)
{
  return getList(string, separator, removeVoids);
}

std::vector<std::string> split(std::string string, char const & separator, bool const & removeVoids = false)
{
  return getList(string, std::string(1, separator), removeVoids);
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
 *        std::string inString = "je_suis_ton_pere";
 *        std::string ostring = replaceCharacter(inString, '_', ' ');
 *        print(ostring);
 *        // output : "je suis ton pere"
 * 
*/
std::string replaceCharacter(std::string const & inString, char const & inChar, char const & outChar)
{
  auto list = getList(inString, std::string(inChar, 1));
  std::string ostring;

  for (auto const & string : list)
  {
    ostring+=(string+outChar);
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


/// @brief Returns true if all its characters are digits (allows E to represent power and . for decimal)
bool isNumber(std::string const & string)
{
  if (string.size() < 1) return false;
  int nb_E = 0;
  int nb_points = 0;
  for (auto const & c : string)
  {
    if (!(isdigit(c)))
    {
      if (c == 'E') ++nb_E; 
      else if (c == '.') ++nb_points;
      else if (c == '+' || c == '-') continue;
      else return false;
    } 
  }
  if (nb_E>1 || nb_points>1) return false;
  return true;
}

/// @brief Returns true if the string has at least one occurrence of substr
bool found(std::string const & string, std::string const & _substr)
{
  return (string.find(_substr) != std::string::npos);
}

/// @brief Remove the first substr to the string if found
bool remove(std::string & string, std::string const & _substr)
{
  auto pos = string.find(_substr);
  if (pos!=std::string::npos)
  {
    string = string.substr(0, pos) + string.substr(pos+_substr.size());
    return true;
  }
  else return false;
}

/// @brief Remove the first char 'c' to the string if found
bool remove(std::string & string, char const & c)
{
  auto pos = string.find(c);
  if (pos!=std::string::npos)
  {
    string = string.substr(0, pos) + string.substr(pos+1);
    return true;
  }
  else return false;
}

/// @brief Remove all the substr to the string if found
void remove_all(std::string & string, std::string const & _substr)
{
  size_t pos = 0;
  while((pos = string.find(_substr)) != std::string::npos)
  {
    string = string.substr(0, pos) + string.substr(pos+_substr.size());
  }
}

/// @brief Remove all the char 'c' to the string if found
void remove_all(std::string & string, char const & c)
{
  size_t pos = 0;
  while((pos = string.find(c)) != std::string::npos)
  {
    string = string.substr(0, pos) + string.substr(pos+1);
  }
}

/// @brief Replace the first substr to the string if it exists
bool replace(std::string & string, std::string const & substr_init, std::string const & substr_substitute)
{
  auto pos = string.find(substr_init);
  if (pos!=std::string::npos)
  {
    string = string.substr(0, pos)+substr_substitute+string.substr(pos+substr_init.size());
    return true;
  }
  else return false;
}

/// @brief Replace the first substr to the string if it exists
void replace_all(std::string & string, std::string const & substr_init, std::string const & substr_substitute)
{
  size_t pos = 0;
  while((pos = string.find(substr_init)) != std::string::npos)
  {
    string = string.substr(0, pos)+substr_substitute+string.substr(pos+substr_init.size());
  }
}

/**
 * @brief Convert i_th first arguments of argv into a string (), by default starting at the first 
 * @attention argv MUST be null-terminated
 * @details
 * Each argument starts with a space
*/
std::string argv_to_string(char** argv, int const & start_i = 0)
{
  std::string ret;
  for (int i = start_i; (argv[i] !=nullptr); ++i)
  {
    ret+= " ";
    ret+=argv[i];
  }
  return ret;
}

/// @brief Create a null terminated C-style array of char from a string
/// @attention you'll have to delete the allocated memory
char** string_to_argv(std::string const & string)
{
  // Breaks down the string into an array of substrings (separated by a space in the string)
  std::vector<std::string> string_array(getList(string, " "));// Source
  
  // Allocate the array
  char** charArray = new (std::nothrow) char*[string_array.size() + 1]; // +1 for the final nullptr
  for (std::size_t i = 0; i < string_array.size(); ++i)
  {
    //Allocate the string
    charArray[i] = new (std::nothrow) char[string_array[i].size() + 1];
    //Copy the string to the array
    std::strcpy(charArray[i], string_array[i].c_str());
  }
  charArray[string_array.size()] = nullptr;
  return charArray;
}

/// @brief Delete an argv manually created by string_to_argv()
void delete_argv(char** argv) 
{
  for (std::size_t i = 0; argv[i] != nullptr; ++i)
    {
        delete[] argv[i];  // Free individual C-style strings
    }
    delete[] argv;  // Free the array of pointers
}

/// @brief Convert any type into string, including vector of any type
template<class T>
std::string my_to_string(const T& value)
{
  std::ostringstream oss;
  oss << value;
  return oss.str();
}

/// @brief Concatenate a series of arguments into a big string
/// @example std::string myString = (concatenate(1, " ", argv[2], " test");
template<class... ARGS>
std::string concatenate(ARGS&&... args)
{
  std::ostringstream oss;
  (oss << ... << my_to_string(std::forward<ARGS>(args)));
  return oss.str();
}

// /// @brief Concatenate a series of arguments into a big string (alias)
// template<class... ARGS>
// std::string str_c(ARGS&&... args) {return concatenate(std::forward<ARGS>(args)...);}

// /// @brief concatenate string, returns a c_str (char**)
// TODO : right now can't work because returning address of local temporary object
// template<class... ARGS>
// auto concatenate_c(ARGS&&... args){return concatenate(std::forward<ARGS>(args)...).c_str();}

/// @brief concatenate string, returns a c_str (char**)
template<class... ARGS>
std::string ctcstr(ARGS&&... args) {return concatenate(std::forward<ARGS>(args)...);}

#endif //STRING_FUNCTIONS_HPP