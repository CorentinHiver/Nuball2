#ifndef LIB_H_CO
#define LIB_H_CO

// *********** STD includes ********* //
#include <any>
#include <array>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <numeric>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

// ********** C includes ************ //
#include <cstdlib>
#include <dirent.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ********** Corentin Lib ************ //
#include "print.hpp"
#include "string_functions.hpp"
#include "files_functions.hpp"
#include "vector_functions.hpp"

//////////////
//   UNITS  //
//////////////

float _ns = 1000.;


////////////////
//    USING   //
////////////////

// Units :
using ushort = unsigned short int  ;
using uchar  = unsigned char       ;
using uint   = unsigned int        ;
using ulong  = unsigned long int   ;
using ulonglong  = unsigned long long int ;
using longlong  = long long int ;

// Containers :
using Bools = std::vector<bool>;
using Strings = std::vector<std::string>;

/////////////////////////////
//    STANDART FUNCTIONS   //
/////////////////////////////

// std::istringstream read_argv(int argc, char** argv)
// {

// }


#endif //LIB_H_CO
