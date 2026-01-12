#pragma once

#include "FasterRootInterface.hpp"
#include "../Classes/Event.hpp"

class FasterRunReader
{
public:
  FasterRunReader() noexcept = default;

  FasterRunReader(int argc, char** argv)
  {
    
  }

  void addFiles(std::string const & files, int nb = -1)
  {
    auto const & filenames = Colib::findFilesWildcard(files);
    for (auto const & filename : filenames) m_files.push_back(filename);
    if (0 < nb) m_files = Colib::subVec(m_files, nb);
  }

  void addFolder(std::string folder, int nb = -1)
  {
    if (folder.back() != '/') folder.push_back('/');
    auto const & filenames = Colib::findFilesWildcard(folder+"*");
    for (auto const & filename : filenames) m_files.push_back(filename);
    if (0 < nb) m_files = Colib::subVec(m_files, nb);
  }

  bool readNextFile(bool eventBuilding = true)
  {
    if (m_files.empty()) Colib::throw_error("No files");
    if (m_files.size() <= m_file_i) return false;
    auto const & file = m_files[m_file_i++];
    m_reader.loadDatafile(file);
    m_reader.timeSorting();
    std::string outFolder = "./"; // Externalise
    auto outFile = Colib::removeExtension(Colib::removePath(file))+".root";
    if (eventBuilding) m_reader.writeEvents(outFolder+outFile);
    else               m_reader.writeHits  (outFolder+outFile);
    return true;
  }

  void setMaxHits(int nb) {m_reader.setMaxHits(nb);}
  void setHitTrigger  (HitTrigger          trigger    ) {m_reader.setHitTrigger  (trigger)    ;}
  void setEventTrigger(EventTrigger        trigger    ) {m_reader.setEventTrigger(trigger)    ;}
  void setTimeWindow  (Time                time_window) {m_reader.setTimeWindow  (time_window);}
  void setTimeShifts  (std::string const & tsFile     ) {m_reader.loadTimeshifts (tsFile)     ;}

  void convert()
  {
    if (m_files.empty()) Colib::throw_error("No files");
    while(readNextFile()) continue;
  }

  void mergeAndConvert(bool eventBuilding = true)
  {
    if (m_files.empty()) Colib::throw_error("No files");
    m_reader.loadDatafiles(m_files);
    m_reader.timeSorting();
    m_reader.checkTimeSorting();
    std::string outFolder = "./"; // Externalise
    auto outFile = outFolder + Colib::removeLastPart(Colib::removePath(m_files[0]), '_')+".root";
    if (eventBuilding) m_reader.writeEvents(outFile);
    else               m_reader.writeHits  (outFile);
  }

  void mergeAndConvertWithRef(Label refLabel)
  {
    if (m_files.empty()) Colib::throw_error("No files");
    m_reader.loadDatafiles(m_files);
    m_reader.timeSorting();
    std::string outFolder = "./"; // Externalise
    auto outFile = outFolder + Colib::removeLastPart(Colib::removePath(m_files[0]), '_')+".root";
    m_reader.writeEventsWithRef(refLabel, outFile);
  }

private:
  std::vector<std::string> m_files;
  FasterRootInterface m_reader;

  // readNextFile() state variables :
  size_t m_file_i = 0;
};