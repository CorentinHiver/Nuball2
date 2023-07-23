#include "../lib/libRoot.hpp"
#include "../lib/Classes/FilesManager.hpp"

void read_histos_conversion(std::string const & _path)
{
  Path path(_path);
  FilesManager files(path);

  std::string run;
  while(files.getNext(run))
  {
    File (path.string()+run+)
  }
}