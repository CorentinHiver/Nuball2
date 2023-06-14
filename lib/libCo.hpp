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
//   TYPEDEF  //
////////////////

typedef unsigned short int  ushort;
typedef unsigned char       uchar ;
typedef unsigned int        uint  ;
typedef unsigned long int   ulong ;
typedef unsigned long long int ulonglong ;

#endif //LIB_H_CO
