#include "../lib/FasterReader/FasterRunReader.hpp"

using namespace std;
using namespace Colib;

int main(int argc, char** argv)
{
  FasterRootInterface::setTreeInMemory();
  FasterRunReader reader(argc, argv);
  // reader.setEventTrigger([](Event const & event){
  //   for (int hit_i = 0; hit_i<event.mult; ++hit_i) if (NSI136::isGe[event.labels[hit_i]]) return true;
  //   return false;
  // });
  reader.run();
}