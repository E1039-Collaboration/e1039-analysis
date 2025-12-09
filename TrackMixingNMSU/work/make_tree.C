R__LOAD_LIBRARY(TrackMixingNMSU)
using namespace std;

UtilTrigger::TrigRoadset rs;

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
  int D1;
  double z;
  double px, py, pz;
  double mass;
  double trk_sep;
  TTree* tr_org = new TTree("tree_org", "");
  tr_org->Branch("run_id" , &run_id);
  tr_org->Branch("spin"   , &spin);
  tr_org->Branch("D1"     , &D1);
  tr_org->Branch("z"      , &z );
  tr_org->Branch("px"     , &px);
  tr_org->Branch("py"     , &py);
  tr_org->Branch("pz"     , &pz);
  tr_org->Branch("mass"   , &mass);
  tr_org->Branch("trk_sep", &trk_sep);
  TTree *tr_mix = new TTree("tree_mix", "");
  tr_mix->Branch("run_id" , &run_id);
  tr_mix->Branch("spin"   , &spin);
  tr_mix->Branch("D1"     , &D1);
  tr_mix->Branch("z"      , &z );
  tr_mix->Branch("px"     , &px);
  tr_mix->Branch("py"     , &py);
  tr_mix->Branch("pz"     , &pz);
  tr_mix->Branch("mass"   , &mass);
  tr_mix->Branch("trk_sep", &trk_sep);

  TH1* h1_D1_org    = new TH1D("h1_D1_org"  , ";D1;N of events", 600, -0.5, 599.5);
  TH1* h1_D1_mix    = new TH1D("h1_D1_mix"  , ";D1;N of events", 600, -0.5, 599.5);
  TH1* h1_dD1_mix   = new TH1D("h1_dD1_mix" , ";D1^{#mu+} #minus D1^{#mu#minus};N of events", 200, -99.5, 100.5);
  TH1* h1_dD2_mix   = new TH1D("h1_dD2_mix" , ";D2^{#mu+} #minus D2^{#mu#minus};N of events", 200, -99.5, 100.5);
  TH1* h1_dD3p_mix  = new TH1D("h1_dD3p_mix", ";D3p^{#mu+} #minus D3p^{#mu#minus};N of events", 200, -99.5, 100.5);
  TH1* h1_dD3m_mix  = new TH1D("h1_dD3m_mix", ";D3m^{#mu+} #minus D3m^{#mu#minus};N of events", 200, -99.5, 100.5);
  TH1* h1_n_dim_org = new TH1D("h1_n_dim_org", ";N of good dimuons;N of events", 5, -0.5, 4.5);
  TH1* h1_n_dim_mix = new TH1D("h1_n_dim_mix", ";N of good dimuons;N of events", 5, -0.5, 4.5);
  
  unsigned int n_run = list_run.size();
  for (unsigned int i_run = 0; i_run < n_run; i_run++) {
    //if (i_run == 1) break; // for debug
    run_id = list_run[i_run];
    ostringstream oss;	
    //oss << "scratch_1107/run_" << setfill('0') << setw(6) << run_id << "/out/output.root";
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
    int b_occuD1;
    int b_plus_occuD1 , b_minus_occuD1;
    int b_plus_occuD2 , b_minus_occuD2;
    int b_plus_occuD3p, b_minus_occuD3p;
    int b_plus_occuD3m, b_minus_occuD3m;
    _sorted->SetBranchAddress("occuD1", &b_occuD1);
    _sorted->SetBranchAddress("recEvent", &sorted_event);
    
    _mixed->SetBranchAddress( "plus_occuD1" , & b_plus_occuD1 );
    _mixed->SetBranchAddress("minus_occuD1" , &b_minus_occuD1 );
    _mixed->SetBranchAddress( "plus_occuD2" , & b_plus_occuD2 );
    _mixed->SetBranchAddress("minus_occuD2" , &b_minus_occuD2 );
    _mixed->SetBranchAddress( "plus_occuD3p", & b_plus_occuD3p);
    _mixed->SetBranchAddress("minus_occuD3p", &b_minus_occuD3p);
    _mixed->SetBranchAddress( "plus_occuD3m", & b_plus_occuD3m);
    _mixed->SetBranchAddress("minus_occuD3m", &b_minus_occuD3m);
    _mixed->SetBranchAddress("recEvent", &mixed_event);

    spin = GetSpin(run_id);
    
    int n_sorted = _sorted ->GetEntries();
    for (int i= 0; i < n_sorted; i++){
      _sorted->GetEntry(i);
      D1 = b_occuD1;
      h1_D1_org->Fill(b_occuD1);
      int n_dim_org_ok = 0;
      for (int n_dims=0; n_dims <sorted_event->getNDimuons(); n_dims++){
        SRecDimuon dim = sorted_event->getDimuon(n_dims);
        SRecTrack* trk_pos = &(sorted_event->getTrack(dim.get_track_id_pos()));
        SRecTrack* trk_neg = &(sorted_event->getTrack(dim.get_track_id_neg()));

        if (! SelectDimuon(&dim, trk_pos, trk_neg)) continue;
        TLorentzVector mom = dim.p_pos_target + dim.p_neg_target;
        z  = dim.get_pos().Z();
        px = mom.X();
        py = mom.Y();
        pz = mom.Z();
        dim.calcVariables(1); //1 : re-fit to target, 2: re-fit to dump
        mass = dim.get_mass();
        trk_sep = trk_pos->get_pos_vtx().Z() - trk_neg->get_pos_vtx().Z();
        tr_org->Fill();
        n_dim_org_ok++;
      }
      h1_n_dim_org->Fill(n_dim_org_ok);
    }

    int n_mixed = _mixed ->GetEntries();
    for (int j= 0; j < n_mixed; j++){
      _mixed->GetEntry(j);
      D1 = (b_plus_occuD1  + b_minus_occuD1 ) / 2;
      h1_D1_mix  ->Fill((b_plus_occuD1  + b_minus_occuD1 ) / 2);
      h1_dD1_mix ->Fill( b_plus_occuD1  - b_minus_occuD1      );
      h1_dD2_mix ->Fill( b_plus_occuD2  - b_minus_occuD2      );
      h1_dD3p_mix->Fill( b_plus_occuD3p - b_minus_occuD3p     );
      h1_dD3m_mix->Fill( b_plus_occuD3m - b_minus_occuD3m     );

      int n_dim_mix_ok = 0;
      for (int n_dims=0; n_dims <mixed_event->getNDimuons(); n_dims++){
        SRecDimuon dim = mixed_event->getDimuon(n_dims);
        SRecTrack* trk_pos = &(mixed_event->getTrack(dim.get_track_id_pos()));
        SRecTrack* trk_neg = &(mixed_event->getTrack(dim.get_track_id_neg()));

        if (! SelectDimuon(&dim, trk_pos, trk_neg)) continue;
        TLorentzVector mom = dim.p_pos_target + dim.p_neg_target;
        z  = dim.get_pos().Z();
        px = mom.X();
        py = mom.Y();
        pz = mom.Z();
        dim.calcVariables(1);
        mass = dim.get_mass();
        trk_sep = trk_pos->get_pos_vtx().Z() - trk_neg->get_pos_vtx().Z();
        tr_mix->Fill();
        n_dim_mix_ok++;
      }
      h1_n_dim_mix->Fill(n_dim_mix_ok);
    }
  }
  
  f_out->cd();
  f_out->Write();
  f_out->Close();
  exit(0);
}

bool SelectDimuon(SRecDimuon* dim, SRecTrack* trk_pos, SRecTrack* trk_neg)
{
  //int road_pos = trk_pos->getTriggerRoad();
  //int road_neg = trk_neg->getTriggerRoad();
  //bool pos_top = rs.PosTop()->FindRoad(road_pos);
  //bool pos_bot = rs.PosBot()->FindRoad(road_pos);
  //bool neg_top = rs.NegTop()->FindRoad(road_neg);
  //bool neg_bot = rs.NegBot()->FindRoad(road_neg);
  //if (!(pos_top && neg_bot) && !(pos_bot && neg_top)) return false;
  
  double z_vtx = dim->get_pos().Z();
  double z_pos = trk_pos->get_pos_vtx().Z();
  double z_neg = trk_neg->get_pos_vtx().Z();
  if (z_vtx < -690 || z_pos < -690 || z_neg < -690) return false;
  if (fabs(z_pos-z_neg)>200) return false;

  double pz_pos = trk_pos->get_mom_vtx().Z();
  double pz_neg = trk_neg->get_mom_vtx().Z();
  if (pz_pos < 5 || pz_neg < 5) return false;
  
  if (fabs(trk_pos->get_pos_st1().Y()) < 3 ||
      fabs(trk_neg->get_pos_st1().Y()) < 3   ) return false;

  double pos_chisq_t  = trk_pos->getChisqTarget();
  double pos_chisq_d  = trk_pos->getChisqDump();
  double pos_chisq_us = trk_pos->get_chsiq_upstream();
  double neg_chisq_t  = trk_neg->getChisqTarget();
  double neg_chisq_d  = trk_neg->getChisqDump();
  double neg_chisq_us = trk_neg->get_chsiq_upstream();
  if (pos_chisq_t < 0 || pos_chisq_d < 0 || pos_chisq_us < 0 ||
      neg_chisq_t < 0 || neg_chisq_d < 0 || neg_chisq_us < 0   ) return false;
  if (pos_chisq_d - pos_chisq_t < 0 || pos_chisq_us - pos_chisq_t < 0 ||
      neg_chisq_d - neg_chisq_t < 0 || neg_chisq_us - neg_chisq_t < 0   ) return false;

  //if (trk_pos->get_mom_st1().Y() * trk_neg->get_mom_st1().Y() > 0) return false;
  //if (trk_pos->get_pos_st1().X() > 25 || trk_neg->get_pos_st1().X() > 25) return false;

  return true;
}

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
bool SelectDimuonV1(SRecDimuon* dim, SRecTrack* trk_pos, SRecTrack* trk_neg)
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
  if (pos_chisq_t < 0 || neg_chisq_t < 0) return false;
  if (pos_chisq_d - pos_chisq_t < 0 || pos_chisq_us - pos_chisq_t < 0 ||
      neg_chisq_d - neg_chisq_t < 0 || neg_chisq_us - neg_chisq_t < 0   ) return false;

  if (trk_pos->get_mom_st1().Y() * trk_neg->get_mom_st1().Y() > 0) return false;

  if (trk_pos->get_pos_st1().X() > 25 || trk_neg->get_pos_st1().X() > 25) return false;

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
