#include <fstream>
#include <iomanip>
#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TH1D.h>
#include <TH2D.h>
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
#include <UtilAna/UtilSQHit.h>
#include <UtilAna/UtilTrigger.h>
#include "AnaTriggerAndTrack.h"
using namespace std;

AnaTriggerAndTrack::AnaTriggerAndTrack(const std::string& name)
  : SubsysReco  (name)
  , m_sq_evt    (0)
  , m_sq_hit_vec(0)
  , m_sq_trk_vec(0)
  , m_sq_dim_vec(0)
  , m_file_name ("output_trig.root")
  , m_file      (0)
  , m_tree      (0)
{
  ;
}

AnaTriggerAndTrack::~AnaTriggerAndTrack()
{
  ;
}

int AnaTriggerAndTrack::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaTriggerAndTrack::InitRun(PHCompositeNode* topNode)
{
  //GeomSvc* geom = GeomSvc::instance();

  m_sq_evt     = findNode::getClass<SQEvent       >(topNode, "SQEvent");
  m_sq_hit_vec = findNode::getClass<SQHitVector   >(topNode, "SQHitVector");
  m_sq_trk_vec = findNode::getClass<SQTrackVector >(topNode, "SQRecTrackVector");
  m_sq_dim_vec = findNode::getClass<SQDimuonVector>(topNode, "SQRecDimuonVector_PM");
  if (!m_sq_evt || !m_sq_hit_vec || !m_sq_trk_vec || !m_sq_dim_vec) return Fun4AllReturnCodes::ABORTEVENT;

  m_file = new TFile(m_file_name.c_str(), "RECREATE");
  m_tree = new TTree("tree", "Created by AnaTriggerAndTrack");
  m_tree->Branch("event"     , &m_evt);
  m_tree->Branch("road"      , &m_road);
  m_tree->Branch("track_list", &m_trk_list);
  //m_tree->Branch("dimuon_list", &m_dim_list);

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

int AnaTriggerAndTrack::process_event(PHCompositeNode* topNode)
{
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

  const bool in_time = false;
  m_road.pos_top.clear();
  m_road.neg_top.clear();
  shared_ptr<SQHitVector> hv_h1t(UtilSQHit::FindFirstHits(m_sq_hit_vec, "H1T", in_time));
  shared_ptr<SQHitVector> hv_h2t(UtilSQHit::FindFirstHits(m_sq_hit_vec, "H2T", in_time));
  shared_ptr<SQHitVector> hv_h3t(UtilSQHit::FindFirstHits(m_sq_hit_vec, "H3T", in_time));
  shared_ptr<SQHitVector> hv_h4t(UtilSQHit::FindFirstHits(m_sq_hit_vec, "H4T", in_time));
  for (auto it1 = hv_h1t->begin(); it1 != hv_h1t->end(); it1++) {
  for (auto it2 = hv_h2t->begin(); it2 != hv_h2t->end(); it2++) {
  for (auto it3 = hv_h3t->begin(); it3 != hv_h3t->end(); it3++) {
  for (auto it4 = hv_h4t->begin(); it4 != hv_h4t->end(); it4++) {
    int road = UtilTrigger::Hodo2Road(
      (*it1)->get_element_id(), 
      (*it2)->get_element_id(), 
      (*it3)->get_element_id(), 
      (*it4)->get_element_id(), +1);
    if (m_rs.PosTop()->FindRoad(road)) m_road.pos_top.push_back(road);
    if (m_rs.NegTop()->FindRoad(road)) m_road.neg_top.push_back(road);
  }
  }
  }
  }
  m_road.pos_top.erase(std::unique(m_road.pos_top.begin(), m_road.pos_top.end()), m_road.pos_top.end());
  m_road.neg_top.erase(std::unique(m_road.neg_top.begin(), m_road.neg_top.end()), m_road.neg_top.end());
  
  m_road.pos_bot.clear();
  m_road.neg_bot.clear();
  shared_ptr<SQHitVector> hv_h1b(UtilSQHit::FindHits(m_sq_hit_vec, "H1B", in_time));
  shared_ptr<SQHitVector> hv_h2b(UtilSQHit::FindHits(m_sq_hit_vec, "H2B", in_time));
  shared_ptr<SQHitVector> hv_h3b(UtilSQHit::FindHits(m_sq_hit_vec, "H3B", in_time));
  shared_ptr<SQHitVector> hv_h4b(UtilSQHit::FindHits(m_sq_hit_vec, "H4B", in_time));
  for (auto it1 = hv_h1b->begin(); it1 != hv_h1b->end(); it1++) {
  for (auto it2 = hv_h2b->begin(); it2 != hv_h2b->end(); it2++) {
  for (auto it3 = hv_h3b->begin(); it3 != hv_h3b->end(); it3++) {
  for (auto it4 = hv_h4b->begin(); it4 != hv_h4b->end(); it4++) {
    int road = UtilTrigger::Hodo2Road(
      (*it1)->get_element_id(), 
      (*it2)->get_element_id(), 
      (*it3)->get_element_id(), 
      (*it4)->get_element_id(), -1);
    if (m_rs.PosBot()->FindRoad(road)) m_road.pos_bot.push_back(road);
    if (m_rs.NegBot()->FindRoad(road)) m_road.neg_bot.push_back(road);
  }
  }
  }
  }
  m_road.pos_bot.erase(std::unique(m_road.pos_bot.begin(), m_road.pos_bot.end()), m_road.pos_bot.end());
  m_road.neg_bot.erase(std::unique(m_road.neg_bot.begin(), m_road.neg_bot.end()), m_road.neg_bot.end());

  m_trk_list.clear();
  for (auto it = m_sq_trk_vec->begin(); it != m_sq_trk_vec->end(); it++) {
    SRecTrack* trk = dynamic_cast<SRecTrack*>(*it);
    TrackData td;
    td.charge         = trk->getCharge();
    td.road           = trk->getTriggerRoad();
    td.road_idx = -1;
    
    UtilTrigger::TrigRoads* roads = 0;
    if (td.charge > 0) {
      if (trk->get_mom_vtx().Y() > 0) roads = m_rs.PosTop();
      else                            roads = m_rs.PosBot();
    } else {
      if (trk->get_mom_vtx().Y() > 0) roads = m_rs.NegTop();
      else                            roads = m_rs.NegBot();
    }
    for (unsigned int idx = 0; idx < roads->GetNumRoads(); idx++) {
      if (roads->GetRoad(idx)->road_id == td.road) {
        td.road_idx = idx;
        break;
      }
    }
    td.n_hits         = trk->getNHits();
    td.chisq          = trk->getChisq();
    td.chisq_target   = trk->getChisqTarget();
    td.chisq_dump     = trk->getChisqDump();
    td.chisq_upstream = trk->getChisqUpstream();
    td.pos_vtx        = trk->get_pos_vtx();
    td.mom_vtx        = trk->get_mom_vtx();
    td.pos_st1        = trk->get_pos_st1();
    td.mom_st1        = trk->get_mom_st1();
    td.pos_st3        = trk->get_pos_st3();
    td.mom_st3        = trk->get_mom_st3();
    m_trk_list.push_back(td);
  }
  
  m_tree->Fill();
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaTriggerAndTrack::End(PHCompositeNode* topNode)
{
  m_file->cd();
  m_file->Write();
  m_file->Close();  
  return Fun4AllReturnCodes::EVENT_OK;
}

void AnaTriggerAndTrack::AnalyzeTree(TChain* tree)
{
  string dir_out = "result/trigger";
  cout << "N of trees = " << tree->GetNtrees() << endl;
  gSystem->mkdir(dir_out.c_str(), true);
  ofstream ofs(dir_out + "/result.txt");

  TFile* file_out = new TFile((dir_out+"/result.root").c_str(), "RECREATE");

  TH1* h1_cnt = new TH1D("h1_cnt", "", 10, 0.5, 10.5);
  
  TH1* h1_n_pos_top  = new TH1D("h1_n_pos_top" ,  ";N of fired pos top roads;N of events", 10, -0.5, 9.5);
  TH1* h1_n_pos_bot  = new TH1D("h1_n_pos_bot" ,  ";N of fired pos bot roads;N of events", 10, -0.5, 9.5);
  TH1* h1_n_neg_top  = new TH1D("h1_n_neg_top" ,  ";N of fired neg top roads;N of events", 10, -0.5, 9.5);
  TH1* h1_n_neg_bot  = new TH1D("h1_n_neg_bot" ,  ";N of fired neg bot roads;N of events", 10, -0.5, 9.5);

  TH1* h1_chi2_pos = new TH1D("h1_chi2_pos", ";#mu^{+}: Track #chi^{2};N of tracks", 100, 0, 50);
  TH1* h1_chi2_neg = new TH1D("h1_chi2_neg", ";#mu^{-}: Track #chi^{2};N of tracks", 100, 0, 50);

  TH1* h1_nhit_pos = new TH1D("h1_nhit_pos", ";#mu^{+}: N of hits/track;N of tracks", 10, 9.5, 19.5);
  TH1* h1_nhit_neg = new TH1D("h1_nhit_neg", ";#mu^{+}: N of hits/track;N of tracks", 10, 9.5, 19.5);
  
  TH1* h1_px_pos = new TH1D("h1_px_pos", ";#mu^{+}: px (GeV);N of tracks", 100, -10, 10);
  TH1* h1_px_neg = new TH1D("h1_px_neg", ";#mu^{-}: px (GeV);N of tracks", 100, -10, 10);

  TH1* h1_py_pos = new TH1D("h1_py_pos", ";#mu^{+}: py (GeV);N of tracks", 100, -4, 4);
  TH1* h1_py_neg = new TH1D("h1_py_neg", ";#mu^{-}: py (GeV);N of tracks", 100, -4, 4);

  TH1* h1_pz_pos = new TH1D("h1_pz_pos", ";#mu^{+}: pz (GeV);N of tracks", 100, 0, 100);
  TH1* h1_pz_neg = new TH1D("h1_pz_neg", ";#mu^{-}: pz (GeV);N of tracks", 100, 0, 100);

  TH1* h1_z_pos = new TH1D("h1_z_pos", ";#mu^{+}: z (cm);N of tracks", 100, -400, 400);
  TH1* h1_z_neg = new TH1D("h1_z_neg", ";#mu^{-}: z (cm);N of tracks", 100, -400, 400);
  
  TH1* h2_xy1_pos = new TH2D("h2_xy1_pos", "#mu^{+} @ St.1;x (cm) @ St.1;y (cm) @ St.1", 100, -70, 30,  100, -60, 60);
  TH1* h2_xy1_neg = new TH2D("h2_xy1_neg", "#mu^{-} @ St.1;x (cm) @ St.1;y (cm) @ St.1", 100, -30, 70,  100, -60, 60);

  TH1* h2_xy3_pos = new TH2D("h2_xy3_pos", "#mu^{+} @ St.3;x (cm) @ St.3;y (cm) @ St.3", 100, -40, 120,  100, -150, 150);
  TH1* h2_xy3_neg = new TH2D("h2_xy3_neg", "#mu^{-} @ St.3;x (cm) @ St.3;y (cm) @ St.3", 100, -120, 40,  100, -150, 150);

  TH1* h1_ntrk_pos_top = new TH1D("h1_ntrk_pos_top", ";N of pos top tracks;N of events", 5, -0.5, 4.5);
  TH1* h1_ntrk_pos_bot = new TH1D("h1_ntrk_pos_bot", ";N of pos bot tracks;N of events", 5, -0.5, 4.5);
  TH1* h1_ntrk_neg_top = new TH1D("h1_ntrk_neg_top", ";N of neg top tracks;N of events", 5, -0.5, 4.5);
  TH1* h1_ntrk_neg_bot = new TH1D("h1_ntrk_neg_bot", ";N of neg bot tracks;N of events", 5, -0.5, 4.5);

  TH2* h2_road_idx = new TH2D("h2_road_idx", ";Road index;Pos/Neg Top/Bot", 120, -1.5, 118.5, 4, -0.5, 3.5);

  TH1* h1_h1t_pos = new TH1D("h1_h1t_pos", ";H1T paddle;N of pos tracks", 23, 0.5, 23.5);
  TH1* h1_h2t_pos = new TH1D("h1_h2t_pos", ";H2T paddle;N of pos tracks", 16, 0.5, 16.5);
  TH1* h1_h3t_pos = new TH1D("h1_h3t_pos", ";H3T paddle;N of pos tracks", 16, 0.5, 16.5);
  TH1* h1_h4t_pos = new TH1D("h1_h4t_pos", ";H4T paddle;N of pos tracks", 16, 0.5, 16.5);
  TH1* h1_h1b_pos = new TH1D("h1_h1b_pos", ";H1T paddle;N of pos tracks", 23, 0.5, 23.5);
  TH1* h1_h2b_pos = new TH1D("h1_h2b_pos", ";H2T paddle;N of pos tracks", 16, 0.5, 16.5);
  TH1* h1_h3b_pos = new TH1D("h1_h3b_pos", ";H3T paddle;N of pos tracks", 16, 0.5, 16.5);
  TH1* h1_h4b_pos = new TH1D("h1_h4b_pos", ";H4T paddle;N of pos tracks", 16, 0.5, 16.5);
  TH1* h1_h1t_neg = new TH1D("h1_h1t_neg", ";H1T paddle;N of neg tracks", 23, 0.5, 23.5);
  TH1* h1_h2t_neg = new TH1D("h1_h2t_neg", ";H2T paddle;N of neg tracks", 16, 0.5, 16.5);
  TH1* h1_h3t_neg = new TH1D("h1_h3t_neg", ";H3T paddle;N of neg tracks", 16, 0.5, 16.5);
  TH1* h1_h4t_neg = new TH1D("h1_h4t_neg", ";H4T paddle;N of neg tracks", 16, 0.5, 16.5);
  TH1* h1_h1b_neg = new TH1D("h1_h1b_neg", ";H1T paddle;N of bot tracks", 23, 0.5, 23.5);
  TH1* h1_h2b_neg = new TH1D("h1_h2b_neg", ";H2T paddle;N of bot tracks", 16, 0.5, 16.5);
  TH1* h1_h3b_neg = new TH1D("h1_h3b_neg", ";H3T paddle;N of bot tracks", 16, 0.5, 16.5);
  TH1* h1_h4b_neg = new TH1D("h1_h4b_neg", ";H4T paddle;N of bot tracks", 16, 0.5, 16.5);
  
  //TH1* h1_nhit_pos = new TH1D("h1_nhit_pos", "#mu^{+};N of hits/track;", 6, 12.5, 18.5);
  //TH1* h1_chi2_pos = new TH1D("h1_chi2_pos", "#mu^{+};Track #chi^{2};", 100, 0, 2);
  //TH1* h1_z_pos    = new TH1D("h1_z_pos"   , "#mu^{+};Track z (cm);"  , 100, -700, 300);
  //TH1* h1_pz_pos   = new TH1D("h1_pz_pos"  , "#mu^{+};Track p_{z} (GeV);", 100, 0, 100);

  //vector<int> list_spill_sel;
  //ifstream ifs("list_spill_H3TGVM_small.txt"); // ("list_spill_H3TGHS_ok.txt");
  //int spill_id;
  //while (ifs >> spill_id) list_spill_sel.push_back(spill_id);
  //ifs.close();
  
  //GeomSvc* geom = GeomSvc::instance();
  ostringstream oss;
  
  EventData* evt = 0;
  RoadData* road_data = 0;
  TrackList* trk_list = 0;
  tree->SetBranchAddress("event"     , &evt);
  tree->SetBranchAddress("road"      , &road_data);
  tree->SetBranchAddress("track_list", &trk_list);

  int n_ent = tree->GetEntries();
  cout << "N of entries = " << n_ent << endl;
  for (int i_ent = 0; i_ent < n_ent; i_ent++) {
    if ((i_ent+1) % (n_ent/10) == 0) cout << "  " << 10*(i_ent+1)/(n_ent/10) << "%" << flush;
    tree->GetEntry(i_ent);
    h1_cnt->Fill(1);
    //ofs << evt->run_id << " " << evt->spill_id << " " << evt->event_id << " " << evt->D1 << " " << evt->D2 << " " << evt->D3p << " " << evt->D3m << endl;

    //if (std::find(list_spill_sel.begin(), list_spill_sel.end(), evt->spill_id) == list_spill_sel.end()) continue;
    
    //if (evt->run_id != 6155 || evt->spill_id != 1941910) continue;
    //if (! (evt->fpga_bits & 0x1)) continue;
    //if (! (evt->fpga_bits & 0x8)) continue;
    if (! (evt->nim_bits & 0x2)) continue;
    h1_cnt->Fill(2);

    int n_pt = road_data->pos_top.size();
    int n_pb = road_data->pos_bot.size();
    int n_nt = road_data->neg_top.size();
    int n_nb = road_data->neg_bot.size();
    h1_n_pos_top->Fill(n_pt);
    h1_n_pos_bot->Fill(n_pb);
    h1_n_neg_top->Fill(n_nt);
    h1_n_neg_bot->Fill(n_nb);
    if (n_pt + n_pb + n_nt + n_nb > 0) h1_cnt->Fill(3);
    
    int ntrk_pos_top = 0;
    int ntrk_pos_bot = 0;
    int ntrk_neg_top = 0;
    int ntrk_neg_bot = 0;
    for (auto it = trk_list->begin(); it != trk_list->end(); it++) {
      TrackData* td = &(*it);
      int charge = td->charge;
      int road_idx = td->road_idx;
      int    nhit = td->n_hits;
      double chi2 = td->chisq;
      double px  = td->mom_vtx.X();
      double py  = td->mom_vtx.Y();
      double pz  = td->mom_vtx.Z();
      double z   = td->pos_vtx.Z();
      double x1  = td->pos_st1.X();
      double x3  = td->pos_st3.X();
      double y1  = td->pos_st1.Y();
      double y3  = td->pos_st3.Y();
      if (charge > 0) {
        if (py > 0) h2_road_idx->Fill(road_idx, 0.0);
        else        h2_road_idx->Fill(road_idx, 1.0);
      } else {
        if (py > 0) h2_road_idx->Fill(road_idx, 2.0);
        else        h2_road_idx->Fill(road_idx, 3.0);
      }
      //if (road_idx >= 0 && 20 < pz && pz < 50 && x3 < -100) {
      //if (road_idx >= 0 && 30 < pz && pz < 32) {
      //if (nhit >= 14 && chi2 < 5 && 50 < z && z < 100 && pz > 20) { 
      if (true) { 
        int h1, h2, h3, h4, tb;
        UtilTrigger::Road2Hodo(td->road, h1, h2, h3, h4, tb);
        if (charge > 0) {
          if (py > 0) ntrk_pos_top++;
          else        ntrk_pos_bot++;
          h1_chi2_pos->Fill(chi2);
          h1_nhit_pos->Fill(nhit);
          h1_px_pos->Fill(px);
          h1_py_pos->Fill(py);
          h1_pz_pos->Fill(pz);
          h1_z_pos ->Fill(z );
          if (py > 0) {
            h1_h1t_pos->Fill(h1);
            h1_h2t_pos->Fill(h2);
            h1_h3t_pos->Fill(h3);
            h1_h4t_pos->Fill(h4);
          } else {
            h1_h1b_pos->Fill(h1);
            h1_h2b_pos->Fill(h2);
            h1_h3b_pos->Fill(h3);
            h1_h4b_pos->Fill(h4);
          }
          h2_xy1_pos->Fill(x1, y1);
          h2_xy3_pos->Fill(x3, y3);
        } else { // charge < 0
          if (py > 0) ntrk_neg_top++;
          else        ntrk_neg_bot++;
          h1_chi2_neg->Fill(chi2);
          h1_nhit_neg->Fill(nhit);
          h1_px_neg->Fill(px);
          h1_py_neg->Fill(py);
          h1_pz_neg->Fill(pz);
          h1_z_neg ->Fill(z );
          if (py > 0) {
            h1_h1t_neg->Fill(h1);
            h1_h2t_neg->Fill(h2);
            h1_h3t_neg->Fill(h3);
            h1_h4t_neg->Fill(h4);
          } else {
            h1_h1b_neg->Fill(h1);
            h1_h2b_neg->Fill(h2);
            h1_h3b_neg->Fill(h3);
            h1_h4b_neg->Fill(h4);
          }
          h2_xy1_neg->Fill(x1, y1);
          h2_xy3_neg->Fill(x3, y3);
        }
      }
    }
    if (n_pt > 0) h1_ntrk_pos_top->Fill(ntrk_pos_top);
    if (n_pb > 0) h1_ntrk_pos_bot->Fill(ntrk_pos_bot);
    if (n_nt > 0) h1_ntrk_neg_top->Fill(ntrk_neg_top);
    if (n_nb > 0) h1_ntrk_neg_bot->Fill(ntrk_neg_bot);
  }
  
  TCanvas* c1 = new TCanvas("c1", "");
  c1->SetGrid();
  //c1->SetLogy(true);

  h1_cnt->Draw();  c1->SaveAs((dir_out+"/h1_cnt.png").c_str());
  ofs << "N: all    = " << h1_cnt->GetBinContent(1) << "\n"
      << "N: trig   = " << h1_cnt->GetBinContent(2) << "\n"
      << "N: road>0 = " << h1_cnt->GetBinContent(3) << "\n";
  
  h1_n_pos_top->Draw();  c1->SaveAs((dir_out+"/h1_n_pos_top.png").c_str());
  h1_n_pos_bot->Draw();  c1->SaveAs((dir_out+"/h1_n_pos_bot.png").c_str());
  h1_n_neg_top->Draw();  c1->SaveAs((dir_out+"/h1_n_neg_top.png").c_str());
  h1_n_neg_bot->Draw();  c1->SaveAs((dir_out+"/h1_n_neg_bot.png").c_str());

  h1_chi2_pos->Draw();  c1->SaveAs((dir_out+"/h1_chi2_pos.png").c_str());
  h1_chi2_neg->Draw();  c1->SaveAs((dir_out+"/h1_chi2_neg.png").c_str());

  h1_nhit_pos->Draw();  c1->SaveAs((dir_out+"/h1_nhit_pos.png").c_str());
  h1_nhit_neg->Draw();  c1->SaveAs((dir_out+"/h1_nhit_neg.png").c_str());

  h1_px_pos->Draw();  c1->SaveAs((dir_out+"/h1_px_pos.png").c_str());
  h1_px_neg->Draw();  c1->SaveAs((dir_out+"/h1_px_neg.png").c_str());
  
  h1_py_pos->Draw();  c1->SaveAs((dir_out+"/h1_py_pos.png").c_str());
  h1_py_neg->Draw();  c1->SaveAs((dir_out+"/h1_py_neg.png").c_str());

  h1_pz_pos->Draw();  c1->SaveAs((dir_out+"/h1_pz_pos.png").c_str());
  h1_pz_neg->Draw();  c1->SaveAs((dir_out+"/h1_pz_neg.png").c_str());

  h1_z_pos->Draw();  c1->SaveAs((dir_out+"/h1_z_pos.png").c_str());
  h1_z_neg->Draw();  c1->SaveAs((dir_out+"/h1_z_neg.png").c_str());
  
  h2_xy1_pos->Draw("colz");  c1->SaveAs((dir_out+"/h2_xy1_pos.png").c_str());
  h2_xy1_neg->Draw("colz");  c1->SaveAs((dir_out+"/h2_xy1_neg.png").c_str());

  h2_xy3_pos->Draw("colz");  c1->SaveAs((dir_out+"/h2_xy3_pos.png").c_str());
  h2_xy3_neg->Draw("colz");  c1->SaveAs((dir_out+"/h2_xy3_neg.png").c_str());

  h1_ntrk_pos_top->Draw();  c1->SaveAs((dir_out+"/h1_ntrk_pos_top.png").c_str());
  h1_ntrk_pos_bot->Draw();  c1->SaveAs((dir_out+"/h1_ntrk_pos_bot.png").c_str());
  h1_ntrk_neg_top->Draw();  c1->SaveAs((dir_out+"/h1_ntrk_neg_top.png").c_str());
  h1_ntrk_neg_bot->Draw();  c1->SaveAs((dir_out+"/h1_ntrk_neg_bot.png").c_str());

  //h1_nhit_pos->Draw();  c1->SaveAs((dir_out+"/h1_nhit_pos.png").c_str());
  //h1_chi2_pos->Draw();  c1->SaveAs((dir_out+"/h1_chi2_pos.png").c_str());
  //h1_z_pos   ->Draw();  c1->SaveAs((dir_out+"/h1_z_pos.png").c_str());
  //h1_pz_pos  ->Draw();  c1->SaveAs((dir_out+"/h1_pz_pos.png").c_str());

  //h2_road_idx->Draw("colz");  c1->SaveAs((dir_out+"/h2_road_idx.png").c_str());
  TH1* h1_road_idx_pos_top = h2_road_idx->ProjectionX("h1_road_idx_pos_top", 1, 1);
  TH1* h1_road_idx_pos_bot = h2_road_idx->ProjectionX("h1_road_idx_pos_bot", 2, 2);
  TH1* h1_road_idx_neg_top = h2_road_idx->ProjectionX("h1_road_idx_neg_top", 3, 3);
  TH1* h1_road_idx_neg_bot = h2_road_idx->ProjectionX("h1_road_idx_neg_bot", 4, 4);
  h1_road_idx_pos_top->Draw();  c1->SaveAs((dir_out+"/h1_road_idx_pos_top.png").c_str());
  h1_road_idx_pos_bot->Draw();  c1->SaveAs((dir_out+"/h1_road_idx_pos_bot.png").c_str());
  h1_road_idx_neg_top->Draw();  c1->SaveAs((dir_out+"/h1_road_idx_neg_top.png").c_str());
  h1_road_idx_neg_bot->Draw();  c1->SaveAs((dir_out+"/h1_road_idx_neg_bot.png").c_str());

  h1_h1t_pos->Draw();  c1->SaveAs((dir_out+"/h1_h1t_pos.png").c_str());
  h1_h2t_pos->Draw();  c1->SaveAs((dir_out+"/h1_h2t_pos.png").c_str());
  h1_h3t_pos->Draw();  c1->SaveAs((dir_out+"/h1_h3t_pos.png").c_str());
  h1_h4t_pos->Draw();  c1->SaveAs((dir_out+"/h1_h4t_pos.png").c_str());
  h1_h1b_pos->Draw();  c1->SaveAs((dir_out+"/h1_h1b_pos.png").c_str());
  h1_h2b_pos->Draw();  c1->SaveAs((dir_out+"/h1_h2b_pos.png").c_str());
  h1_h3b_pos->Draw();  c1->SaveAs((dir_out+"/h1_h3b_pos.png").c_str());
  h1_h4b_pos->Draw();  c1->SaveAs((dir_out+"/h1_h4b_pos.png").c_str());
  h1_h1t_neg->Draw();  c1->SaveAs((dir_out+"/h1_h1t_neg.png").c_str());
  h1_h2t_neg->Draw();  c1->SaveAs((dir_out+"/h1_h2t_neg.png").c_str());
  h1_h3t_neg->Draw();  c1->SaveAs((dir_out+"/h1_h3t_neg.png").c_str());
  h1_h4t_neg->Draw();  c1->SaveAs((dir_out+"/h1_h4t_neg.png").c_str());
  h1_h1b_neg->Draw();  c1->SaveAs((dir_out+"/h1_h1b_neg.png").c_str());
  h1_h2b_neg->Draw();  c1->SaveAs((dir_out+"/h1_h2b_neg.png").c_str());
  h1_h3b_neg->Draw();  c1->SaveAs((dir_out+"/h1_h3b_neg.png").c_str());
  h1_h4b_neg->Draw();  c1->SaveAs((dir_out+"/h1_h4b_neg.png").c_str());
  
  //c1->SetLogy(true);
  delete c1;

  ofs.close();
  file_out->Write();
  file_out->Close();
}
