#ifndef _ANA_CLEAN_AND_MESSY_DATA__H_
#define _ANA_CLEAN_AND_MESSY_DATA__H_
#include "TreeData.h"
class TFile;
class TTree;
class TH1;
class TH2;

/// A SubsysReco module to analyze the non-embedded and embedded data at once.
class AnaCleanAndMessyData {
  bool m_req_fpga1;
  int m_verb;

  /// Input: clean data
  TFile*      m_cl_file;
  TTree*      m_cl_tree;
  EventData*  m_cl_evt;
  OccData*    m_cl_occ;
  TrackList*  m_cl_trk_true;
  TrackList*  m_cl_trk_reco;
  DimuonList* m_cl_dim_true;
  DimuonList* m_cl_dim_reco;

  /// Input: messy data
  TFile*      m_me_file;
  TTree*      m_me_tree;
  EventData*  m_me_evt;
  OccData*    m_me_occ;
  TrackList*  m_me_trk_true;
  TrackList*  m_me_trk_reco;
  DimuonList* m_me_dim_true;
  DimuonList* m_me_dim_reco;

  /// Output
  TFile* m_out_file;
  TH1* m_h1_rfp00;
  TH1* m_h1_D1   ;
  TH1* m_h1_D2   ;
  TH1* m_h1_D3p  ;
  TH1* m_h1_D3m  ;
  
  TH1* m_h1_trk_pos_cl;
  TH1* m_h1_trk_pos_me;
  TH1* m_h1_trk_neg_cl;
  TH1* m_h1_trk_neg_me;
  TH1* m_h1_dim_cl;
  TH1* m_h1_dim_me;
  TH2* m_h2_ndim;

  TH1* m_h1_mass_cl;
  TH1* m_h1_mass_me;
  TH1* m_h1_mass_diff;
  TH1* m_h1_pz_cl;
  TH1* m_h1_pz_me;
  TH1* m_h1_pz_diff;
  TH1* m_h1_xF_cl;
  TH1* m_h1_xF_me;
  TH1* m_h1_xF_diff;

 public:
  AnaCleanAndMessyData();
  virtual ~AnaCleanAndMessyData();
  void Init(const char* fn_clean, const char* fn_messy);
  void Analyze();
  void End();

  void RequireFpga1(const bool val) { m_req_fpga1 = val; }
  bool RequireFpga1() const  { return m_req_fpga1; }
  void Verbosity(const int verb) { m_verb = verb; }
  int  Verbosity() const  { return m_verb; }

 private:
  void AnalyzeEvent();
  void DrawAndWriteOutput();
};

#endif // _ANA_CLEAN_AND_MESSY_DATA__H_
