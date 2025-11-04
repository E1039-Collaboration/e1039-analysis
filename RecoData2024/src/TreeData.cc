#include "TreeData.h"
using namespace std;

EventData::EventData()
  : run_id   (0)
  , spill_id (0)
  , event_id (0)
  , fpga_bits(0)
  , nim_bits (0)
  , D1       (0)
  , D2       (0)
  , D3p      (0)
  , D3m      (0)
{
  ;
}

RoadData::RoadData()
{
  ;
}

DimuonData::DimuonData() 
  : pos_top(false)
  , pos_bot(false)
  , neg_top(false)
  , neg_bot(false)
  , pos(0,0,0)
  , mom(0,0,0,0)
  , mom_target(0,0,0,0)
  , mom_dump  (0,0,0,0)
{
  ;
}

TrackData::TrackData() 
  : charge  (0)
  , road    (0)
  , road_idx(0)
  , n_hits  (0)
  , chisq   (0)
  , chisq_target  (0)
  , chisq_dump    (0)
  , chisq_upstream(0)
  , pos_vtx(0,0,0)
  , mom_vtx(0,0,0,0)
  , pos_st1(0,0,0)
  , mom_st1(0,0,0,0)
  , pos_st3(0,0,0)
  , mom_st3(0,0,0,0)
{
  ;
}
