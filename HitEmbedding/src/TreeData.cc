#include "TreeData.h"
using namespace std;

EventData::EventData()
  : job_id(0)
  , event_id(0)
  , trig_bits(0)
  , rfp01(0)
  , rfp00(0)
  , rfm01(0)
  , weight(1.0)
  , rec_stat(0)
  , n_h1x(0)
  , n_h2x(0)
  , n_h3x(0)
  , n_h4x(0)
  , n_d1 (0)
  , n_d2 (0)
  , n_d3 (0)
{
  ;
}

OccData::OccData() 
  : D1 (0)
  , D2 (0)
  , D3p(0)
  , D3m(0)
{
  ;
}

TrackData::TrackData() 
  : charge (0)
  , road_id(0)
  , n_hits (0)
  , chi2   (0)
  , pos_vtx(0,0,0)
  , mom_vtx(0,0,0,0)
{
  ;
}
  
DimuonData::DimuonData() 
  : road_pos(0)
  , road_neg(0)
  , pos_top(true)
  , pos_bot(true)
  , neg_top(true)
  , neg_bot(true)
  , pos(0,0,0)
  , mom(0,0,0,0)
  , mom_pos(0,0,0,0)
  , mom_neg(0,0,0,0)
  , mass(0)
  , pT(0)
  , x1(0)
  , x2(0)
  , xF(0)
  , costh_cs(0)
  , phi_cs(0)
{
  ;
}

EmbQAData::EmbQAData()
  : turn_id(0)
  , rf_id  (0)
  , rfp01  (0)
  , rfp00  (0)
  , rfm01  (0)
  , D1     (0)
  , D2     (0)
  , D3p    (0)
  , D3m    (0)
{
  ;
}
