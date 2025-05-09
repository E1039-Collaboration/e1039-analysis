#include <fstream>
#include <iomanip>
#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQHitVector.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQTrackVector.h>
#include <interface_main/SQDimuonVector.h>
#include <ktracker/SRecEvent.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <geom_svc/GeomSvc.h>
//#include <UtilAna/UtilHist.h>
#include "AnaDimuon.h"
using namespace std;

AnaDimuon::AnaDimuon(const std::string& name)
  : SubsysReco  (name)
  , m_sq_evt    (0)
  , m_sq_hit_vec(0)
  , m_sq_trk_vec(0)
  , m_sq_dim_vec(0)
  , m_file_name ("output_PM.root")
  , m_file      (0)
  , m_tree      (0)
{
  ;
}

AnaDimuon::~AnaDimuon()
{
  ;
}

int AnaDimuon::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaDimuon::InitRun(PHCompositeNode* topNode)
{
  //GeomSvc* geom = GeomSvc::instance();

  m_sq_evt     = findNode::getClass<SQEvent       >(topNode, "SQEvent");
  m_sq_hit_vec = findNode::getClass<SQHitVector   >(topNode, "SQHitVector");
  m_sq_trk_vec = findNode::getClass<SQTrackVector >(topNode, "SQRecTrackVector");
  m_sq_dim_vec = findNode::getClass<SQDimuonVector>(topNode, "SQRecDimuonVector_PM");
  if (!m_sq_evt || !m_sq_hit_vec || !m_sq_trk_vec || !m_sq_dim_vec) return Fun4AllReturnCodes::ABORTEVENT;

  m_file = new TFile(m_file_name.c_str(), "RECREATE");
  m_tree = new TTree("tree", "Created by AnaDimuon");
  m_tree->Branch("event"      , &m_evt);
  m_tree->Branch("dimuon_list", &m_dim_list);

  SQRun* sq_run = findNode::getClass<SQRun>(topNode, "SQRun");
  if (!sq_run) return Fun4AllReturnCodes::ABORTEVENT;
  int LBtop = sq_run->get_v1495_id(2);
  int LBbot = sq_run->get_v1495_id(3);
  int ret = m_rs.LoadConfig(LBtop, LBbot);
  if (ret != 0) {
    cout << "!!WARNING!!  OnlMonTrigEP::InitRunOnlMon():  roadset.LoadConfig returned " << ret << ".\n";
  }
  cout <<"Roadset " << m_rs.str(1) << endl;

  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaDimuon::process_event(PHCompositeNode* topNode)
{
  //if (! m_sq_evt->get_trigger(SQEvent::MATRIX1)) {
  //  return Fun4AllReturnCodes::EVENT_OK;
  //}

  m_evt.run_id    = m_sq_evt->get_run_id();
  m_evt.spill_id  = m_sq_evt->get_spill_id();
  m_evt.event_id  = m_sq_evt->get_event_id();
  m_evt.fpga_bits = (m_sq_evt->get_trigger() >> SQEvent::MATRIX1) & 0x1f;
  m_evt.nim_bits  = (m_sq_evt->get_trigger() >> SQEvent::NIM1   ) & 0x1f;

  m_evt.D1 = m_evt.D2 = m_evt.D3p = m_evt.D3m = 0;
  for (SQHitVector::Iter it = m_sq_hit_vec->begin(); it != m_sq_hit_vec->end(); it++) {
    SQHit* hit = *it;
    int det_id = hit->get_detector_id();
    if      ( 0 < det_id && det_id <=  6) m_evt.D1++;
    else if (12 < det_id && det_id <= 18) m_evt.D2++;
    else if (18 < det_id && det_id <= 24) m_evt.D3p++;
    else if (24 < det_id && det_id <= 30) m_evt.D3m++;
  }

  //int n_trk = m_srec->getNTracks();
  //for (int i_trk = 0; i_trk < n_trk; i_trk++) {
  //  SRecTrack strk = m_srec->getTrack(i_trk);
  //  cout << m_evt.event_id
  //       << " " << i_trk
  //       << " " << strk.get_charge()
  //       << " " << strk.get_num_hits()
  //       << " " << strk.get_chisq()
  //       << " " << strk.get_pos_vtx().X()
  //       << " " << strk.get_pos_vtx().Y()
  //       << " " << strk.get_pos_vtx().Z()
  //       << " " << strk.get_mom_vtx().X()
  //       << " " << strk.get_mom_vtx().Y()
  //       << " " << strk.get_mom_vtx().Z()
  //       << endl;
  //}
  
  m_dim_list.clear();
  for (auto it = m_sq_dim_vec->begin(); it != m_sq_dim_vec->end(); it++) {
    SRecDimuon* dim = dynamic_cast<SRecDimuon*>(*it);
    int trk_id_pos = dim->get_track_id_pos();
    int trk_id_neg = dim->get_track_id_neg();
    SRecTrack* trk_pos = dynamic_cast<SRecTrack*>(m_sq_trk_vec->at(trk_id_pos));
    SRecTrack* trk_neg = dynamic_cast<SRecTrack*>(m_sq_trk_vec->at(trk_id_neg));

    int road_pos = trk_pos->getTriggerRoad();
    int road_neg = trk_neg->getTriggerRoad();
    bool pos_top = m_rs.PosTop()->FindRoad(road_pos);
    bool pos_bot = m_rs.PosBot()->FindRoad(road_pos);
    bool neg_top = m_rs.NegTop()->FindRoad(road_neg);
    bool neg_bot = m_rs.NegBot()->FindRoad(road_neg);
    //cout << "T " << road_pos << " " << road_neg << " " << pos_top << pos_bot << neg_top << neg_bot << endl;

    DimuonData dd;
    dd.road_pos           = road_pos;
    dd.road_neg           = road_neg;
    dd.pos_top            = pos_top;
    dd.pos_bot            = pos_bot;
    dd.neg_top            = neg_top;
    dd.neg_bot            = neg_bot;
    dd.pos                = dim->get_pos();
    dd.mom                = dim->get_mom();
    dd.n_hits_pos         = trk_pos->get_num_hits();
    dd.chisq_pos          = trk_pos->get_chisq();
    dd.chisq_target_pos   = trk_pos->getChisqTarget();//get_chisq_target();
    dd.chisq_dump_pos     = trk_pos->get_chisq_dump();
    dd.chisq_upstream_pos = trk_pos->get_chsiq_upstream();
    dd.pos_pos            = trk_pos->get_pos_vtx();
    dd.mom_pos            = trk_pos->get_mom_vtx();
    dd.pos_target_pos     = trk_pos->get_pos_target();
    dd.pos_dump_pos       = trk_pos->get_pos_dump();
    dd.n_hits_neg         = trk_neg->get_num_hits();
    dd.chisq_neg          = trk_neg->get_chisq();
    dd.chisq_target_neg   = trk_neg->getChisqTarget();//get_chisq_target();
    dd.chisq_dump_neg     = trk_neg->get_chisq_dump();
    dd.chisq_upstream_neg = trk_neg->get_chsiq_upstream(); // not chisq
    dd.pos_neg            = trk_neg->get_pos_vtx();
    dd.mom_neg            = trk_neg->get_mom_vtx();
    dd.pos_target_neg     = trk_neg->get_pos_target();
    dd.pos_dump_neg       = trk_neg->get_pos_dump();
    
    //sdim.calcVariables(1); // 1 = target
    dd.mom_target = dim->p_pos_target + dim->p_neg_target; // sdim.get_mom();
    //sdim.calcVariables(2); // 2 = dump
    dd.mom_dump = dim->p_pos_dump + dim->p_neg_dump; // sdim.get_mom();
    
    m_dim_list.push_back(dd);
  }
  
  m_tree->Fill();
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaDimuon::End(PHCompositeNode* topNode)
{
  m_file->cd();
  m_file->Write();
  m_file->Close();  
  return Fun4AllReturnCodes::EVENT_OK;
}

void AnaDimuon::AnalyzeTree(TChain* tree)
{
  string dir_out = "result/PM"; //  + label;
  cout << "N of trees = " << tree->GetNtrees() << endl;
  gSystem->mkdir(dir_out.c_str(), true);
  ofstream ofs(dir_out + "/result.txt");

  TFile* file_out = new TFile((dir_out+"/result.root").c_str(), "RECREATE");
  
  TH1* h1_D1  = new TH1D("h1_D1" ,  ";D1 occupancy;N of events", 500, -0.5, 499.5);
  TH1* h1_D2  = new TH1D("h1_D2" ,  ";D2 occupancy;N of events", 300, -0.5, 299.5);
  TH1* h1_D3p = new TH1D("h1_D3p", ";D3p occupancy;N of events", 300, -0.5, 299.5);
  TH1* h1_D3m = new TH1D("h1_D3m", ";D3m occupancy;N of events", 300, -0.5, 299.5);
  
  TH1* h1_nhit_pos = new TH1D("h1_nhit_pos", "#mu^{+};N of hits/track;", 6, 12.5, 18.5);
  TH1* h1_chi2_pos = new TH1D("h1_chi2_pos", "#mu^{+};Track #chi^{2};", 100, 0, 2);
  TH1* h1_z_pos    = new TH1D("h1_z_pos"   , "#mu^{+};Track z (cm);"  , 100, -700, 300);
  TH1* h1_pz_pos   = new TH1D("h1_pz_pos"  , "#mu^{+};Track p_{z} (GeV);", 100, 0, 100);
  //TH1* h1_x_t_pos  = new TH1D("h1_x_t_pos", "#mu^{+};Track x (cm) @ Target;", 100, -50, 50);
  //TH1* h1_y_t_pos  = new TH1D("h1_y_t_pos", "#mu^{+};Track y (cm) @ Target;", 100, -25, 25);
  //TH1* h1_x_d_pos  = new TH1D("h1_x_d_pos", "#mu^{+};Track x (cm) @ Dump;"  , 100, -10, 10);
  //TH1* h1_y_d_pos  = new TH1D("h1_y_d_pos", "#mu^{+};Track y (cm) @ Dump;"  , 100, -10, 10);
  
  TH1* h1_chi2_tgt_pos = new TH1D("h1_chi2_tgt_pos", "#mu^{+};Track #chi^{2} at target;"  , 100, 0, 10);
  TH1* h1_chi2_dum_pos = new TH1D("h1_chi2_dum_pos", "#mu^{+};Track #chi^{2} at dump;"    , 100, 0, 10);
  TH1* h1_chi2_ups_pos = new TH1D("h1_chi2_ups_pos", "#mu^{+};Track #chi^{2} at upstream;", 100, 0, 10);
  TH1* h1_chi2_tmd_pos = new TH1D("h1_chi2_tmd_pos", "#mu^{+};#chi^{2}_{Target} - #chi^{2}_{Dump};"    , 100, -10, 10);
  TH1* h1_chi2_tmu_pos = new TH1D("h1_chi2_tmu_pos", "#mu^{+};#chi^{2}_{Target} - #chi^{2}_{Upstream};", 100, -10, 10);
  
  TH1* h1_nhit_neg = new TH1D("h1_nhit_neg", "#mu^{-};N of hits/track;", 6, 12.5, 18.5);
  TH1* h1_chi2_neg = new TH1D("h1_chi2_neg", "#mu^{-};Track #chi^{2};", 100, 0, 2);
  TH1* h1_z_neg    = new TH1D("h1_z_neg"   , "#mu^{-};Track z (cm);", 100, -700, 300);
  TH1* h1_pz_neg   = new TH1D("h1_pz_neg"  , "#mu^{-};Track p_{z} (GeV);", 100, 0, 100);
  //TH1* h1_x_t_neg  = new TH1D("h1_x_t_neg", "#mu^{-};Track x (cm) @ Target;", 100, -50, 50);
  //TH1* h1_y_t_neg  = new TH1D("h1_y_t_neg", "#mu^{-};Track y (cm) @ Target;", 100, -25, 25);
  //TH1* h1_x_d_neg  = new TH1D("h1_x_d_neg", "#mu^{-};Track x (cm) @ Dump;"  , 100, -10, 10);
  //TH1* h1_y_d_neg  = new TH1D("h1_y_d_neg", "#mu^{-};Track y (cm) @ Dump;"  , 100, -10, 10);

  TH1* h1_chi2_tgt_neg = new TH1D("h1_chi2_tgt_neg", "#mu^{-};Track #chi^{2} at target;"  , 100, 0, 10);
  TH1* h1_chi2_dum_neg = new TH1D("h1_chi2_dum_neg", "#mu^{-};Track #chi^{2} at dump;"    , 100, 0, 10);
  TH1* h1_chi2_ups_neg = new TH1D("h1_chi2_ups_neg", "#mu^{-};Track #chi^{2} at upstream;", 100, 0, 10);
  TH1* h1_chi2_tmd_neg = new TH1D("h1_chi2_tmd_neg", "#mu^{-};#chi^{2}_{Target} - #chi^{2}_{Dump};"    , 100, -10, 10);
  TH1* h1_chi2_tmu_neg = new TH1D("h1_chi2_tmu_neg", "#mu^{-};#chi^{2}_{Target} - #chi^{2}_{Upstream};", 100, -10, 10);
  
  TH1* h1_dx  = new TH1D("h1_dx" , ";Dimuon x (cm);", 100, -1, 1);
  TH1* h1_dy  = new TH1D("h1_dy" , ";Dimuon y (cm);", 100, -1, 1);
  TH1* h1_dz  = new TH1D("h1_dz" , ";Dimuon z (cm);", 100, -700, 300);
  TH1* h1_dpx = new TH1D("h1_dpx", ";Dimuon p_{x} (GeV);", 100, -5, 5);
  TH1* h1_dpy = new TH1D("h1_dpy", ";Dimuon p_{y} (GeV);", 100, -5, 5);
  TH1* h1_dpz = new TH1D("h1_dpz", ";Dimuon p_{z} (GeV);", 100, 30, 130);
  TH1* h1_m   = new TH1D("h1_m"  , ";Dimuon mass (GeV);", 100, 0, 10);
  TH1* h1_trk_sep = new TH1D("h1_trk_sep", ";Track separation: z_{#mu +} - z_{#mu -} (cm);", 100, -500, 500);

  TH1* h1_dz_sel  = new TH1D("h1_dz_sel" , ";Dimuon z (cm);", 100, -700, 300);
  TH1* h1_dpz_sel = new TH1D("h1_dpz_sel", ";Dimuon p_{z} (GeV);", 100, 30, 130);
  TH1* h1_m_sel   = new TH1D("h1_m_sel"  , ";Dimuon mass (GeV);", 100, 0, 10);

  TH1* h1_dz_tgt  = new TH1D("h1_dz_tgt" , ";Dimuon z (cm);"     , 100, -700, 300);
  TH1* h1_dpz_tgt = new TH1D("h1_dpz_tgt", ";Dimuon p_{z} (GeV);", 100, 30, 130);
  TH1* h1_m_tgt   = new TH1D("h1_m_tgt"  , ";Dimuon mass (GeV);" , 100, 0, 10);

  //GeomSvc* geom = GeomSvc::instance();
  ostringstream oss;
  
  EventData* evt = 0;
  DimuonList* dim_list = 0;
  tree->SetBranchAddress("event"      , &evt);
  tree->SetBranchAddress("dimuon_list", &dim_list);

  int n_ent = tree->GetEntries();
  cout << "N of entries = " << n_ent << endl;
  for (int i_ent = 0; i_ent < n_ent; i_ent++) {
    if ((i_ent+1) % (n_ent/10) == 0) cout << "  " << 10*(i_ent+1)/(n_ent/10) << "%" << flush;
    tree->GetEntry(i_ent);
    //ofs << evt->run_id << " " << evt->spill_id << " " << evt->event_id << " " << evt->D1 << " " << evt->D2 << " " << evt->D3p << " " << evt->D3m << endl;

    //if (evt->run_id != 6155 || evt->spill_id != 1941910) continue;
    if (! (evt->fpga_bits & 0x1)) continue;
    //if (! (evt->nim_bits & 0x4)) continue;

    h1_D1 ->Fill(evt->D1 );
    h1_D2 ->Fill(evt->D2 );
    h1_D3p->Fill(evt->D3p);
    h1_D3m->Fill(evt->D3m);
    //if (evt->D1 > 120 || evt->D2 > 60 || evt->D3p > 50 || evt->D3m > 50) continue;
    
    for (auto it = dim_list->begin(); it != dim_list->end(); it++) {
      DimuonData* dd = &(*it);

      double trk_sep      = dd->pos_pos.Z() - dd->pos_neg.Z();
      double chi2_tgt_pos = dd->chisq_target_pos;
      double chi2_dum_pos = dd->chisq_dump_pos;
      double chi2_ups_pos = dd->chisq_upstream_pos;
      double chi2_tgt_neg = dd->chisq_target_neg;
      double chi2_dum_neg = dd->chisq_dump_neg;
      double chi2_ups_neg = dd->chisq_upstream_neg;
      
      ofs << evt->run_id << " " << evt->spill_id << " " << evt->event_id << " "
          << evt->D1 << " " << evt->D2 << " " << evt->D3p << " " << evt->D3m << " "
          << dd->pos.Z() << " " << dd->mom.M() << endl;
      //ofs << chi2_tgt_pos << " " << chi2_dum_pos << " " << chi2_ups_pos << " " << chi2_tgt_neg << " " << chi2_dum_neg << " " << chi2_ups_neg << endl;

      //if (dd->n_hits_pos < 14 || dd->n_hits_neg < 14) continue;
      
      h1_nhit_pos->Fill(dd->n_hits_pos);
      h1_chi2_pos->Fill(dd->chisq_pos);
      h1_z_pos   ->Fill(dd->pos_pos.Z());
      h1_pz_pos  ->Fill(dd->mom_pos.Z());

      h1_nhit_neg->Fill(dd->n_hits_neg);
      h1_chi2_neg->Fill(dd->chisq_neg);
      h1_z_neg   ->Fill(dd->pos_neg.Z());
      h1_pz_neg  ->Fill(dd->mom_neg.Z());

      if (dd->pos_pos.Z() < -690 || dd->pos_neg.Z() < -690) continue;
      //if (dd->n_hits_pos < 15 || dd->n_hits_neg < 15) continue;
      //if (fabs(trk_sep) > 200) continue;
      
      //bool top_bot = dd->pos_top && dd->neg_bot;
      //bool bot_top = dd->pos_bot && dd->neg_top;
      //if (!top_bot && !bot_top) continue;
      
      h1_chi2_tgt_pos->Fill(chi2_tgt_pos);
      h1_chi2_dum_pos->Fill(chi2_dum_pos);
      h1_chi2_ups_pos->Fill(chi2_ups_pos);
      h1_chi2_tmd_pos->Fill(chi2_tgt_pos - chi2_dum_pos);
      h1_chi2_tmu_pos->Fill(chi2_tgt_pos - chi2_ups_pos);
      //double x_t_pos = dd->pos_target_pos.X();
      //double y_t_pos = dd->pos_target_pos.Y();
      //double x_d_pos = dd->pos_dump_pos.X();
      //double y_d_pos = dd->pos_dump_pos.Y();      
      //h1_x_t_pos->Fill(x_t_pos);
      //h1_y_t_pos->Fill(y_t_pos);
      //h1_x_d_pos->Fill(x_d_pos);
      //h1_y_d_pos->Fill(y_d_pos);

      h1_chi2_tgt_neg->Fill(chi2_tgt_neg);
      h1_chi2_dum_neg->Fill(chi2_dum_neg);
      h1_chi2_ups_neg->Fill(chi2_ups_neg);
      h1_chi2_tmd_neg->Fill(chi2_tgt_neg - chi2_dum_neg);
      h1_chi2_tmu_neg->Fill(chi2_tgt_neg - chi2_ups_neg);
      //double x_t_neg = dd->pos_target_neg.X();
      //double y_t_neg = dd->pos_target_neg.Y();
      //double x_d_neg = dd->pos_dump_neg.X();
      //double y_d_neg = dd->pos_dump_neg.Y();      
      //h1_x_t_neg->Fill(x_t_neg);
      //h1_y_t_neg->Fill(y_t_neg);
      //h1_x_d_neg->Fill(x_d_neg);
      //h1_y_d_neg->Fill(y_d_neg);
      
      h1_dx     ->Fill(dd->pos.X());
      h1_dy     ->Fill(dd->pos.Y());
      h1_dz     ->Fill(dd->pos.Z());
      h1_dpx    ->Fill(dd->mom.X());
      h1_dpy    ->Fill(dd->mom.Y());
      h1_dpz    ->Fill(dd->mom.Z());
      h1_m      ->Fill(dd->mom.M());
      h1_trk_sep->Fill(trk_sep);
      
      if (chi2_tgt_pos < 0 || chi2_dum_pos < 0 || chi2_ups_pos < 0 ||
          chi2_tgt_pos - chi2_dum_pos > 0 || chi2_tgt_pos - chi2_ups_pos > 0) continue;
      if (chi2_tgt_neg < 0 || chi2_dum_neg < 0 || chi2_ups_neg < 0 ||
          chi2_tgt_neg - chi2_dum_neg > 0 || chi2_tgt_neg - chi2_ups_neg > 0) continue;

      //double r_t_pos = sqrt(x_t_pos*x_t_pos + y_t_pos*y_t_pos);
      //double r_d_pos = sqrt(x_d_pos*x_d_pos + y_d_pos*y_d_pos);
      //double r_t_neg = sqrt(x_t_neg*x_t_neg + y_t_neg*y_t_neg);
      //double r_d_neg = sqrt(x_d_neg*x_d_neg + y_d_neg*y_d_neg);
      //if (r_t_pos >= r_d_pos || r_t_neg >= r_d_neg) continue;
      
      h1_dz_sel ->Fill(dd->pos.Z());
      h1_dpz_sel->Fill(dd->mom.Z());
      h1_m_sel  ->Fill(dd->mom.M());
      
      h1_dz_tgt ->Fill(dd->pos.Z());
      h1_dpz_tgt->Fill(dd->mom_target.Z());
      h1_m_tgt  ->Fill(dd->mom_target.M());
    }
  }
  
  TCanvas* c1 = new TCanvas("c1", "");
  c1->SetGrid();
  //c1->SetLogy(true);

  h1_D1 ->Draw();  c1->SaveAs((dir_out+"/h1_D1.png").c_str());
  h1_D2 ->Draw();  c1->SaveAs((dir_out+"/h1_D2.png").c_str());
  h1_D3p->Draw();  c1->SaveAs((dir_out+"/h1_D3p.png").c_str());
  h1_D3m->Draw();  c1->SaveAs((dir_out+"/h1_D3m.png").c_str());
  
  h1_nhit_pos->Draw();  c1->SaveAs((dir_out+"/h1_nhit_pos.png").c_str());
  h1_chi2_pos->Draw();  c1->SaveAs((dir_out+"/h1_chi2_pos.png").c_str());
  h1_z_pos   ->Draw();  c1->SaveAs((dir_out+"/h1_z_pos.png").c_str());
  h1_pz_pos  ->Draw();  c1->SaveAs((dir_out+"/h1_pz_pos.png").c_str());
  //h1_x_t_pos ->Draw();  c1->SaveAs(dir_out+"/h1_x_t_pos.png");  
  //h1_y_t_pos ->Draw();  c1->SaveAs(dir_out+"/h1_y_t_pos.png");  
  //h1_x_d_pos ->Draw();  c1->SaveAs(dir_out+"/h1_x_d_pos.png");  
  //h1_y_d_pos ->Draw();  c1->SaveAs(dir_out+"/h1_y_d_pos.png");  

  h1_chi2_tgt_pos->Draw();  c1->SaveAs((dir_out+"/h1_chi2_tgt_pos.png").c_str());
  h1_chi2_dum_pos->Draw();  c1->SaveAs((dir_out+"/h1_chi2_dum_pos.png").c_str());
  h1_chi2_ups_pos->Draw();  c1->SaveAs((dir_out+"/h1_chi2_ups_pos.png").c_str());
  h1_chi2_tmd_pos->Draw();  c1->SaveAs((dir_out+"/h1_chi2_tmd_pos.png").c_str());
  h1_chi2_tmu_pos->Draw();  c1->SaveAs((dir_out+"/h1_chi2_tmu_pos.png").c_str());
  
  h1_nhit_neg->Draw();  c1->SaveAs((dir_out+"/h1_nhit_neg.png").c_str());
  h1_chi2_neg->Draw();  c1->SaveAs((dir_out+"/h1_chi2_neg.png").c_str());
  h1_z_neg   ->Draw();  c1->SaveAs((dir_out+"/h1_z_neg.png").c_str());
  h1_pz_neg  ->Draw();  c1->SaveAs((dir_out+"/h1_pz_neg.png").c_str());
  //h1_x_t_neg ->Draw();  c1->SaveAs(dir_out+"/h1_x_t_neg.png");  
  //h1_y_t_neg ->Draw();  c1->SaveAs(dir_out+"/h1_y_t_neg.png");  
  //h1_x_d_neg ->Draw();  c1->SaveAs(dir_out+"/h1_x_d_neg.png");  
  //h1_y_d_neg ->Draw();  c1->SaveAs(dir_out+"/h1_y_d_neg.png");  

  h1_chi2_tgt_neg->Draw();  c1->SaveAs((dir_out+"/h1_chi2_tgt_neg.png").c_str());
  h1_chi2_dum_neg->Draw();  c1->SaveAs((dir_out+"/h1_chi2_dum_neg.png").c_str());
  h1_chi2_ups_neg->Draw();  c1->SaveAs((dir_out+"/h1_chi2_ups_neg.png").c_str());
  h1_chi2_tmd_neg->Draw();  c1->SaveAs((dir_out+"/h1_chi2_tmd_neg.png").c_str());
  h1_chi2_tmu_neg->Draw();  c1->SaveAs((dir_out+"/h1_chi2_tmu_neg.png").c_str());

  h1_dx     ->Draw();  c1->SaveAs((dir_out+"/h1_dx.png"     ).c_str());
  h1_dy     ->Draw();  c1->SaveAs((dir_out+"/h1_dy.png"     ).c_str());
  h1_dz     ->Draw();  c1->SaveAs((dir_out+"/h1_dz.png"     ).c_str());
  h1_dpx    ->Draw();  c1->SaveAs((dir_out+"/h1_dpx.png"    ).c_str());
  h1_dpy    ->Draw();  c1->SaveAs((dir_out+"/h1_dpy.png"    ).c_str());
  h1_dpz    ->Draw();  c1->SaveAs((dir_out+"/h1_dpz.png"    ).c_str());
  h1_m      ->Draw();  c1->SaveAs((dir_out+"/h1_m.png"      ).c_str());
  h1_trk_sep->Draw();  c1->SaveAs((dir_out+"/h1_trk_sep.png").c_str());

  //c1->SetLogy(true);

  h1_dz_sel->SetLineColor(kRed);
  h1_dz_sel->SetLineWidth(2);
  h1_dz_sel->Draw();
  c1->SaveAs((dir_out+"/h1_dz_sel.png").c_str());

  h1_dpz_sel->SetLineColor(kRed);
  h1_dpz_sel->SetLineWidth(2);
  h1_dpz_sel->Draw();
  c1->SaveAs((dir_out+"/h1_dpz_sel.png").c_str());

  h1_m_sel ->SetLineColor(kRed);
  h1_m_sel ->SetLineWidth(2);
  h1_m_sel->Draw();
  c1->SaveAs((dir_out+"/h1_m_sel.png").c_str());

  h1_dz_tgt->SetLineColor(kBlue);
  h1_dz_tgt->SetLineWidth(2);
  h1_dz_tgt->Draw();
  c1->SaveAs((dir_out+"/h1_dz_tgt.png").c_str());

  h1_dpz_tgt->SetLineColor(kBlue);
  h1_dpz_tgt->SetLineWidth(2);
  h1_dpz_tgt->Draw();
  c1->SaveAs((dir_out+"/h1_dpz_tgt.png").c_str());

  h1_m_tgt ->SetLineColor(kBlue);
  h1_m_tgt ->SetLineWidth(2);
  h1_m_tgt->Draw();
  c1->SaveAs((dir_out+"/h1_m_tgt.png").c_str());
  
  delete c1;

  ofs.close();
  file_out->Write();
  file_out->Close();
}
