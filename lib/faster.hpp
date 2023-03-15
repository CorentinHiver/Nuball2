#ifndef FASTER_H_CO
#define FASTER_H_CO

#include "Hit.h"
#ifdef FASTERAC
// ******* fasterac includes ******* //
#include "fasterac/adc.h"
#include "fasterac/adc_caras.h"
#include "fasterac/electrometer.h"
#include "fasterac/farray.h"
#include "fasterac/fast_data.h"
#include "fasterac/fasterac.h"
#include "fasterac/group.h"
// typedef enum {
//   GROUP_TYPE_ALIAS         =  10,
//   // GROUP_COUNTER_TYPE_ALIAS =  30,
//   GROUP_COUNTER_TYPE_ALIAS =  30,
// } group_const;
#include "fasterac/jdb_hv.h"
#include "fasterac/online.h"
// #include "fasterac/plas.h"
#include "fasterac/qdc.h"
#include "fasterac/qdc_caras.h"
// #include "fasterac/qt2t.h"
#include "fasterac/qtdc.h"
#include "fasterac/rf.h"
#include "fasterac/rf_caras.h"
#include "fasterac/sampler.h"
// #include "fasterac/sampling.h"
#include "fasterac/scaler.h"
#include "fasterac/spectro.h"
#include "fasterac/utils.h"

// *********** ROOT includes *********//
#include "TROOT.h"
#include <iostream>

void TreatTrapez(Hit* hit, const faster_data_p& data)
{  // Load Ge data
   trapez_spectro adc;
   faster_data_load(data, &adc);
   // Set up hit
   hit->nrj = adc.measure;
   hit->pileup = (adc.pileup == 1 || adc.saturated == 1);
}

void TreatCRRC4(Hit* hit, const faster_data_p& data)
{  // Load ADC data
   crrc4_spectro crrc4_adc;
   faster_data_load(data, &crrc4_adc);

   // Set up hit
   hit->nrj = crrc4_adc.measure;
   hit->pileup = (false); //TO BE LOOKED AT
}

// void TreatQDC(Hit* hit, const faster_data_p& data)
// {
//   //read the number of gates
//   //and then fills the corresponding number of nrj
// }

void TreatQDC1(Hit* hit, const faster_data_p& data)
{ // Load QDC data
  qdc_t_x1 qdc;
  faster_data_load(data, &qdc);

  // Set up hit
  hit->nrj = qdc.q1;
  hit->pileup = (qdc.q1_saturated == 1); // No pileup for BGO - they are always piled up!
}

void TreatQDC2(Hit* hit, const faster_data_p& data)
{
  qdc_t_x2 qdc;
  faster_data_load(data, &qdc);
  hit->nrj = qdc.q1;
  #ifdef QDC2
  hit->nrj2 = qdc.q2;
  #endif //QDC2
  hit->pileup = (qdc.q1_saturated == 1 || qdc.q2_saturated == 1);
}

// void TreatQDC3(Hit* hit, const faster_data_p& data)
// {
//    qdc_t_x3 qdc;
//    faster_data_load(data, &qdc);
//
//    // Set up hit
//    hit->nrj = qdc.q1;
//    hit->nrj2 = qdc.q2;
//    hit->nrj3 = qdc.q3;
//    hit->pileup = (qdc.q1_saturated == 1 || qdc.q2_saturated == 1 || qdc.q3_saturated == 1);
// }
//
// void TreatQDC4(Hit* hit, const faster_data_p& data)
// {
//    qdc_t_x4 qdc;
//    faster_data_load(data, &qdc);
//    hit->nrj = qdc.q1;
//    hit->nrj2 = qdc.q2;
//    hit->nrj3 = qdc.q3;
//    hit->nrj4 = qdc.q4;
//    hit->pileup = (qdc.q1_saturated == 1 || qdc.q2_saturated == 1 || qdc.q3_saturated == 1 || qdc.q4_saturated == 1);
// }

void TreatRF(Hit* hit, const faster_data_p& data)
{
   rf_data rf;
   faster_data_load(data, &rf);
   hit->nrj = rf_period_ns(rf)*1000;
   hit->pileup = false;
}
#endif //FASTERAC
#endif //FASTER_H_CO
