#ifndef _TREE_DATA__H_
#define _TREE_DATA__H_
#include <vector>
#include <TROOT.h>
#include <TLorentzVector.h>

struct EventData {
  short run_id;
  int   spill_id;
  int   event_id;
  short fpga_bits;
  short nim_bits;
  short D1;
  short D2;
  short D3p;
  short D3m;

  EventData();
  virtual ~EventData() {;}
  ClassDef(EventData, 1);
};

struct RoadData {
  std::vector<int> pos_top;
  std::vector<int> pos_bot;
  std::vector<int> neg_top;
  std::vector<int> neg_bot;

  RoadData();
  virtual ~RoadData() {;}
  ClassDef(RoadData, 1);
};


struct DimuonData {
  int            road_pos;
  int            road_neg;
  bool           pos_top;
  bool           pos_bot;
  bool           neg_top;
  bool           neg_bot;
  TVector3       pos;
  TLorentzVector mom;
  short          n_hits_pos;
  double         chisq_pos ;
  double         chisq_target_pos;
  double         chisq_dump_pos;
  double         chisq_upstream_pos;
  TVector3       pos_pos   ;
  TLorentzVector mom_pos   ;
  TVector3       pos_target_pos;
  TVector3       pos_dump_pos;
  short          n_hits_neg;
  double         chisq_neg ;
  double         chisq_target_neg;
  double         chisq_dump_neg;
  double         chisq_upstream_neg;
  TVector3       pos_neg   ;
  TLorentzVector mom_neg   ;
  TVector3       pos_target_neg;
  TVector3       pos_dump_neg;
  TLorentzVector mom_target; ///< Dimuon momentum with choice = 1.
  TLorentzVector mom_dump  ; ///< Dimuon momentum with choice = 2.
  
  DimuonData();
  virtual ~DimuonData() {;}
  ClassDef(DimuonData, 1);
};

typedef std::vector<DimuonData> DimuonList;

struct TrackData {
  short          charge;
  int            road; // road_id
  short          road_idx;
  short          n_hits;
  double         chisq ;
  double         chisq_target;
  double         chisq_dump;
  double         chisq_upstream;
  TVector3       pos_vtx;
  TLorentzVector mom_vtx;
  TVector3       pos_st1;
  TLorentzVector mom_st1;
  TVector3       pos_st3;
  TLorentzVector mom_st3;
  
  TrackData();
  virtual ~TrackData() {;}
  ClassDef(TrackData, 1);
};

typedef std::vector<TrackData> TrackList;

#endif // _TREE_DATA__H_
