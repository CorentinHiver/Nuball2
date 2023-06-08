
std::string firstPart       (const std::string string, const char sep) { return (string.substr(0, string.find_first_of(sep) ));  }
std::string lastPart        (const std::string string, const char sep) { return (string.substr(   string.find_last_of(sep)+1));  }
std::string removeFirstPart (const std::string string, const char sep) { return (string.substr(   string.find_first_of(sep) ));  }
std::string removeLastPart  (const std::string string, const char sep) { return (string.substr(0, string.find_last_of(sep)  ));  }
std::string removeBlankSpace(std::string str)
{ //  In a std::string, removes blank spaces
	int pos = 0;
	while ( (pos = (int)str.find(" ")) != -1)
	{
		str = str.substr(0,pos) + str.substr(pos+1,str.size()-pos-1);
	}
	return str;
}

std::string rpCommaWDots(std::string str)
{//  In a std::string, replaces all commas with dots
	int pos = 0;
	while ( (pos = (int)str.find(",")) != -1)
	{
		str = str.substr(0,pos) + "." + str.substr(pos+1,str.size()-pos-1);
	}
	return str;
}

bool isNumber(std::string const & string)
{
  for (auto const & c : string)
  {
    if (!isdigit(c)) return false;
  }
  return true;
}
