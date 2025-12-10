#ifndef _TREE_DATA__H_
#define _TREE_DATA__H_
#include <phool/PHObject.h>
#include <TLorentzVector.h>

struct EventData {
  int proc_id;
  int par_id[4]; // 2 -> 2
  TLorentzVector par_mom[4];
  double weight;
  int trig_bits;
  int rec_stat;
  int n_dim_true;
  int n_dim_reco;

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
typedef std::vector<RoadData> RoadList;

struct TrackData {
  int            charge;
  TVector3       pos_vtx;
  TLorentzVector mom_vtx;

  TrackData();
  virtual ~TrackData() {;}

  ClassDef(TrackData, 1);
};
typedef std::vector<TrackData> TrackList;

struct DimuonData {
  int            pdg_id;
  TVector3       pos;
  TLorentzVector mom;
  TLorentzVector mom_pos;
  TLorentzVector mom_neg;
  double         mass;
  double         pT;
  double         x1;
  double         x2;
  double         xF;
  double         costh;
  double         phi;

  DimuonData();
  virtual ~DimuonData() {;}

  ClassDef(DimuonData, 1);
};
typedef std::vector<DimuonData> DimuonList;

struct RecoTrackData {
  short          charge;
  int            road;
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
  TVector3       pos_target;
  TVector3       pos_dump;
  
  RecoTrackData();
  virtual ~RecoTrackData() {;}
  ClassDef(RecoTrackData, 1);
};
typedef std::vector<RecoTrackData> RecoTrackList;

struct RecoDimuonData {
  bool           pos_top;
  bool           pos_bot;
  bool           neg_top;
  bool           neg_bot;
  TVector3       pos;
  TLorentzVector mom;
  TLorentzVector mom_target; ///< Dimuon momentum with choice = 1.
  TLorentzVector mom_dump  ; ///< Dimuon momentum with choice = 2.
  RecoTrackData  trk_pos;
  RecoTrackData  trk_neg;
  
  RecoDimuonData();
  virtual ~RecoDimuonData() {;}
  ClassDef(RecoDimuonData, 1);
};
typedef std::vector<RecoDimuonData> RecoDimuonList;

#endif /* _TREE_DATA__H_ */
