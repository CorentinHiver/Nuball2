#include <Faster2Histo.hpp>

int main(int argc, char ** argv)
{
  // MultiHist<TH1F>::verbose(false); // To silent the writting of the histograms
  Faster2Histo convertor(argc, argv);
  return 1;
}