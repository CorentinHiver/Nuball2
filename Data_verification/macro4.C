#include "../lib/libRoot.hpp"

void draw(TH1* histo, const char* options = "")
{
  histo->Draw(options);
  gPad->WaitPrimitive();
  gPad->Update();
}

void macro4(std::string filename)
{
  auto file = TFile::Open(filename.c_str(), "read");
  if (!file) {error(filename, "not opened..."); return;}
  auto list_bidim = get_map_histo<TH2F*>(file, "TH2F");
  auto d_VS_delayed_Module_mult = list_bidim["d_VS_delayed_Module_mult"];
  std::vector<TH1D*> projs_d_VS_delayed_Module_mult;
  if (d_VS_delayed_Module_mult)
  {
    for (int mult = 0; mult<int_cast(d_VS_delayed_Module_mult->GetXaxis()->GetXmax()); ++mult)
    {
      projs_d_VS_delayed_Module_mult.push_back(d_VS_delayed_Module_mult->ProjectionY(to_string(mult).c_str(), mult, mult+1));
    }
    for (auto const & proj : projs_d_VS_delayed_Module_mult) {draw(proj);}
  }
  else print("projs_d_VS_delayed_Module_mult not found in file ...");
}