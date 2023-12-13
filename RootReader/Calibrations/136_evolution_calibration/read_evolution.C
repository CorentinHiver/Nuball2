#include "../../../lib/libRoot.hpp"
#include "../../../lib/Classes/FilesManager.hpp"

void read_evolution(Path folder = "")
{
  if (folder == Path::pwd()) folder = Path("~/faster_data/N-SI-136-U_histo");
  if (!folder) throw_error(concatenate("Folder ", folder, " do not exist"));
  FilesManager files(folder);
  for (auto const & filename : files)
  {
    auto file = TFile::Open(filename.c_str());
    auto list_names = getTH1FNames(file);
    print(filename);
    for (auto const & name : list_names) std::cout << name << "\n";
    pauseCo();
  }
}