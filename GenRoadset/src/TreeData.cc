#include "TreeData.h"
using namespace std;

SignalData::SignalData()
  : weight(1.0)
  , mass  (0.0)
  , pT    (0.0)
  , xF    (0.0)
  , x1    (0.0)
  , x2    (0.0)
  , mom   (0.0)
  , phi   (0.0)
  , theta (0.0)
  , road_pos(0)
  , road_neg(0)
{
  ;
}

BgData::BgData()
  : run       (0)
  , evt       (0)
  , fpga1     (false)
  , inte_rfp00(0)
  , inte_max  (0)
{
  ;
}

//EventData::EventData()
//  : proc_id(0)
//  , weight(1.0)
//  , trig_bits(0)
//  , rec_stat(0)
//  , n_dim_true(0)
//  , n_dim_reco(0)
//{
//  for (int ii = 0; ii < 4; ii++) {
//    par_id [ii] = 0;
//    par_mom[ii].SetXYZT(0, 0, 0, 0);
//  }
//}
//
//TrackData::TrackData() 
//  : charge(0)
//{
//  ;
//}
//  
//DimuonData::DimuonData() 
//  : pdg_id(0)
//  , mass(0)
//  , pT(0)
//  , x1(0)
//  , x2(0)
//  , xF(0)
//  , costh(0)
//  , phi(0)
//{
//  ;
//}
