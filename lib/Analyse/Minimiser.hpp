#ifndef MINIMISERVARIABLE_HPP
#define MINIMISERVARIABLE_HPP

#include "TH2F.h"
#include "TH3F.h"

#include "../libCo.hpp"
#include "../Classes/CalibAndScale.hpp"

namespace Colib
{
  /// @brief First-order interpolation to get the value of a non-integer bin, most likely from calibration
  /// @param histo 
  /// @param calibrated_bin 
  /// @return 
  double linearInterpolatedBinContent(TH1* histo, double const & calibrated_bin)
  {
    int bin_i = static_cast<int>(calibrated_bin); //bin_i
    if (0 < bin_i || bin_i > (histo->GetNbinsX()-2)) return 0; // todo Faire attention au -2, c'est peut-être -1 ou -3 dans un TH1
    auto const & a = histo->GetBinContent(bin_i+1) - histo->GetBinContent(bin_i);// a  =  y_i+1 - y_i
    auto const & b = histo->GetBinContent(bin_i)   - a*bin_i;                    // b  =  y_i - a*bin_i
    return a*calibrated_bin+b;
  }

  double linearInterpolatedXContent(TH1F* hist, double const & x)
  {
    int bin = hist->FindBin(x);
    
    // Prevent out-of-range access
    if (bin <= 0) return hist->GetBinContent(bin);
    if (bin >= hist->GetNbinsX()) bin = hist->GetNbinsX() - 1;

    double x0 = hist->GetBinCenter(bin);
    double y0 = hist->GetBinContent(bin);

    double x1 = hist->GetBinCenter(bin + 1);
    double y1 = hist->GetBinContent(bin + 1);

    if (x1 == x0) return y0; // prevent division by zero

    return y0 + (y1 - y0) * (x - x0) / (x1 - x0);
  }

  /// @brief Quadratic interpolation to get the value of any random X value
  /// @param hist 
  /// @param x 
  /// @return 
  double quadraticInterpolatedXContent(TH1F* hist, double const & x)
  {
    int bin = hist->FindBin(x);

    // Ensure enough bins around for quadratic interpolation
    if (bin <= 1) bin = 2;
    if (bin >= hist->GetNbinsX() - 1) bin = hist->GetNbinsX() - 1;

    double x0 = hist->GetBinCenter(bin - 1);
    double y0 = hist->GetBinContent(bin - 1);

    double x1 = hist->GetBinCenter(bin);
    double y1 = hist->GetBinContent(bin);

    double x2 = hist->GetBinCenter(bin + 1);
    double y2 = hist->GetBinContent(bin + 1);

    // Lagrange quadratic interpolation formula
    double L0 = ((x - x1)*(x - x2)) / ((x0 - x1)*(x0 - x2));
    double L1 = ((x - x0)*(x - x2)) / ((x1 - x0)*(x1 - x2));
    double L2 = ((x - x0)*(x - x1)) / ((x2 - x0)*(x2 - x1));

    return y0 * L0 + y1 * L1 + y2 * L2;
  }
  
  class Chi2Calculator
  {
  public:
    Chi2Calculator(TH1* reference) : m_reference(reference){}
    template<class THist>
    double operator()(THist* const test, CalibAndScale const & calib)
    {
      auto testCal = calib(test);
      testCal->SetName((test->GetName() + std::string("calib : ") + mergeStrings(calib.get(), "_")).c_str());
      return calculate(testCal);
    }
    
    template<class THist> double calculate(THist* const testCal) const
    {
      double sum_errors_squared = 0.0;
      auto const & bins = testCal->GetNbinsX();
  
      for (int bin = 0; bin<bins; bin++) if (testCal->GetBinContent(bin)>0)
      {
        // Calculate the difference for this bin :
        auto const & diff = m_reference->GetBinContent(bin)-testCal->GetBinContent(bin);
  
        // Variance of the bin :
        double const & weight = 1/testCal->GetBinContent(bin); // V = sigma² = 1/N
  
        // Add the diff to the total squared diff of the spectra :
        sum_errors_squared += diff*diff*weight;
  
      }
      return sum_errors_squared/bins;
    }

    double calculate(CalibAndScale const & calib) const
    {
      double sum_errors_squared = 0.0;

      auto const & bins = calib.getHisto()->GetNbinsX();
  
      for (int bin = 0; bin<bins; bin++) if (calib[bin]>0)
      {
        // Calculate the difference for this bin :
        auto const & diff = m_reference->GetBinContent(bin)-calib[bin];
  
        // Variance of the bin :
        double const & weight = 1/calib[bin]; // V = sigma² = 1/N
  
        // Add the diff to the total squared diff of the spectra :
        sum_errors_squared += diff*diff*weight;
  
      }
      return sum_errors_squared/bins;
    }

    double calculateForMinuit(double const * par)
    {
      double sum_errors_squared = 0.0;

      auto calib = *m_calib; // Dereference aliasing for elegance and effiency
      calib = {par[0], par[1], par[2]};

      auto const & bins = calib.getHisto()->GetNbinsX();
  
      for (int bin = 0; bin<bins; bin++) if (calib[bin]>0)
      {
        // Calculate the difference for this bin :
        auto const & diff = m_reference->GetBinContent(bin)-calib[bin];
  
        // Variance of the bin :
        double const & weight = 1/calib[bin]; // V = sigma² = 1/N
  
        // Add the diff to the total squared diff of the spectra :
        sum_errors_squared += diff*diff*weight;
  
      }
      return sum_errors_squared/bins;
    }

    void setCalibForMinuit(CalibAndScale * calib)
    {
      m_calib = calib;
    }
    
  private:
    CalibAndScale * m_calib = nullptr;
    TH1* m_reference = nullptr;
  };
  
  struct MinimiserVariable
  {
    double initGuess = 0.;
    double bound = 0.;
    double step = 0.;
    int nb_steps = 0;
    double min = 0; 
    double max = 0; 
  
    MinimiserVariable(std::initializer_list<double> init)
    {
      auto it = init.begin();
      initGuess = double_cast(*it++);
      bound = double_cast(*it++);
      nb_steps = int_cast(*it++);
      initialize();
    }
  
    MinimiserVariable& operator=(std::initializer_list<double> init)
    {
      auto it = init.begin();
      initGuess = double_cast(*it++);
      bound = double_cast(*it++);
      nb_steps = int_cast(*it++);
      initialize();
      return *this;
    }

    friend std::ostream& operator<<(std::ostream& out, MinimiserVariable const & minvar)
    {
      out << 
            " initGuess " << minvar.initGuess <<
            " bound "     << minvar.bound     <<
            " step "      << minvar.step      <<
            " nb_steps "  << minvar.nb_steps  <<
            " min "       << minvar.min       <<
            " max "       << minvar.max       << 
            std::endl;
      return out ;
    }
  
  private:
    void initialize() 
    {
      step = bound / nb_steps;
      min = initGuess - nb_steps * step;
      max = initGuess + nb_steps * step;
    }
  };
  
  class Minimiser
  {
  public:
    Minimiser(){}

    void calculator(Chi2Calculator & chi2Calc, CalibAndScale & calib)
    {
      auto chi2 = chi2Calc.calculate(calib);
      if (chi2<m_min_chi2) 
      {
        m_min_chi2 = chi2;
        m_calib = calib;
      }
      if (s_fill_histo) 
      {
        auto parameters = calib.get();
        m_chi2map->Fill(parameters[0], parameters[1], parameters[2], chi2);
      }
    }
    
    template<class THist>
    void calculate(THist* reference, THist* test, MinimiserVariable xParam, MinimiserVariable yParam, MinimiserVariable zParam)
    {
      Chi2Calculator chi2Calc(reference);
      this->calculate(chi2Calc, test, xParam, yParam, zParam);
    }

    template<class THist>
    void calculate(Chi2Calculator & chi2Calc, THist* test, MinimiserVariable xParam, MinimiserVariable yParam, MinimiserVariable zParam)
    {
      CalibAndScale calib(test);
      chi2Calc.setCalibForMinuit(&calib);
      m_calib.setHisto(test);
      if (m_bruteforce)
      {
        if (s_fill_histo) m_chi2map = new TH3F("chi2map", "chi2map;x;y;z", 
                            xParam.nb_steps*2+1, xParam.min, xParam.max*(1+1e-5), 
                            yParam.nb_steps*2+1, yParam.min, yParam.max*(1+1e-5),
                            zParam.nb_steps*2+1, zParam.min, zParam.max*(1+1e-5)
                          );

        for (int stepx = 0; stepx<xParam.nb_steps*2; ++stepx) {
          for (int stepy = 0; stepy<yParam.nb_steps*2; ++stepy) {
            for (int stepz = 0; stepz<zParam.nb_steps*2; ++stepz) {
              calib = {
                xParam.min + stepx*xParam.step, 
                yParam.min + stepy*yParam.step,
                zParam.min + stepz*zParam.step
              };
  
              calculator(chi2Calc, calib);
            }
          }
        }
      }
      else
      {
      #ifdef INCLUDE_MINUIT
        // ROOT::Math::Minimizer* minimizer = new ROOT::Minuit2::Minuit2Minimizer("Migrad") ;
        auto minimizer = new ROOT::Minuit2::Minuit2Minimizer(ROOT::Minuit2::kMigrad);
        // ROOT::Math::Minimizer* minimizer = ROOT::Math::Factory::CreateMinimizer("Minuit2", "Migrad");
        
        // Set properties
        minimizer->SetMaxFunctionCalls(100000); // for Minuit/Minuit2
        // minimizer->SetMaxIterations(10000);     // for GSL
        minimizer->SetTolerance(0.001);
        minimizer->SetPrintLevel(1);            // 0: silent, 1: default
        
  
        // Create Functor object
        auto func = [&chi2Calc](const double* x) {
          return chi2Calc.calculateForMinuit(x);
        };
        ROOT::Math::Functor f(func, 3);
  
        // Set function and initial values
        minimizer->SetFunction(f);
        minimizer->SetVariable(0, "param_x0", 0.0, xParam.step);
        minimizer->SetVariable(1, "param_x1", 1.0, yParam.step);
        minimizer->SetVariable(2, "param_x2", 1.0, zParam.step);
        
        // Perform minimization
        minimizer->Minimize();
  
        // Get results
        const double* xs = minimizer->X();
        m_calib = {xs[0], xs[1], xs[2]};
        // std::cout << "Minimum at: x0 = " << xs[0] << ", x1 = " << xs[1] << ", x2 = " << xs[2] << std::endl;
        // std::cout << "Minimum value: f = " << minimizer->MinValue() << std::endl;
      #else 
        throw_error("Compile with -DINCLUDE_MINUIT");
      #endif //INCLUDE_MINUIT
      }
    }
  
    auto const & getCalib() const {return m_calib;}
    auto const & getMinChi2() const {return m_min_chi2;}
    auto & getChi2Map() const {return m_chi2map;}
  
    void brutefore(bool const & b) {m_bruteforce = b;}
    void multistages(int nb_stages)
    {
      m_nb_stages = nb_stages;
      m_multistages = true;
    }
  
    static void fillHisto(bool const & b) {s_fill_histo = b;}
  
  private:
    static bool s_fill_histo; 
    bool m_bruteforce = true;
    bool m_multistages = false;
    int m_nb_stages = 1;
    double m_min_chi2 = 1e100;
    CalibAndScale m_calib;
    TH3F* m_chi2map = nullptr;
  };
  
  bool Minimiser::s_fill_histo = true; 
}
#endif //MINIMISERVARIABLE_HPP
