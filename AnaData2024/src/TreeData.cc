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

HitData::HitData() 
  : det_id    (0)
  , ele_id    (0)
  , tdc_time  (0)
  , drift_dist(0)
{
  ;
}

BeamData::BeamData()
  : run_id   (0)
  , spill_id (0)
  , event_id (0)
  , fpga_bits(0)
  , nim_bits (0)
  , turn_id  (0)
  , rf_id    (0)
  , rfp01    (0)
  , rfp00    (0)
  , rfm01    (0)
  , d1       (0)
  , d2       (0)
  , d3p      (0)
  , d3m      (0)
{
  ;
}
