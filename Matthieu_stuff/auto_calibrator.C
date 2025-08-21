// Includes of ROOT libraries
#include <TFile.h>
#include <TKey.h>
#include <TH1F.h>
#include <TClass.h>
#include <TROOT.h>
#include <TList.h>
#include <TSpectrum.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TMath.h>
#include <TF1.h>
#include <TGraph.h>

// Includes of C++ libraries
#include <iostream>
#include <string>
#include <regex>
#include <cctype>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <vector>
#include <math.h>
#include <numeric>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <set>
#include <tuple>
#include "./stdc++.h" 

// Structure pour stocker les associations position-énergie
struct PeakEnergy {
    double position;
    double energy;
    PeakEnergy(double p, double e) : position(p), energy(e) {}
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

long calculate_seconds_between(
    const uint Y1, const uint M1, const uint D1,
    const uint H1, const uint m1, const uint S1,
    const uint Y2, const uint M2, const uint D2,
    const uint H2, const uint m2, const uint S2)
{
    auto const
        t1 = std::tie(Y1, M1, D1, H1, m1, S1),
        t2 = std::tie(Y2, M2, D2, H2, m2, S2);

    if (t2 < t1)
        return -calculate_seconds_between(Y2, M2, D2, H2, m2, S2,
                                          Y2, M2, D2, H2, m2, S2);

    int years_days = (Y2 - Y1) * 365;

    // Leap Years
    auto const is_leap_year = [](uint y)
    { return y % 4 ? 0 : y % 100 ? 1
                                 : y % 400 == 0; };
    for (uint i = Y1 + (M1 > 2); i < Y2 + (M2 > 2);)
    {
        if (is_leap_year(i))
        {
            ++years_days;
            i += 4;
        }
        else
        {
            ++i;
        }
    }

    static const uint month_days_sum[] =
        {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
    const uint Y1_days = month_days_sum[M1 - 1] + D1;
    const uint Y2_days = month_days_sum[M2 - 1] + D2;

    // compute total seconds
    const long days = years_days + Y2_days - Y1_days;
    const long hours = days * 24 + H2 - H1;
    const long minutes = hours * 60 + m2 - m1;
    return minutes * 60 + S2 - S1;
}

bool isZero(Double_t i)
{
    return i == 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
// Function to extract the detector name
TString extractDetectorName(const TString& spectrumName) {
    std::string name = spectrumName.Data();
    std::regex pattern1(R"(qdcspectrum(?:2)?(.+))");
    std::regex pattern2(R"(nrjspectrum(.+))");
    std::smatch match;

    if (std::regex_match(name, match, pattern1)) {
        return TString(match[1].str());
    } else if (std::regex_match(name, match, pattern2)) {
        return TString(match[1].str());
    }

    return TString(""); // Return an empty TString if no match is found
}
//
Double_t DoubleTailedStepedGaussian(Double_t *xx, Double_t *pp)
{
    Double_t f_tot = 0.;

    Double_t Back_const = pp[1];
    Double_t Back_slope = pp[2];
    Double_t Back_Exp = pp[3];

    f_tot += (Back_const + (xx[0]) * Back_slope) * exp((xx[0]) * Back_Exp);

    Double_t Ampli = pp[4];
    Double_t Mean = pp[5];
    Double_t Sigma = pp[6] * 1. / sqrt(8. * log(2.));
    Double_t Lambda = pp[7];
    Double_t Rho = pp[8];
    Double_t S = pp[9];

    Double_t U = (xx[0] - Mean) / Sigma;
    Double_t f_g = Ampli * TMath::Exp(-U * U * 0.5);
    Double_t f_lambda = Ampli * TMath::Exp(-0.5 * Lambda * (2. * U - Lambda));
    Double_t f_rho = Ampli * TMath::Exp(-0.5 * Rho * (2. * U - Rho));
    Double_t f_S = Ampli * S * 1. / ((1 + TMath::Exp(U)) * (1 + TMath::Exp(U)));

    if (U < Lambda)
        f_tot += f_lambda;
    else if (U > Rho)
        f_tot += f_rho;
    else
        f_tot += f_g;

    f_tot += f_S;

    return f_tot;
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......



//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
// Function to sort a vector and apply the same permutation to other vectors
template <typename T, typename Compare>
void getSortPermutation(
    std::vector<unsigned> &out,
    const std::vector<T> &v,
    Compare compare = std::less<T>())
{
    out.resize(v.size());
    std::iota(out.begin(), out.end(), 0);

    std::sort(out.begin(), out.end(),
              [&](unsigned i, unsigned j)
              { return compare(v[i], v[j]); });
}

template <typename T>
void applyPermutation(
    const std::vector<unsigned> &order,
    std::vector<T> &t)
{
    assert(order.size() == t.size());
    std::vector<T> st(t.size());
    for (unsigned i = 0; i < t.size(); i++)
    {
        st[i] = t[order[i]];
    }
    t = st;
}

template <typename T, typename... S>
void applyPermutation(
    const std::vector<unsigned> &order,
    std::vector<T> &t,
    std::vector<S> &...s)
{
    applyPermutation(order, t);
    applyPermutation(order, s...);
}

// sort multiple vectors using the criteria of the first one
template <typename T, typename Compare, typename... SS>
void sortVectors(
    const std::vector<T> &t,
    Compare comp,
    std::vector<SS> &...ss)
{
    std::vector<unsigned> order;
    getSortPermutation(order, t, comp);
    applyPermutation(order, ss...);
}

// make less verbose for the usual ascending order
template <typename T, typename... SS>
void sortVectorsAscending(
    const std::vector<T> &t,
    std::vector<SS> &...ss)
{
    sortVectors(t, std::less<T>(), ss...);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Double_t VectorMean(std::vector<Double_t> &theVector)
{
    double sum = std::accumulate(theVector.begin(), theVector.end(), 0.0);
    double mean = sum / theVector.size();

    return mean;
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Double_t VectorWeightedMean(std::vector<Double_t> &theVector, std::vector<Double_t> &theWeight)
{
    double sum(0.);
    double sumweight(0);
    for (auto i = 0; i < (int)theVector.size(); i++)
    {
        sum += theVector.at(i) * theWeight.at(i);
        sumweight += theWeight.at(i);
    }
    double mean = sum / sumweight;

    return mean;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Double_t VectorStdDev(std::vector<Double_t> &theVector)
{
    double mean = VectorMean(theVector);
    std::vector<double> diff(theVector.size());
    std::transform(theVector.begin(), theVector.end(), diff.begin(), [mean](double x)
                   { return x - mean; });
    double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
    double stdev = std::sqrt(sq_sum / theVector.size());

    return stdev;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

std::tuple<std::vector<Double_t>, std::vector<Double_t>, std::vector<Double_t>, Double_t> SetCalibrationSource(TString sourcename, Bool_t Calib_BGO, Bool_t Calib_Ge, Bool_t Calib_LaBr, Bool_t Calib_NaI)
{
    // I define the energies I need to calibrate
    std::vector<Double_t> Ref_nrj;
    std::vector<Double_t> Ref_nrj_err;
    std::vector<Double_t> Ref_nrj_intensity;
    Double_t _Ndecays = 1.;

    // Now I complete these as a function of the source:
    std::map<TString, Bool_t> isSource;
    isSource.insert(std::pair<TString, Bool_t>("Eu", kFALSE));
    isSource.insert(std::pair<TString, Bool_t>("Co", kFALSE));
    isSource.insert(std::pair<TString, Bool_t>("Cs", kFALSE));
    isSource.insert(std::pair<TString, Bool_t>("Ba", kFALSE));
    isSource.insert(std::pair<TString, Bool_t>("Am", kFALSE));
    isSource.insert(std::pair<TString, Bool_t>("AmBe", kFALSE));
    isSource.insert(std::pair<TString, Bool_t>("Ni", kFALSE));
    isSource.insert(std::pair<TString, Bool_t>("Th", kFALSE));
    isSource.insert(std::pair<TString, Bool_t>("natTh", kFALSE));
    isSource.insert(std::pair<TString, Bool_t>("pn", kFALSE));
    isSource.insert(std::pair<TString, Bool_t>("LaBr", kFALSE));
    isSource.insert(std::pair<TString, Bool_t>("Perso", kFALSE));

    // Now I need to decrypt the source name
    // It is a mono-source
    Bool_t isMono = kFALSE;
    if (sourcename.Index("+") == -1)
    {
        isSource[sourcename] = kTRUE;
        isMono = kTRUE;
    }
    else
    {
        while (sourcename.Index("+") != -1)
        {
            // I find the first source name
            int it1 = sourcename.Index("+", 1, sourcename.kExact);
            isSource[sourcename(0, it1)] = kTRUE;
            sourcename = sourcename(it1 + 1, sourcename.Length());
        }
        isSource[sourcename] = kTRUE;
    }

    if (isSource["Eu"])
    {
        std::cout << "Adding 152Eu energies for calibration" << std::endl;
        if (Calib_LaBr)
        {
            // int nrj_nbr = 7;
            // Double_t nrj[7]={121.7817,244.6975,344.2785,778.904,867.378,964.079,1408.006};
            // Double_t nrj_err[7]={0.0003,.0008,.0012,.0024,.003,.018,.003};
            // Double_t intensite[7]={0.2858,0.07583,0.265,0.12942,0.04245,0.14605,0.21005};
            int nrj_nbr = 6;
            Double_t nrj[6] = {121.7817, 244.6975, 344.2785, 778.904, 964.079, 1408.006};
            Double_t nrj_err[6] = {0.0003, .0008, .0012, .0024, .018, .003};
            Double_t intensite[6] = {0.2858, 0.07583, 0.265, 0.12942, 0.14605, 0.21005};

            // I initialize the table of energies
            for (int i = 0; i < nrj_nbr; i++)
            {
                Ref_nrj.push_back(nrj[i]);
                Ref_nrj_err.push_back(nrj_err[i]);
                Ref_nrj_intensity.push_back(intensite[i]);
            }
        }
        else if (Calib_NaI)
        {
            // int nrj_nbr = 7;
            // Double_t nrj[7]={121.7817,244.6975,344.2785,778.904,867.378,964.079,1408.006};
            // Double_t nrj_err[7]={0.0003,.0008,.0012,.0024,.003,.018,.003};
            // Double_t intensite[7]={0.2858,0.07583,0.265,0.12942,0.04245,0.14605,0.21005};
            // I'm going to add the 1085 + 1112 keV peak as the resolution is poor 1098.9715
            int nrj_nbr = 4;
            Double_t nrj[4] = {778.904, 964.079,1098.9715, 1408.006};
            Double_t nrj_err[4] = {.0024,  .018, .004, .003};
            Double_t intensite[4] = {0.12942, 0.14605, 0, 0.21005};

            // I initialize the table of energies
            for (int i = 0; i < nrj_nbr; i++)
            {
                Ref_nrj.push_back(nrj[i]);
                Ref_nrj_err.push_back(nrj_err[i]);
                Ref_nrj_intensity.push_back(intensite[i]);
            }
        }
        else if (Calib_Ge)
        {
            // int nrj_nbr = 8;
            // Double_t nrj[8]={121.7817,244.6975,344.2785,778.904,867.378,964.079,1112.074,1408.006};
            // Double_t nrj_err[8]={0.0003,.0008,.0012,.0024,.003,.018,.003,.003};
            // Double_t intensite[8]={0.2858,0.07583,0.265,0.12942,0.04245,0.14605,0.13644,0.21005};

            int nrj_nbr = 7;
            Double_t nrj[7] = {121.7817, 244.6975, 344.2785, 778.904, 964.079, 1408.006};
            Double_t nrj_err[7] = {0.0003, .0008, .0012, .0024, .018, .003};
            Double_t intensite[7] = {0.2858, 0.07583, 0.265, 0.12942, 0.14605, 0.21005};

            // I initialize the table of energies
            for (int i = 0; i < nrj_nbr; i++)
            {
                Ref_nrj.push_back(nrj[i]);
                Ref_nrj_err.push_back(nrj_err[i]);
                Ref_nrj_intensity.push_back(intensite[i]);
            }
        }

        // I calculate the number fo decays
        // using namespace boost::gregorian;
        // Creation date of the source
        // date date1(2021, Oct, 8);
        // date date2(2003, Feb, 2);
        // long difference = (date1 - date2).days();
        Double_t time_diff = calculate_seconds_between(2021, 10, 6, 13, 0, 0,
                                                       2023, 4, 23, 13, 33, 28);

        // cout << "Age of the source " << difference*24*3600 << " in days" << std::endl;
        std::cout << "Age of the source " << time_diff << " in seconds" << std::endl;

        double half_life = 13.517 * 365.25 * 24 * 3600; // 13.517 y in seconds
        double lambda = TMath::Log(2.) / half_life;
        double _initial_activity = 424.e3; // Bq
        double _activity_at_measure = _initial_activity * TMath::Exp(-1. * lambda * time_diff);
        double tmeas = 2 * 3600 + 7; // seconds
        _Ndecays = _activity_at_measure / lambda * (1 - TMath::Exp(-1. * lambda * tmeas));

        std::cout << "Source d'Eu" << std::endl;
        std::cout << "Created on 06/10/2021 at 13h00" << std::endl;
        std::cout << "With an activity of " << _initial_activity << " Bq" << std::endl;
        std::cout << "Source activity at the moment of the measurement : " << _activity_at_measure << std::endl;
        std::cout << "Measurement lasted for " << tmeas << " seconds" << std::endl;
        std::cout << _Ndecays << " decays were observed" << std::endl;
    }

    if (isSource["Co"])
    {
        std::cout << "Adding 60Co energies for calibration" << std::endl;
        if (Calib_LaBr)
        {
            int nrj_nbr = 2;
            Double_t nrj[2] = {1173.228, 1332.501};
            Double_t nrj_err[2] = {.003, .005};
            Double_t intensite[2] = {0.9985, 0.999856};

            // I initialize the table of energies
            for (int i = 0; i < nrj_nbr; i++)
            {
                Ref_nrj.push_back(nrj[i]);
                Ref_nrj_err.push_back(nrj_err[i]);
                Ref_nrj_intensity.push_back(intensite[i]);
            }
        }
        else if (Calib_NaI)
        {
            int nrj_nbr = 1;
            Double_t nrj[1] = {1173.228};
            Double_t nrj_err[1] = {.003};
            Double_t intensite[1] = {0.9985};

            // I initialize the table of energies
            for (int i = 0; i < nrj_nbr; i++)
            {
                Ref_nrj.push_back(nrj[i]);
                Ref_nrj_err.push_back(nrj_err[i]);
                Ref_nrj_intensity.push_back(intensite[i]);
            }
        }
        else if (Calib_Ge)
        {
            int nrj_nbr = 2;
            Double_t nrj[2] = {1173.228, 1332.501};
            Double_t nrj_err[2] = {.003, .005};
            Double_t intensite[2] = {0.9985, 0.999856};

            // I initialize the table of energies
            for (int i = 0; i < nrj_nbr; i++)
            {
                Ref_nrj.push_back(nrj[i]);
                Ref_nrj_err.push_back(nrj_err[i]);
                Ref_nrj_intensity.push_back(intensite[i]);
            }
        }
    }

    if (isSource["Cs"])
    {
        std::cout << "Adding 137Cs energies for calibration" << std::endl;
        int nrj_nbr = 1;
        Double_t nrj[1] = {661.657};
        Double_t nrj_err[1] = {0.003};
        Double_t intensite[1] = {0.851};

        // I initialize the table of energies
        for (int i = 0; i < nrj_nbr; i++)
        {
            Ref_nrj.push_back(nrj[i]);
            Ref_nrj_err.push_back(nrj_err[i]);
            Ref_nrj_intensity.push_back(intensite[i]);
        }
    }

    if (isSource["Ba"])
    {
        std::cout << "Adding 133Ba energies for calibration" << std::endl;
        int nrj_nbr = 3;
        Double_t nrj[3] = {80.9971, 302.853, 356.017};
        Double_t nrj_err[3] = {0.0014, 0.001, 0.002};
        Double_t intensite[3] = {0.3406, 0.1833, 0.6205};

        // I initialize the table of energies
        for (int i = 0; i < nrj_nbr; i++)
        {
            Ref_nrj.push_back(nrj[i]);
            Ref_nrj_err.push_back(nrj_err[i]);
            Ref_nrj_intensity.push_back(intensite[i]);
        }
    }

    if (isSource["Am"])
    {
        std::cout << "Adding 241Am energies for calibration" << std::endl;
        int nrj_nbr = 1;
        Double_t nrj[1] = {59.5412};
        Double_t nrj_err[1] = {0.0002};
        Double_t intensite[1] = {0.359};

        // I initialize the table of energies
        for (int i = 0; i < nrj_nbr; i++)
        {
            Ref_nrj.push_back(nrj[i]);
            Ref_nrj_err.push_back(nrj_err[i]);
            Ref_nrj_intensity.push_back(intensite[i]);
        }
    }

    if (isSource["AmBe"])
    {
        if (Calib_LaBr)
        {
            std::cout << "Adding AmBe energies for calibration" << std::endl;
            int nrj_nbr = 3;
            Double_t nrj[3] = {3416.91, 3927.91, 4438.91};
            Double_t nrj_err[3] = {1, 1, 1};
            Double_t intensite[3] = {0, 0, 0.56};

            // I initialize the table of energies
            for (int i = 0; i < nrj_nbr; i++)
            {
                Ref_nrj.push_back(nrj[i]);
                Ref_nrj_err.push_back(nrj_err[i]);
                Ref_nrj_intensity.push_back(intensite[i]);
            }
        }
        if (Calib_NaI)
        {
            std::cout << "Adding AmBe energies for calibration" << std::endl;
            int nrj_nbr = 3;
            Double_t nrj[3] = {3416.91, 3927.91, 4438.91};
            Double_t nrj_err[3] = {1, 1, 1};
            Double_t intensite[3] = {0, 0, 0.56};

            // I initialize the table of energies
            for (int i = 0; i < nrj_nbr; i++)
            {
                Ref_nrj.push_back(nrj[i]);
                Ref_nrj_err.push_back(nrj_err[i]);
                Ref_nrj_intensity.push_back(intensite[i]);
            }
        }
        if (Calib_Ge)
        {
            std::cout << "Adding AmBe energies for calibration" << std::endl;
            int nrj_nbr = 3;
            Double_t nrj[3] = {3416.91, 3927.91, 4438.91};
            Double_t nrj_err[3] = {1, 1, 1};
            Double_t intensite[3] = {0, 0, 0.56};

            // I initialize the table of energies
            for (int i = 0; i < nrj_nbr; i++)
            {
                Ref_nrj.push_back(nrj[i]);
                Ref_nrj_err.push_back(nrj_err[i]);
                Ref_nrj_intensity.push_back(intensite[i]);
            }
        }
    }

    if (isSource["Ni"])
    {
        std::cout << "Adding Ni(n,g) energies for calibration" << std::endl;
        int nrj_nbr = 3;
        Double_t nrj[3] = {7975, 8486, 8997};
        Double_t nrj_err[3] = {1, 1, 1};
        Double_t intensite[3] = {0, 0, 0.26};

        // I initialize the table of energies
        for (int i = 0; i < nrj_nbr; i++)
        {
            Ref_nrj.push_back(nrj[i]);
            Ref_nrj_err.push_back(nrj_err[i]);
            Ref_nrj_intensity.push_back(intensite[i]);
        }
    }

    if (isSource["Th"])
    {
        std::cout << "Adding natTh energies for calibration" << std::endl;
        int nrj_nbr = 1;
        Double_t nrj[1] = {2614.533};
        Double_t nrj_err[1] = {0.013};
        Double_t intensite[1] = {0.3725};

        // I initialize the table of energies
        for (int i = 0; i < nrj_nbr; i++)
        {
            Ref_nrj.push_back(nrj[i]);
            Ref_nrj_err.push_back(nrj_err[i]);
            Ref_nrj_intensity.push_back(intensite[i]);
        }
    }

    if (isSource["natTh"])
    {
        std::cout << "Adding natTh energies for calibration" << std::endl;
        int nrj_nbr = 10;
        Double_t nrj[10] = {238.632, 338.320, 463.004, 510.77, 583.191, 727.330, 794.947, 911.204, 968.971, 2614.533};
        Double_t nrj_err[10] = {0.002, 0.003, 0.006, 0.1, 0.002, 0.009, 0.005, 0.004, 0.017, 0.013};
        Double_t intensite[10] = {0.4330, 0.1127, 0.0044, 0.226, 0.317, 0.0658, 0.258, 0.158, 0.3725};

        // I initialize the table of energies
        for (int i = 0; i < nrj_nbr; i++)
        {
            Ref_nrj.push_back(nrj[i]);
            Ref_nrj_err.push_back(nrj_err[i]);
            Ref_nrj_intensity.push_back(intensite[i]);
        }
    }
    
    if (isSource["pn"])
    {
        std::cout << "Adding pn energies for calibration" << std::endl;
        int nrj_nbr = 1;
        Double_t nrj[1] = {2224.566};
        Double_t nrj_err[1] = {0.002};
        Double_t intensite[1] = {0};

        // I initialize the table of energies
        for (int i = 0; i < nrj_nbr; i++)
        {
            Ref_nrj.push_back(nrj[i]);
            Ref_nrj_err.push_back(nrj_err[i]);
            Ref_nrj_intensity.push_back(intensite[i]);
        }
    }

    if (isSource["Perso"])
    {
        TString filename = "BOUIK";
        std::cout << "Adding specific energies from file" << filename << std::endl;
        int nrj_nbr = 1;
        Double_t nrj[1] = {666};
        Double_t nrj_err[1] = {0.666};
        Double_t intensite[1] = {0.666};

        // I initialize the table of energies
        for (int i = 0; i < nrj_nbr; i++)
        {
            Ref_nrj.push_back(nrj[i]);
            Ref_nrj_err.push_back(nrj_err[i]);
            Ref_nrj_intensity.push_back(intensite[i]);
        }
    }

    // If I calibrate LaBr3 I need to add some peaks associated to intrinsic activity
    if (isSource["LaBr"])
    {
        std::cout << "Adding LaBr3 intrinsic energies for calibration" << std::endl;
        int nrj_nbr = 2;
        Double_t nrj[2] = {1435.795, 1460.830};
        Double_t nrj_err[2] = {0.01, 0.003};
        Double_t intensite[2] = {0.66, 0.11};

        // I initialize the table of energies
        for (int i = 0; i < nrj_nbr; i++)
        {
            Ref_nrj.push_back(nrj[i]);
            Ref_nrj_err.push_back(nrj_err[i]);
            Ref_nrj_intensity.push_back(intensite[i]);
        }
    }

    // I order the energies
    if (!isMono)
    {
        // auto p = sort_permutation(Ref_nrj,[](T const& a, T const& b){ /*some comparison*/ });
        //  auto p = sort_permutation(Ref_nrj,[](T const& a, T const& b));
        //
        //  Ref_nrj = apply_permutation(Ref_nrj, p);
        //  Ref_nrj_err = apply_permutation(Ref_nrj_err, p);
        //  Ref_nrj_intensity = apply_permutation(Ref_nrj_intensity, p);
        sortVectors(Ref_nrj, std::less<Double_t>(), Ref_nrj, Ref_nrj_err, Ref_nrj_intensity);
    }

    // Je check qu'il n'y ait pas de zéro dans les énergies et autres vecteurs
    // std::vector<Double_t>::iterator Ref_nrj_Iter = std::remove_if( Ref_nrj.begin() , Ref_nrj.end() , isZero);
    // Ref_nrj.resize( Ref_nrj_Iter -  Ref_nrj.begin() );
    // std::vector<Double_t>::iterator Ref_nrj_err_Iter = std::remove_if( Ref_nrj_err.begin() , Ref_nrj_err.end() , isZero);
    // Ref_nrj_err.resize( Ref_nrj_err_Iter -  Ref_nrj_err.begin() );
    // std::vector<Double_t>::iterator Ref_nrj_intensity_Iter = std::remove_if( Ref_nrj_intensity.begin() , Ref_nrj_intensity.end() , isZero);
    // Ref_nrj_intensity.resize( Ref_nrj_intensity_Iter -  Ref_nrj_intensity.begin() );

    return std::make_tuple(Ref_nrj, Ref_nrj_err, Ref_nrj_intensity, _Ndecays);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......



// Fonction pour calculer la moyenne d'un vecteur
double calculateMean(const std::vector<double>& values) {
    return std::accumulate(values.begin(), values.end(), 0.0) / values.size();
}

// Fonction pour estimer initialement a et b
void estimateInitialCoefficients(const std::vector<double>& positions, const std::vector<double>& energies, double& a, double& b) {
    double meanPosition = calculateMean(positions);
    double meanEnergy = calculateMean(energies);
    a = meanEnergy / meanPosition;
    b = 0.0; // Initialement, nous pouvons supposer que b est 0
}

// Fonction de régression linéaire pour ajuster a et b
void linearRegression(const std::vector<PeakEnergy>& data, double& a, double& b) {
    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
    size_t n = data.size();

    for (const auto& item : data) {
        sumX += item.position;
        sumY += item.energy;
        sumXY += item.position * item.energy;
        sumX2 += item.position * item.position;
    }

    // Calcul des coefficients a et b
    a = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
    b = (sumY - a * sumX) / n;
}

// Fonction pour associer les positions aux énergies en minimisant l'erreur
std::vector<PeakEnergy> associateEnergies(const std::vector<double>& positions, const std::vector<double>& energies, double a, double b) {
    std::vector<PeakEnergy> associations;
    std::vector<bool> associatedPositions(positions.size(), false);
    std::vector<bool> associatedEnergies(energies.size(), false);

    for (size_t i = 0; i < energies.size(); ++i) {
        if (energies[i] == 0.0) {
            continue; // Ignorer les énergies nulles
        }

        double minResidual = std::numeric_limits<double>::max();
        int bestPositionIndex = -1;

        for (size_t j = 0; j < positions.size(); ++j) {
            if (!associatedPositions[j]) {
                double predictedEnergy = positions[j] * a + b;
                double residual = std::abs(predictedEnergy - energies[i]);

                if (residual < minResidual) {
                    minResidual = residual;
                    bestPositionIndex = j;
                }
            }
        }

        if (bestPositionIndex != -1) {
            associations.emplace_back(positions[bestPositionIndex], energies[i]);
            associatedPositions[bestPositionIndex] = true;
            associatedEnergies[i] = true;
        }
    }

    return associations;
}

// Fonction pour vérifier si deux ensembles d'associations sont égaux
bool areAssociationsEqual(const std::vector<PeakEnergy>& a1, const std::vector<PeakEnergy>& a2) {
    if (a1.size() != a2.size()) return false;
    for (size_t i = 0; i < a1.size(); ++i) {
        if (a1[i].position != a2[i].position || a1[i].energy != a2[i].energy) return false;
    }
    return true;
}

// Fonction itérative pour affiner l'association
std::vector<PeakEnergy> refineAssociations(const std::vector<double>& positions, const std::vector<double>& energies, double& a, double& b, int maxIterations = 10) {
    // Estimation initiale de a et b
    if (a == 1 && b == 0) estimateInitialCoefficients(positions, energies, a, b);
    
    std::vector<PeakEnergy> associations;
    std::vector<PeakEnergy> previousAssociations;

    for (int i = 0; i < maxIterations; ++i) {
        previousAssociations = associations;
        associations = associateEnergies(positions, energies, a, b);
        linearRegression(associations, a, b);

        if (areAssociationsEqual(associations, previousAssociations)) {
            std::cout << "Associations have stabilized, stopping iterations." << std::endl;
            break;
        }

        if (a < 0) {
            // Inverser les paires position/énergie si a est négatif
            for (auto& assoc : associations) {
                std::swap(assoc.position, assoc.energy);
            }
            linearRegression(associations, a, b);
        }

        std::cout << "Iteration " << i + 1 << " - Coefficient a: " << a << ", Coefficient b: " << b << std::endl;
        // std::cout << "Associations:" << std::endl;
        // for (const auto& assoc : associations) {
        //     std::cout << "Position: " << assoc.position << ", Energy: " << assoc.energy << std::endl;
        // }
    }

    return associations;
}


std::tuple<double,double,double,double,double,double> PeakFitting_HR(TH1F *h,double position,double gamma_energy,double gamma_energy_err,double gamma_intensity,double _Ndecays)
{
    double peakpos = 0;
    double peakpos_err = 0;
    double peakresolution = 0;
    double peakresolution_err = 0;
    double peakefficiency = 0;
    double peakefficiency_err = 0;

    Double_t xp = position;
    double binwidth = h->GetXaxis()->GetBinWidth(xp);
    Double_t sigma = 40;
    // cout << std::endl << std::endl << "One peak found @ " << xp << std::endl;
    std::vector<Double_t> temp_amplitude;
    std::vector<Double_t> temp_pos;
    std::vector<Double_t> temp_sigma;
    std::vector<Double_t> weight;

    // Define my gaussian around my peak
    TF1 *f1 = new TF1("f1", "gaus", xp - (2 * sigma), xp + (2 * sigma));
    f1->SetParameter(1, xp);
    // f1->SetParLimits(1,xp-sigma,xp+sigma);

    // Then I get amplitude
    Int_t bin = h->GetXaxis()->FindBin(xp);
    Double_t yp = h->GetBinContent(bin);
    f1->SetParameter(0, yp);
    // f1->SetParLimits(0,yp-TMath::Sqrt(yp),yp+TMath::Sqrt(yp));

    // I fix the sigma
    f1->SetParameter(2, sigma);
    // f1->SetParLimits(2,0.,(Double_t)3*sigma);

    // Printing for debug
    // cout << xp-(3*sigma) << "\t" << xp+(3*sigma) << std::endl;
    // cout << xp << " p/m " << xp-sigma << "\t"<<xp+sigma<< std::endl;
    // cout << yp << " p/m " << yp-TMath::Sqrt(yp) << "\t"<<yp+TMath::Sqrt(yp)<< std::endl;
    // cout << sigma << " p/m " << 0 << "\t"<<3*sigma<< std::endl;

    // Then I fit my spectrum with a gaussian
    // cout << "Fitting with f1" << std::endl;
    h->Fit("f1", "RIQE");

    // I store the first fit info
    temp_amplitude.push_back(f1->GetParameter(0));
    temp_pos.push_back(f1->GetParameter(1));
    weight.push_back(1);
    temp_sigma.push_back(f1->GetParameter(2));

    // Printing the result out
    // cout << "temp Peak amplitude " << temp_amplitude << std::endl;
    // cout << "temp Peak mean value " << f1->GetParameter(1) << std::endl;
    // cout << "temp Peak sigma " << temp_sigma << std::endl;

    // I get a second fit with a linear bckground
    TF1 *f2 = new TF1("f2", "gaus(0)+pol1(3)", xp - (2 * sigma), xp + (2 * sigma));
    // cout << xp-(3*sigma) << "\t" << xp+(3*sigma) << std::endl;
    // cout << xp << " p/m " << xp-sigma << "\t"<<xp+sigma<< std::endl;
    // cout << yp << " p/m " << yp-TMath::Sqrt(yp) << "\t"<<yp+TMath::Sqrt(yp)<< std::endl;
    // cout << sigma << " p/m " << 0 << "\t"<<3*sigma<< std::endl << std::endl << std::endl;
    f2->SetParameter(0, temp_amplitude.at(0));
    f2->SetParameter(1, temp_pos.at(0));
    f2->SetParameter(2, temp_sigma.at(0));
    // f2->SetParameter(0,yp);
    // f2->SetParameter(1,xp);
    // f2->SetParameter(2,sigma);
    // f2->SetParLimits(0,yp-TMath::Sqrt(yp),yp+TMath::Sqrt(yp));f2->SetParameter(0,temp_amplitude);
    // f2->SetParLimits(1,xp-sigma,xp+sigma);f2->SetParameter(1,temp_pos);
    // f2->SetParLimits(2,0,3*sigma);f2->SetParameter(2,temp_sigma);
    // cout << "Fitting with f2" << std::endl;
    h->Fit("f2", "RIQE");
    temp_amplitude.push_back(f2->GetParameter(0));
    temp_pos.push_back(f2->GetParameter(1));
    weight.push_back(20);
    temp_sigma.push_back(f2->GetParameter(2));

    // cout << f2->GetParameter(1) << std::endl;

    // And now I Fit with a skewed gaussian
    // TF1 * f3 = new TF1("sgf","2.*gaus(x,[0],[1],[2])*ROOT::Math::normal_cdf([3]*x,1,0)",xp-(2*sigma), xp+(2*sigma));
    // f3->SetParameter(0,temp_amplitude.at(1));
    // f3->SetParameter(1,temp_pos.at(1));
    // f3->SetParameter(2,temp_sigma.at(1));
    // h->Fit("sgf","RIQE");
    // temp_amplitude.push_back(f3->GetParameter(0));
    // temp_pos.push_back(f3->GetParameter(1));
    // temp_sigma.push_back(f3->GetParameter(2));

    // cout << "Fitting with f3" << std::endl;
    TF1 *fFitFunction = new TF1("MyFit", DoubleTailedStepedGaussian, xp - (2 * sigma), xp + (2 * sigma), 10);
    fFitFunction->SetParName(0, "NumberOfPeaks");
    fFitFunction->SetParName(1, "BkgConst");
    fFitFunction->SetParName(2, "BkgSlope");
    fFitFunction->SetParName(3, "BkgExp");
    fFitFunction->SetParName(4 + 0, "Height");
    fFitFunction->SetParName(4 + 1, "Position");
    fFitFunction->SetParName(4 + 2, "FWHM");
    fFitFunction->SetParName(4 + 3, "LeftTail");
    fFitFunction->SetParName(4 + 4, "RightTail");
    fFitFunction->SetParName(4 + 5, "AmplitudeStep");
    fFitFunction->FixParameter(0, 1);
    fFitFunction->SetParameter(1, f2->GetParameter(3));
    fFitFunction->SetParameter(2, f2->GetParameter(4));
    fFitFunction->FixParameter(3, 0.);
    fFitFunction->SetParameter(4, f2->GetParameter(0));
    fFitFunction->SetParameter(5, f2->GetParameter(1));
    fFitFunction->SetParameter(6, f2->GetParameter(2));
    fFitFunction->SetParameter(7, -2.);
    fFitFunction->SetParLimits(7, -5., -0.1);
    fFitFunction->SetParameter(8, 2.);
    fFitFunction->SetParLimits(8, 0.1, 5);
    fFitFunction->SetParameter(9, 0.01);
    fFitFunction->SetParLimits(9, -1., 1.);

    // cout << "Before fitting " << std::endl;
    // for(auto p= 0; p<10; p++)
    // {
    //   cout << fFitFunction->GetParName(p) << "\t" << fFitFunction->GetParameter(p) << std::endl;
    // }

    h->Fit("MyFit", "R0Q");
    // cout << "After fitting " << std::endl;
    // for(auto p= 0; p<10; p++)
    // {
    //   cout << fFitFunction->GetParName(p) << "\t" << fFitFunction->GetParameter(p) << std::endl;
    // }
    temp_amplitude.push_back(fFitFunction->GetParameter(4));
    temp_pos.push_back(fFitFunction->GetParameter(5));
    weight.push_back(5);
    temp_sigma.push_back(fFitFunction->GetParameter(6));

    // cout << fFitFunction->GetParameter(5) << std::endl;

    // Now I calculate the final parameters
    Double_t final_amplitude = VectorWeightedMean(temp_amplitude, weight);
    Double_t final_amplitude_err = TMath::Sqrt(final_amplitude); // TMath::Sqrt(TMath::Power(VectorStdDev(temp_amplitude),2)+TMath::Power(TMath::Sqrt(final_amplitude),2));

    Double_t final_pos = VectorWeightedMean(temp_pos, weight);
    Double_t final_pos_err = VectorStdDev(temp_pos);
    Double_t final_sigma = VectorWeightedMean(temp_sigma,weight);
    Double_t final_sigma_err = TMath::Sqrt(TMath::Power(VectorStdDev(temp_sigma),2)+TMath::Power(binwidth,2));
    
    std::cout << std::endl << std::endl;
    std::cout << "Final Amplitude = " << final_amplitude << " p/m " << final_amplitude_err << std::endl;
    std::cout << "Final Position = " << final_pos << " p/m " << final_pos_err << std::endl;
    std::cout << "Final Sigma = " << final_sigma << " p/m " << final_sigma_err << std::endl;

    
    // Also store the variables that will be returned
    peakpos =  static_cast<double>(final_pos);
    peakpos_err =  static_cast<double>(binwidth);//static_cast<double>(final_pos_err);
    // I calculate resolution
    Double_t cons2 = 2 * TMath::Sqrt(2 * TMath::Log(2));
    Double_t FWHM = cons2 * final_sigma;
    Double_t localresolution = FWHM / final_pos;
    Double_t localresolution_err = TMath::Sqrt(TMath::Power(1 / final_pos * cons2 * final_sigma_err, 2.) + TMath::Power(TMath::Power(final_pos, -2.) * FWHM * final_pos_err, 2));

    peakresolution =  static_cast<double>(localresolution);
    peakresolution_err =  static_cast<double>(localresolution_err);
    std::cout << "Calculated Resolution " << peakresolution << " p/m " << peakresolution_err << std::endl;

 
    // I calculate the integral of the peak
    Double_t final_Int = final_amplitude * final_sigma/binwidth * TMath::Sqrt(2 * TMath::Pi());
    Double_t a = final_amplitude;
    Double_t c = final_sigma/binwidth;
    Double_t Delta_a = TMath::Sqrt(final_amplitude);
    Double_t Delta_c = final_sigma_err/binwidth;
    Double_t cons = 2 * TMath::Pi();
    Double_t final_Int_syst_err = TMath::Sqrt(TMath::Power(c, 2) * TMath::Power(Delta_a, 2) * cons + TMath::Power(a, 2) * TMath::Power(Delta_c, 2) * cons);
    Double_t final_Int_err = TMath::Sqrt(TMath::Power(final_Int_syst_err, 2) + final_Int);
    std::cout << "Final Integral =  " << final_Int << " p/m " << final_Int_err << std::endl;


    Double_t N_gamma_emitted = _Ndecays * gamma_intensity;
    Double_t local_efficiency;
    Double_t u_Ndecays;
    Double_t u1;
    Double_t u2;
    Double_t u3;
    if(_Ndecays !=0 && gamma_intensity != 0)
    {
        u_Ndecays = TMath::Sqrt(_Ndecays);
        u1 = TMath::Power(final_Int_err / (_Ndecays * gamma_intensity), 2);
        u2 = TMath::Power(final_Int / (gamma_intensity) * TMath::Power(-1. * _Ndecays, -2.) * u_Ndecays, 2);
        u3 = 0.; // TMath::Power(final_Int/(_Ndecays)*TMath::Power(-1.*Ref_nrj_intensity.at(pos),-2.)*(Ref_nrj_intensity.at(pos)*1.e-4),2);
        peakefficiency_err = TMath::Sqrt(u1 + u2 + u3);
        peakefficiency = final_Int / N_gamma_emitted;
    }
    if(TMath::Abs(peakefficiency_err/peakefficiency) >=0.1) peakefficiency_err = peakefficiency_err/5.;
    std::cout << "# of decays = " << _Ndecays << " p/m " << TMath::Sqrt(_Ndecays) << std::endl;
    std::cout << "# of emitted g = " << N_gamma_emitted << std::endl;
    std::cout << "Calculated Efficiency " << peakefficiency << " p/m " << peakefficiency_err << std::endl;
    std::cout << std::endl << std::endl;


    return std::make_tuple(peakpos,peakpos_err,peakresolution,peakresolution_err,peakefficiency,peakefficiency_err);
}

std::tuple<double,double,double,double,double,double>PeakFitting(TH1F *h,double position,double gamma_energy,double gamma_energy_err,double gamma_intensity,double _Ndecays)
{
    double peakpos = 0;
    double peakpos_err = 0;
    double peakresolution = 0;
    double peakresolution_err = 0;
    double peakefficiency = 0;
    double peakefficiency_err = 0;
    
    Double_t xp = position;
    double binwidth = h->GetXaxis()->GetBinWidth(xp);
    Double_t sigma = 1500;
    // cout << std::endl << std::endl << "One peak found @ " << xp << std::endl;
    std::vector<Double_t> temp_amplitude;
    std::vector<Double_t> temp_pos;
    std::vector<Double_t> temp_sigma;
    std::vector<Double_t> weight;

    // Define my gaussian around my peak
    TF1 *f1 = new TF1("f1", "gaus", xp - (2 * sigma), xp + (2 * sigma));
    f1->SetParameter(1, xp);
    // f1->SetParLimits(1,xp-sigma,xp+sigma);

    // Then I get amplitude
    Int_t bin = h->GetXaxis()->FindBin(xp);
    Double_t yp = h->GetBinContent(bin);
    f1->SetParameter(0, yp);
    // f1->SetParLimits(0,yp-TMath::Sqrt(yp),yp+TMath::Sqrt(yp));

    // I fix the sigma
    f1->SetParameter(2, sigma);
    // f1->SetParLimits(2,0.,(Double_t)3*sigma);

    // Printing for debug
    // cout << xp-(3*sigma) << "\t" << xp+(3*sigma) << std::endl;
    // cout << xp << " p/m " << xp-sigma << "\t"<<xp+sigma<< std::endl;
    // cout << yp << " p/m " << yp-TMath::Sqrt(yp) << "\t"<<yp+TMath::Sqrt(yp)<< std::endl;
    // cout << sigma << " p/m " << 0 << "\t"<<3*sigma<< std::endl;

    // Then I fit my spectrum with a gaussian
    // cout << "Fitting with f1" << std::endl;
    h->Fit("f1", "RIQE");

    // I store the first fit info
    temp_amplitude.push_back(f1->GetParameter(0));
    temp_pos.push_back(f1->GetParameter(1));
    weight.push_back(1);
    temp_sigma.push_back(f1->GetParameter(2));

    // Printing the result out
    // cout << "temp Peak amplitude " << temp_amplitude << std::endl;
    // cout << "temp Peak mean value " << f1->GetParameter(1) << std::endl;
    // cout << "temp Peak sigma " << temp_sigma << std::endl;

    // I get a second fit with a linear bckground
    TF1 *f2 = new TF1("f2", "gaus(0)+pol1(3)", xp - (2 * sigma), xp + (2 * sigma));
    // cout << xp-(3*sigma) << "\t" << xp+(3*sigma) << std::endl;
    // cout << xp << " p/m " << xp-sigma << "\t"<<xp+sigma<< std::endl;
    // cout << yp << " p/m " << yp-TMath::Sqrt(yp) << "\t"<<yp+TMath::Sqrt(yp)<< std::endl;
    // cout << sigma << " p/m " << 0 << "\t"<<3*sigma<< std::endl << std::endl << std::endl;
    f2->SetParameter(0, temp_amplitude.at(0));
    f2->SetParameter(1, temp_pos.at(0));
    f2->SetParameter(2, temp_sigma.at(0));
    // f2->SetParameter(0,yp);
    // f2->SetParameter(1,xp);
    // f2->SetParameter(2,sigma);
    // f2->SetParLimits(0,yp-TMath::Sqrt(yp),yp+TMath::Sqrt(yp));f2->SetParameter(0,temp_amplitude);
    // f2->SetParLimits(1,xp-sigma,xp+sigma);f2->SetParameter(1,temp_pos);
    // f2->SetParLimits(2,0,3*sigma);f2->SetParameter(2,temp_sigma);
    // cout << "Fitting with f2" << std::endl;
    auto fitResult = h->Fit("f2", "RIQES");
    temp_amplitude.push_back(f2->GetParameter(0));
    temp_pos.push_back(f2->GetParameter(1));
    weight.push_back(20);
    temp_sigma.push_back(f2->GetParameter(2));

    // cout << f2->GetParameter(1) << std::endl;

    // And now I Fit with a skewed gaussian
    // TF1 * f3 = new TF1("sgf","2.*gaus(x,[0],[1],[2])*ROOT::Math::normal_cdf([3]*x,1,0)",xp-(2*sigma), xp+(2*sigma));
    // f3->SetParameter(0,temp_amplitude.at(1));
    // f3->SetParameter(1,temp_pos.at(1));
    // f3->SetParameter(2,temp_sigma.at(1));
    // h->Fit("sgf","RIQE");
    // temp_amplitude.push_back(f3->GetParameter(0));
    // temp_pos.push_back(f3->GetParameter(1));
    // temp_sigma.push_back(f3->GetParameter(2));

    // Now I calculate the final parameters
    Double_t final_amplitude = f2->GetParameter(0);              // VectorMean(temp_amplitude);
    Double_t final_amplitude_err = TMath::Sqrt(final_amplitude); // TMath::Sqrt(TMath::Power(VectorStdDev(temp_amplitude),2)+TMath::Power(TMath::Sqrt(final_amplitude),2));
    Double_t final_pos = f2->GetParameter(1);                    // VectorWeightedMean(temp_pos,weight);
    Double_t final_pos_err = TMath::Sqrt(TMath::Power(f2->GetParError(1), 2) + (binwidth * binwidth));
    Double_t final_sigma =VectorWeightedMean(temp_sigma,weight);
    Double_t final_sigma_err = TMath::Sqrt(TMath::Power(f2->GetParError(2), 2) + TMath::Power(final_sigma / (TMath::Sqrt(4 * sigma / binwidth - 1)), 2)) / (binwidth);

    std::cout << std::endl << std::endl;
    std::cout << "Final Amplitude = " << final_amplitude << " p/m " << final_amplitude_err << std::endl;
    std::cout << "Final Position = " << final_pos << " p/m " << final_pos_err << std::endl;
    std::cout << "Final Sigma = " << final_sigma << " p/m " << final_sigma_err << std::endl;
    peakpos = static_cast<double>(final_pos);
    peakpos_err =  static_cast<double>(binwidth);//peakpos_err =  static_cast<double>(final_pos_err);
    
    
    // // Printing the result out
    // cout << std::endl
    //      << std::endl
    //      << "binwidth " << binwidth * 2. << std::endl;
    // cout << "Peak amplitude " << final_amplitude << " p/m " << final_amplitude_err << std::endl;
    // cout << "Peak mean value " << final_pos << " p/m " << final_pos_err << std::endl;
    // cout << "Peak sigma " << final_sigma << " p/m " << final_sigma_err << std::endl;

    // I calculate the integral of the peak
    Double_t cons = TMath::Sqrt(2 * TMath::Pi());
    Double_t final_Int = final_amplitude * final_sigma/binwidth * cons;
    Double_t a = final_amplitude;
    Double_t c = final_sigma/binwidth;
    Double_t Delta_a = final_amplitude_err;
    Double_t Delta_c = final_sigma_err/binwidth;
    Double_t final_Int_syst_err = TMath::Sqrt(TMath::Power(c * Delta_a * cons, 2) + TMath::Power(a * Delta_c * cons, 2));
    Double_t final_Int_err = TMath::Sqrt(TMath::Power(final_Int_syst_err, 2) + final_Int);
    //cout << "Calculated Integral = " << final_Int << " +/- " << final_Int_err << std::endl;
    std::cout << "Final Integral =  " << final_Int << " p/m " << final_Int_err << std::endl;

    // auto covMatrix = fitResult->GetCovarianceMatrix();
    // cout << "ROOT Integral = " << f2->Integral(xp-(2*sigma), xp+(2*sigma))/binwidth << " +/- " << f2->IntegralError(0,1, fitResult->GetParams() , covMatrix.GetMatrixArray())/binwidth << std::endl;;
    //cout << "size of ref_nrj_intensity " << Ref_nrj_intensity.size() << std::endl;
    // Now that I have the integral, I can calculate the associated efficiency
    // First I calculate the number of emitted gamma rays
    Double_t N_gamma_emitted = _Ndecays * gamma_intensity;
    Double_t local_efficiency;
    Double_t u_Ndecays;
    Double_t u1;
    Double_t u2;
    Double_t u3;
    Double_t local_efficiency_err;    
    if(_Ndecays !=0 && gamma_intensity != 0)
    {
        u_Ndecays = TMath::Sqrt(_Ndecays);
        u1 = TMath::Power(final_Int_err / (_Ndecays * gamma_intensity), 2);
        u2 = TMath::Power(final_Int / (gamma_intensity) * TMath::Power(-1. * _Ndecays, -2.) * u_Ndecays, 2);
        u3 = 0.; // TMath::Power(final_Int/(_Ndecays)*TMath::Power(-1.*Ref_nrj_intensity.at(pos),-2.)*(Ref_nrj_intensity.at(pos)*1.e-4),2);
        peakefficiency_err = TMath::Sqrt(u1 + u2 + u3);
        peakefficiency = final_Int / N_gamma_emitted;
    }
    if(TMath::Abs(peakefficiency_err/peakefficiency) >=0.1) peakefficiency_err = peakefficiency_err/5.;
    std::cout << "# of decays = " << _Ndecays << " p/m " << TMath::Sqrt(_Ndecays) << std::endl;
    std::cout << "# of emitted g = " << N_gamma_emitted << std::endl;
    std::cout << "Calculated Efficiency " << peakefficiency << " p/m " << peakefficiency_err << std::endl;

    // I calculate resolution
    Double_t cons2 = 2 * TMath::Sqrt(2 * TMath::Log(2));
    Double_t FWHM = cons2 * final_sigma;
    Double_t localresolution = FWHM / final_pos;
    Double_t localresolution_err = TMath::Sqrt(TMath::Power(1 / final_pos * cons2 * final_sigma_err, 2.) + TMath::Power(TMath::Power(-1. * final_pos, -2.) * FWHM * final_pos_err, 2));

    peakresolution =  static_cast<double>(localresolution);
    peakresolution_err =  static_cast<double>(localresolution_err);
    if(TMath::Abs(peakresolution_err/peakresolution) >=0.1) peakresolution_err = peakresolution_err/5.;
    

    std::cout << "Calculated Resolution " << peakresolution << " p/m " << peakresolution_err << std::endl;
    std::cout << std::endl << std::endl;

    return std::make_tuple(peakpos,peakpos_err,peakresolution,peakresolution_err,peakefficiency,peakefficiency_err);
}

// Function to read existing calibration data if available
bool readCalibrationData(const TString& filename, std::vector<double>& positions, std::vector<double>& energies) {
    std::ifstream infile(filename.Data());
    if (!infile.is_open()) return false;

    std::string line;
    // Skip the header line
    std::getline(infile, line);

    // Read the data lines
    while (std::getline(infile, line)) {
        std::istringstream data(line);
        double energy, position, dummy;
        data >> energy >> position;
        if (data) {
            positions.push_back(position);
            energies.push_back(energy);
        }
        // Skip the rest of the line
        while (data >> dummy);
    }

    infile.close();
    return true;
}


// Function to perform the calibration
std::tuple<double, double> performCalibration(const std::vector<double>& positions, const std::vector<double>& energies) {
    if (positions.size() != energies.size() || positions.empty()) return std::make_tuple(0.0, 0.0);

    TGraph* graph = new TGraph(positions.size(), &positions[0], &energies[0]);
    TF1* fit = new TF1("fit", "pol1");
    graph->Fit(fit);

    double a = fit->GetParameter(1);
    double b = fit->GetParameter(0);

    std::cout << "Calibration parameters: a = " << a << ", b = " << b << std::endl;

    delete graph; // Clean up
    return std::make_tuple(a, b);
}

void DrawHistogram(TH1F *hist, const char *title)
{
    TCanvas *c = new TCanvas();
    hist->Draw();
    TString titlebis = title; titlebis += ".pdf";
    c->SaveAs(titlebis);
    titlebis=title; titlebis += ".C";
    c->SaveAs(titlebis);
    delete c;
}

int main(int argc, char **argv)
{
    // Vérifier qu'un fichier a été passé en argument
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <file.root>" << std::endl;
        return 1;
    }

    // Nom du fichier ROOT à ouvrir
    const char *fileName = argv[1];
    TString sourcename = argv[2];

    // Ouvrir le fichier ROOT
    TFile *file = TFile::Open(fileName);
    if (!file || file->IsZombie())
    {
        std::cerr << "Erreur : impossible d'ouvrir le fichier " << fileName << std::endl;
        return 1;
    }

    // Obtenir la liste des clés (objets) dans le fichier
    TList *listOfKeys = file->GetListOfKeys();
    TIter next(listOfKeys);
    TKey *key;

    // Expressions régulières pour filtrer les noms des histogrammes
    std::regex pattern1("qdcspectrumPARIS_.*");
    std::regex pattern2("qdcspectrum2PARIS_.*");
    std::regex pattern3("nrjspectrumR.*");
    std::regex excludePattern(".*BGO.*");

    // Parcourir la liste des clés
    while ((key = (TKey *)next()))
    {
        // Obtenir le nom de la classe de l'objet
        TClass *cl = gROOT->GetClass(key->GetClassName());

        // Vérifier si l'objet est un histogramme 1D (TH1F)
        if (cl->InheritsFrom("TH1F"))
        {
            // Lire l'objet dans la mémoire
            TH1F *hist = (TH1F *)key->ReadObj();
            std::string histName = hist->GetName();

            // Vérifier si le nom de l'histogramme correspond aux motifs
            if ((std::regex_match(histName, pattern1) || std::regex_match(histName, pattern2) || std::regex_match(histName, pattern3)) &&
                !std::regex_match(histName, excludePattern))
            {
                std::cout << std::endl << std::endl;
                Bool_t Calib_BGO = kFALSE;
                Bool_t Calib_Ge = kFALSE;
                Bool_t Calib_LaBr = kFALSE;
                Bool_t Calib_NaI = kFALSE;
                if (std::regex_match(histName, pattern1))
                    Calib_LaBr = kTRUE;
                if (std::regex_match(histName, pattern2))
                    Calib_NaI = kTRUE;
                if (std::regex_match(histName, pattern3))
                    Calib_Ge = kTRUE;

                // I define the energies I need to calibrate
                std::vector<Double_t> Ref_nrj;
                std::vector<Double_t> Ref_nrj_err;
                std::vector<Double_t> Ref_nrj_intensity;
                Double_t _Ndecays;

                std::tie(Ref_nrj, Ref_nrj_err, Ref_nrj_intensity, _Ndecays) = SetCalibrationSource(sourcename, Calib_BGO, Calib_Ge, Calib_LaBr, Calib_NaI);
                std::cout << "In the spectra you have to point " << Ref_nrj.size() << " peaks corresponding to:" << std::endl;
                if (Ref_nrj.at(0) == 0)
                    Ref_nrj.erase(Ref_nrj.begin());
                // for (int e = 0; e < (int)Ref_nrj.size(); e++)
                for (auto e : Ref_nrj)
                {
                    std::cout << e << "\tkeV" << std::endl;
                }

                // Afficher le nom de l'histogramme
                std::cout << "Histogramme trouvé : " << hist->GetName() << std::endl;

                // Afficher l'histogramme original
                DrawHistogram(hist, ("Images/"+histName + "_original").c_str());

                // Trouver le premier bin avec zéro
                int firstNonZeroBin = hist->GetNbinsX();
                for (int bin = 1; bin < firstNonZeroBin; bin++)
                {
                    if (hist->GetBinContent(bin) != 0)
                    {
                        firstNonZeroBin = bin;
                        break;
                    }
                }
                int firstZeroBin = hist->GetNbinsX();
                // int start = 0;
                // if(std::regex_match(histName, pattern1) || std::regex_match(histName, pattern2)) start =firstNonZeroBin * 10;
                // if(std::regex_match(histName, pattern3) ) start =firstNonZeroBin * 100;
                // for (int bin = firstZeroBin; bin > start; bin--)
                // {
                //     if (hist->GetBinContent(bin) == 0)
                //     {
                //         firstZeroBin = bin;
                //         //break;
                //     }
                // }

                // Créer un nouvel histogramme pour la région réduite
                int firstBin = 1;
                int nBins = firstZeroBin - firstBin + 1;
                double xMin = hist->GetXaxis()->GetBinCenter(firstBin);
                double xMax = hist->GetXaxis()->GetBinCenter(firstZeroBin);
                TH1F *reducedHist = new TH1F((histName + "_reduced").c_str(), hist->GetTitle(), nBins, xMin, xMax);

                std::cout << "nBins = " << nBins << "; xMin = " << xMin << "; xMax = " << xMax << std::endl;

                // Copier les contenus des bins dans le nouvel histogramme
                for (int bin = firstBin; bin <= firstZeroBin; ++bin)
                {
                    reducedHist->SetBinContent(bin - firstBin + 1, hist->GetBinContent(bin));
                }

                // Afficher l'histogramme réduit
                DrawHistogram(reducedHist, ("Images/"+histName + "_reduced").c_str());

                // Enlever le fond de l'histogramme réduit
                TSpectrum spectrum;
                TH1 *background;
                if (std::regex_match(histName, pattern3))
                    background = spectrum.Background(reducedHist, 30, "BackIncreasingWindow,BackOrder2,BackSmoothing15,Compton,same");
                else if (std::regex_match(histName, pattern1))
                    background = spectrum.Background(reducedHist, 50, "BackIncreasingWindow,BackOrder4,nosmoothing,BackSmoothing15,same");
                else if (std::regex_match(histName, pattern2))
                    background = spectrum.Background(reducedHist, 50, "BackIncreasingWindow,BackOrder6,nosmoothing,BackSmoothing15,same");
                reducedHist->Add(background, -1);

                // Afficher l'histogramme réduit sans fond
                DrawHistogram(reducedHist, ("Images/"+histName + "_no_background").c_str());

                // Trouver les pics dans l'histogramme réduit
                double sigma = 1;
                double intensity = 0.01;
                int nPeaks;
                if(std::regex_match(histName, pattern3))
                {
                    nPeaks = spectrum.Search(reducedHist, 20, "", 0.02);
                }
                if (std::regex_match(histName, pattern1))
                {
                    nPeaks =  spectrum.Search(reducedHist, 5, "nobackground", 0.01);
                }
                if (std::regex_match(histName, pattern2))
                {
                    nPeaks =  spectrum.Search(reducedHist, 5, "", 0.005);
                }
                // if (std::regex_match(histName, pattern2))
                // {
                //     sigma = 5;
                //     intensity = 0.01;
                // }
                // int nPeaks = spectrum.Search(reducedHist, 20, "", 0.02);//spectrum.Search(reducedHist, sigma, "nobackground", intensity);

                std::cout << "Nombre de pics trouvés : " << nPeaks << std::endl;

                // Obtenir les positions des pics
                double *peakPositions = spectrum.GetPositionX();
                                
                // Ok maintenant j'ai une liste de peak il faut que je vérifie si elle match ma liste d'énergies
                std::vector<double> peaks(peakPositions, peakPositions+nPeaks);
                sort(peaks.begin(),peaks.end());
                for (int i = 0; i < nPeaks; ++i)
                {
                    std::cout << "Pic " << i + 1 << " à la position : " << peaks[i] << std::endl;
                }
                
                // Prepare vectors for existing calibration data
                std::vector<double> positions, energies;

                // Attempt to read existing calibration data
                TString calibrationFile="Calib_peak_files/Calib_";
                TString detname = extractDetectorName(hist->GetName());
                calibrationFile+=detname;
                if(Calib_LaBr) calibrationFile += "_Qs";
                if(Calib_NaI) calibrationFile += "_Ql";
                calibrationFile +="_allpeaks.txt";
                if (readCalibrationData(calibrationFile, positions, energies)) {
                    std::cout << "Loaded pre-existing calibration data." << std::endl;
                } else {
                    std::cout << "No pre-existing calibration data found." << std::endl;
                }
                
                double a(1), b(0);
                
                // If calibration data was loaded, perform calibration
                if (!positions.empty() && !energies.empty()) {
                    std::tie(a, b) = performCalibration(positions, energies);
                }
                
                auto associations = refineAssociations(peaks, Ref_nrj, a, b);
                
                std::cout << "Coefficient a: " << a << std::endl;
                std::cout << "Coefficient b: " << b << std::endl;
                
                std::cout << "Associations:" << std::endl;
                for (const auto& assoc : associations) {
                    std::cout << "Position: " << assoc.position << ", Energy: " << assoc.energy << std::endl;
                }
                
                // Maintenant que j'ai la position du pic et son énergie...
                // Je vais boucler sur les pics et les fitter correctement
                // Je déclare les vecteurs qui vont me permettre de stocker les donnees pour les écrire dans un fichier texte à la fin
                std::vector<double> energy;
                std::vector<double> energy_err;
                std::vector<double> position;
                std::vector<double> position_err;
                std::vector<double> resolution;
                std::vector<double> resolution_err;
                std::vector<double> efficiency;
                std::vector<double> efficiency_err;
                
                for(const auto& assoc : associations)
                {
                    std::cout << "Fitting Position: " << assoc.position << " corresponding to Energy: " << assoc.energy  << " keV" << std::endl;
                    //First I need to find the gamma properties from the list
                    Double_t gamma_energy = 0;
                    Double_t gamma_energy_err = 0.;
                    Double_t gamma_intensity = 0.;
                    for(int e = 0; e< Ref_nrj.size(); e++)
                    {
                        if(assoc.energy == Ref_nrj[e])
                        {
                            gamma_energy = Ref_nrj[e];
                            gamma_energy_err = Ref_nrj_err[e];
                            gamma_intensity = Ref_nrj_intensity[e];
                            
                            //Ref_nrj, Ref_nrj_err, Ref_nrj_intensity, _Ndecays
                        }
                    }
                    Double_t peakpos = 0;
                    Double_t peakpos_err = 0;
                    Double_t peakresolution = 0;
                    Double_t peakresolution_err = 0;
                    Double_t peakefficiency = 0;
                    Double_t peakefficiency_err = 0;
                    if(Calib_Ge && hist->GetEntries() !=0) 
                        std::tie(peakpos,peakpos_err, peakresolution, peakresolution_err, peakefficiency, peakefficiency_err) = PeakFitting_HR(hist,assoc.position,gamma_energy,gamma_energy_err,gamma_intensity,_Ndecays);
                    if((Calib_LaBr || Calib_NaI)&& hist->GetEntries() !=0) 
                        std::tie(peakpos,peakpos_err, peakresolution, peakresolution_err, peakefficiency, peakefficiency_err) = PeakFitting(hist,assoc.position,gamma_energy,gamma_energy_err,gamma_intensity,_Ndecays);
                    
                    std::cout << "Measurement for the peak : " << std::endl;
                    std::cout << "Gamma Energy  : " << gamma_energy << "p/m"  <<gamma_energy_err <<  "keV; placed at " << peakpos << " p/m " << peakpos_err << std::endl;
                    std::cout << "Measured with a resolution " << peakresolution << " p/m " << peakresolution_err << std::endl;
                    std::cout << "Measured with an efficiency of " << peakefficiency << " p/m " << peakefficiency_err << std::endl;
                    
                    energy.push_back(gamma_energy);
                    energy_err.push_back(gamma_energy_err);
                    position.push_back(peakpos);
                    position_err.push_back(peakpos_err);
                    resolution.push_back(peakresolution);
                    resolution_err.push_back(peakresolution_err);
                    efficiency.push_back(peakefficiency);
                    efficiency_err.push_back(peakefficiency_err);
                }
                
                // Now all the peaks are fitted
                // I store the results to a .txt file
                TString outcalibmeasure="Calib_peak_files/Calib_";
                TString detectorname = extractDetectorName(hist->GetName());
                outcalibmeasure+=detectorname;
                if(Calib_LaBr) outcalibmeasure += "_Qs";
                if(Calib_NaI) outcalibmeasure += "_Ql";
                outcalibmeasure +="_allpeaks.txt";
                std::ifstream file(outcalibmeasure);
                if(file.good())
                {
                  std::ofstream calibfileperdetect;
                  calibfileperdetect.open(outcalibmeasure, std::ios::app);
                  calibfileperdetect.precision(7);
                  for(auto pos = 0; pos < (int)energy.size(); pos++)
                  {
                    std::cout << "Peak of " << energy.at(pos) << " keV is placed @x = " << position.at(pos) << std::endl;
                    calibfileperdetect << energy.at(pos) << "\t"<< position.at(pos) << "\t"<< energy_err.at(pos) << "\t"<< position_err.at(pos) << "\t" << resolution.at(pos) << "\t" << resolution_err.at(pos)<< "\t" << efficiency.at(pos) << "\t" << efficiency_err.at(pos) << std::endl ;
                  }
                }
                else
                {
                  std::ofstream calibfileperdetect(outcalibmeasure, std::ios::out);
                  calibfileperdetect.precision(7);
                  calibfileperdetect << "NRJ (keV) \t Position \t NRJ_err \t Position_err \t Resolution \t Resolution_err \t Efficiency \t Efficiency_err" << std::endl;
                  for(auto pos = 0; pos < (int)energy.size(); pos++)
                  {
                    std::cout << "Peak of " << energy.at(pos) << " keV is placed @x = " << position.at(pos) << std::endl;
                    calibfileperdetect << energy.at(pos) << "\t"<< position.at(pos) << "\t"<< energy_err.at(pos) << "\t"<< position_err.at(pos) << "\t" << resolution.at(pos) << "\t" << resolution_err.at(pos)<< "\t" << efficiency.at(pos) << "\t" << efficiency_err.at(pos) << std::endl ;
                  }
                }
                
                // Dessiner les pics trouvés sur l'histogramme réduit
                TCanvas *c = new TCanvas();
                reducedHist->Draw();
                spectrum.Draw("same");
                c->SaveAs(("Images/"+histName + "_peaks.pdf").c_str());
                c->SaveAs(("Images/"+histName + "_peaks.C").c_str());
                delete c;

                // Libérer la mémoire
                delete reducedHist;
            }
        }
    }

    // Fermer le fichier ROOT
    file->Close();
    delete file;

    return 0;
}

// Do not delete next comment
// g++ -o auto_calibrator auto_calibrator.C `root-config --cflags --glibs --libs` -lSpectrum && ./auto_calibrator Eu152_UncalibratedEnergyspectra_all.root Eu