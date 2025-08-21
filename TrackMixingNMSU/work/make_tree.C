//R__LOAD_LIBRARY(ktracker)
//R__LOAD_LIBRARY(sqgenfit)
R__LOAD_LIBRARY(E906Ana)
#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TCanvas.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <TH1D.h>
#include <TMultiGraph.h>
#include <ktracker/SRecEvent.h>
#include <ktracker/SRawEvent.h>
#include <TLorentzVector.h>
using namespace std;

UtilTrigger::TrigRoadset rs;

//int dim_origin(SRecTrack* trk_pos, SRecTrack* trk_neg);
bool SelectDimuon(SRecDimuon* dim, SRecTrack* trk_pos, SRecTrack* trk_neg);
int GetSpin(const int run_id);

void make_tree(const string list_run_spill="list_run_spill.txt")
{   
  vector<int> list_run;
  ifstream ifs(list_run_spill);
  if (ifs.is_open()) {
    int run_id, spill_id;
    while (ifs >> run_id >> spill_id) list_run.push_back(run_id);
    ifs.close();
  } else {
    std::cout<<"___file for run ID "<<list_run_spill<<" could not be opened"<<std::endl;
    std::exit(1);
  }
  sort(list_run.begin(), list_run.end());
  list_run.erase(unique(list_run.begin(), list_run.end()), list_run.end());

  rs.LoadConfig(131); // Roadset ID.
  
  TFile* f_out = new TFile("tree.root", "RECREATE");
  int run_id;
  int spin;
  double px, py, pz;
  double mass;
  TTree* tr_org = new TTree("tree_org", "");
  tr_org->Branch("run_id", &run_id);
  tr_org->Branch("spin"  , &spin);
  tr_org->Branch("px"    , &px);
  tr_org->Branch("py"    , &py);
  tr_org->Branch("pz"    , &pz);
  tr_org->Branch("mass"  , &mass);
  TTree *tr_mix = new TTree("tree_mix", "");
  tr_mix->Branch("run_id", &run_id);
  tr_mix->Branch("spin"  , &spin);
  tr_mix->Branch("px"    , &px);
  tr_mix->Branch("py"    , &py);
  tr_mix->Branch("pz"    , &pz);
  tr_mix->Branch("mass"  , &mass);

  unsigned int n_run = list_run.size();
  for (unsigned int i_run = 0; i_run < n_run; i_run++) {
    run_id = list_run[i_run];
    ostringstream oss;	
    oss << "scratch/run_" << setfill('0') << setw(6) << run_id << "/out/output.root";
    const string fn_in = oss.str();
    std::cout << "Input [" << i_run << "/" << n_run << "] " << fn_in << std::endl;
    
    TFile* f_file = new TFile(fn_in.c_str());
    if(!f_file->IsOpen()){
      cout << "Cannot get the DST tree.  Abort." << endl;
      exit(1);
    }
    TTree* _sorted = (TTree*) f_file->Get("save_sorted");
    TTree* _mixed  = (TTree*) f_file->Get("save_mix");	
    if (!_sorted || !_mixed) {
      cout << "Cannot get the sorted/mix trees.  Abort." << endl;
      exit(2);
    }
    SRecEvent* sorted_event = new SRecEvent();
    SRecEvent* mixed_event = new SRecEvent();
    std::vector<SRecTrack> * pos_tracks =0;
    std::vector<SRecTrack> * neg_tracks =0;
    _sorted->SetBranchAddress("recEvent", &sorted_event);
    _mixed->SetBranchAddress("recEvent", &mixed_event);

    spin = GetSpin(run_id);
    
    int n_sorted = _sorted ->GetEntries();
    for (int i= 0; i < n_sorted; i++){
      _sorted->GetEntry(i);
      for (int n_dims=0; n_dims <sorted_event->getNDimuons(); n_dims++){
        SRecDimuon s_dim = sorted_event->getDimuon(n_dims);
        SRecTrack* trk_pos = &(sorted_event->getTrack(s_dim.get_track_id_pos()));
        SRecTrack* trk_neg = &(sorted_event->getTrack(s_dim.get_track_id_neg()));

        if (! SelectDimuon(&s_dim, trk_pos, trk_neg)) continue;
        TLorentzVector mom = s_dim.p_pos_target + s_dim.p_neg_target;
        px = mom.X();
        py = mom.Y();
        pz = mom.Z();
        s_dim.calcVariables(1); //1 : re-fit to target, 2: re-fit to dump
        mass = s_dim.get_mass();
        tr_org->Fill();
      }
    }

    int n_mixed = _mixed ->GetEntries();
    for (int j= 0; j < n_mixed; j++){
      _mixed->GetEntry(j);
      for (int n_dims=0; n_dims <mixed_event->getNDimuons(); n_dims++){
        SRecDimuon m_dim = mixed_event->getDimuon(n_dims);
        SRecTrack* trk_pos = &(mixed_event->getTrack(m_dim.get_track_id_pos()));
        SRecTrack* trk_neg = &(mixed_event->getTrack(m_dim.get_track_id_neg()));

        if (! SelectDimuon(&m_dim, trk_pos, trk_neg)) continue;
        TLorentzVector mom = m_dim.p_pos_target + m_dim.p_neg_target;
        px = mom.X();
        py = mom.Y();
        pz = mom.Z();
        m_dim.calcVariables(1);
        mass = m_dim.get_mass();
        tr_mix->Fill();
      }
    }
  }
  
  f_out->cd();
  tr_org->Write();
  tr_mix->Write();
  f_out->Close();
  exit(0);
}

//int dim_origin(SRecTrack* trk_pos, SRecTrack* trk_neg)
//{
//  double z_pos = trk_pos->get_pos_vtx().Z();
//  double z_neg = trk_neg->get_pos_vtx().Z();
//
//  double pos_chisq_t  = trk_pos->getChisqTarget();
//  double pos_chisq_d  = trk_pos->getChisqDump();
//  double pos_chisq_us = trk_pos->get_chsiq_upstream();
//
//  double neg_chisq_t  = trk_neg->getChisqTarget();
//  double neg_chisq_d  = trk_neg->getChisqDump();
//  double neg_chisq_us = trk_neg->get_chsiq_upstream();
//  
//  double pos_chi_td  = pos_chisq_t - pos_chisq_d;
//  double pos_chi_tus = pos_chisq_t - pos_chisq_us;
//  double neg_chi_td  = neg_chisq_t - neg_chisq_d;
//  double neg_chi_tus = neg_chisq_t - neg_chisq_us;
//  
//  //
//  //Target_Like:- 0
//  //
//  bool tgt_pos_ok = pos_chisq_t >=0 && pos_chisq_d >=0 && pos_chisq_us >=0 &&
//    pos_chi_td <0 && pos_chi_tus <0;
//  bool tgt_neg_ok = neg_chisq_t >=0 && neg_chisq_d >=0 && neg_chisq_us >=0 &&
//    neg_chi_td <0 && neg_chi_tus <0;	
//  if (tgt_pos_ok && tgt_neg_ok && z_pos > -690 && z_neg > -690) return 0;
//  
//  //
//  //Dump_Like:- 1
//  //
//  bool d_pos_ok = pos_chisq_t >=0. && pos_chisq_d >=0. && -(pos_chi_td)<0.;
//  bool d_neg_ok = neg_chisq_t >=0. && neg_chisq_d >=0. && -(neg_chi_td)<0.;
//  if (d_pos_ok && d_neg_ok) return 1;
//  
//  //
//  //Upstream_Like:- 2
//  //
//  bool us_pos_ok = pos_chisq_t >=0. && pos_chisq_us >=0. && -(pos_chi_tus)<0.;
//  bool us_neg_ok = neg_chisq_t >=0. && neg_chisq_us >=0. && -(neg_chi_tus)<0.;
//  if (us_pos_ok && us_neg_ok) return 2;
//  
//  return -999;
//}

/**
 * Dimuon selection being used by Liliet, confirmed on 2025-08-18.
 *   z_track > -600 cm
 *   |y_st1| > 3 cm
 *   chi2_tgt > 0
 *   chi2_dump - chi2_tgt > 0
 *   chi2_ups - chi2_tgt > 0
 *   py_st1_pos * py_st1_neg < 0
 *   x1_st1_pos < 25 cm
 *   x1_st1_neg < 25 cm
 */
bool SelectDimuon(SRecDimuon* dim, SRecTrack* trk_pos, SRecTrack* trk_neg)
{
  //int road_pos = trk_pos->getTriggerRoad();
  //int road_neg = trk_neg->getTriggerRoad();
  //bool pos_top = rs.PosTop()->FindRoad(road_pos);
  //bool pos_bot = rs.PosBot()->FindRoad(road_pos);
  //bool neg_top = rs.NegTop()->FindRoad(road_neg);
  //bool neg_bot = rs.NegBot()->FindRoad(road_neg);
  //if (!(pos_top && neg_bot) && !(pos_bot && neg_top)) return false;
  
  double z_pos = trk_pos->get_pos_vtx().Z();
  double z_neg = trk_neg->get_pos_vtx().Z();
  if (z_pos < -600 || z_neg < -600) return false;
  //if (fabs(z1-z2)>200) return false;
  
  if (fabs(trk_pos->get_pos_st1().Y()) < 3 ||
      fabs(trk_neg->get_pos_st1().Y()) < 3   ) return false;

  double pos_chisq_t  = trk_pos->getChisqTarget();
  double pos_chisq_d  = trk_pos->getChisqDump();
  double pos_chisq_us = trk_pos->get_chsiq_upstream();
  double neg_chisq_t  = trk_neg->getChisqTarget();
  double neg_chisq_d  = trk_neg->getChisqDump();
  double neg_chisq_us = trk_neg->get_chsiq_upstream();
  if (pos_chisq_t < 0 || pos_chisq_t < 0) return false;
  if (pos_chisq_d - pos_chisq_t < 0 || pos_chisq_us - pos_chisq_t < 0 ||
      neg_chisq_d - neg_chisq_t < 0 || neg_chisq_us - neg_chisq_t < 0   ) return false;

  if (trk_pos->get_mom_st1().Y() * trk_neg->get_mom_st1().Y() > 0) return false;

  //if (trk_pos->get_pos_st1().X() > 25 || trk_neg->get_pos_st1().X() > 25) return false;

  return true;
}

int GetSpin(const int run_id)
{
  static map<int, int> list_spin; // run_id -> spin
  if (list_spin.size() == 0) {
    ifstream ifs("list_run_spin.txt");
    int run, spin;
    while (ifs >> run >> spin) list_spin[run] = spin;
    ifs.close();
    if (list_spin.size() == 0) {
      cout << "GetSpin():  Failed at reading the list.  Abort." << endl;
      exit(1);
    }
  }
  return list_spin[run_id];
}
