#include <fstream>
#include <iomanip>
#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
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
#include <UtilAna/UtilTrigger.h>
#include <UtilAna/UtilTrack.h>
#include "AnaRecoDimuon.h"
using namespace std;

AnaRecoDimuon::AnaRecoDimuon(const std::string& name)
  : SubsysReco  (name)
  , m_rs_id     (0)
  , m_sq_evt    (0)
  , m_sq_hit_vec(0)
  , m_sq_trk_vec(0)
  , m_sq_dim_vec(0)
  , m_file_name ("output.root")
  , m_file      (0)
  , m_tree      (0)
{
  ;
}

AnaRecoDimuon::~AnaRecoDimuon()
{
  ;
}

int AnaRecoDimuon::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaRecoDimuon::InitRun(PHCompositeNode* topNode)
{
  //GeomSvc* geom = GeomSvc::instance();
  m_sq_evt     = findNode::getClass<SQEvent       >(topNode, "SQEvent");
  m_sq_hit_vec = findNode::getClass<SQHitVector   >(topNode, "SQHitVector");
  m_sq_trk_vec = findNode::getClass<SQTrackVector >(topNode, "SQRecTrackVector");
  m_sq_dim_vec = findNode::getClass<SQDimuonVector>(topNode, "SQRecDimuonVector_PM");
  if (!m_sq_evt || !m_sq_hit_vec || !m_sq_trk_vec || !m_sq_dim_vec) return Fun4AllReturnCodes::ABORTEVENT;

  m_file = new TFile(m_file_name.c_str(), "RECREATE");
  m_tree = new TTree("tree", "Created by AnaRecoDimuon");
  m_tree->Branch("event"       , &m_evt);
  m_tree->Branch("road_list"   , &m_road_list);
  m_tree->Branch("dimuon_list" , &m_dim_list);
  //m_tree->Branch("trk_pos_list", &m_trk_pos_list);
  //m_tree->Branch("trk_neg_list", &m_trk_neg_list);

  if (m_rs_id != 0) {
    int ret = m_rs.LoadConfig(m_rs_id);
    if (ret != 0) {
      cout << "!!WARNING!!  AnaSimDst::InitRun():  roadset.LoadConfig returned " << ret << ".\n";
    }
    cout <<"Roadset " << m_rs.str(1) << endl;
  }
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaRecoDimuon::process_event(PHCompositeNode* topNode)
{
  //m_evt.event_id  = m_sq_evt->get_event_id();
  m_evt.trig_bits  = m_sq_evt->get_trigger();  
  
  m_road_list.clear();
  m_dim_list.clear();
  //m_trk_pos_list.clear();
  //m_trk_neg_list.clear();
  for (auto it = m_sq_dim_vec->begin(); it != m_sq_dim_vec->end(); it++) {
    SRecDimuon* dim = dynamic_cast<SRecDimuon*>(*it);
    int trk_id_pos = dim->get_track_id_pos();
    int trk_id_neg = dim->get_track_id_neg();
    SRecTrack* trk_pos = dynamic_cast<SRecTrack*>(m_sq_trk_vec->at(trk_id_pos));
    SRecTrack* trk_neg = dynamic_cast<SRecTrack*>(m_sq_trk_vec->at(trk_id_neg));
    int road_pos = trk_pos->getTriggerRoad();
    int road_neg = trk_neg->getTriggerRoad();

    UtilTrigger::TrigRoads* roads_pos_top = m_rs.PosTop();
    UtilTrigger::TrigRoads* roads_pos_bot = m_rs.PosBot();
    UtilTrigger::TrigRoads* roads_neg_top = m_rs.NegTop();
    UtilTrigger::TrigRoads* roads_neg_bot = m_rs.NegBot();
    bool pos_top = roads_pos_top->FindRoad(road_pos);
    bool pos_bot = roads_pos_bot->FindRoad(road_pos);
    bool neg_top = roads_neg_top->FindRoad(road_neg);
    bool neg_bot = roads_neg_bot->FindRoad(road_neg);
    
    const double margin = 0.0; // cm
    UtilTrack::verbosity = 4;
    std::vector<int> list_road_pos_0 = UtilTrack::FindMatchedRoads(trk_pos, margin);
    CheckRoadList(list_road_pos_0, road_pos, "list_road_pos");
    std::vector<int> list_road_neg_0 = UtilTrack::FindMatchedRoads(trk_neg, margin);
    CheckRoadList(list_road_neg_0, road_neg, "list_road_neg");
    RoadData rd;
    rd.pos_top = roads_pos_top->FindRoadIDs(list_road_pos_0);
    rd.pos_bot = roads_pos_bot->FindRoadIDs(list_road_pos_0);
    rd.neg_top = roads_neg_top->FindRoadIDs(list_road_neg_0);
    rd.neg_bot = roads_neg_bot->FindRoadIDs(list_road_neg_0);

    RecoDimuonData ddr;
    ddr.pos_top    = pos_top;
    ddr.pos_bot    = pos_bot;
    ddr.neg_top    = neg_top;
    ddr.neg_bot    = neg_bot;
    ddr.pos        = dim->get_pos();
    ddr.mom        = dim->get_mom();
    ddr.mom_target = dim->p_pos_target + dim->p_neg_target;
    ddr.mom_dump   = dim->p_pos_dump   + dim->p_neg_dump;

    ddr.trk_pos.charge         = trk_pos->get_charge();
    ddr.trk_pos.road           = road_pos;
    ddr.trk_pos.n_hits         = trk_pos->get_num_hits();
    ddr.trk_pos.chisq          = trk_pos->get_chisq();
    ddr.trk_pos.chisq_target   = trk_pos->get_chisq_target();
    ddr.trk_pos.chisq_dump     = trk_pos->get_chisq_dump();
    ddr.trk_pos.chisq_upstream = trk_pos->get_chsiq_upstream();
    ddr.trk_pos.pos_vtx        = trk_pos->get_pos_vtx();
    ddr.trk_pos.mom_vtx        = trk_pos->get_mom_vtx();
    ddr.trk_pos.pos_st1        = trk_pos->get_pos_st1();
    ddr.trk_pos.mom_st1        = trk_pos->get_mom_st1();
    ddr.trk_pos.pos_st3        = trk_pos->get_pos_st3();
    ddr.trk_pos.mom_st3        = trk_pos->get_mom_st3();
    ddr.trk_pos.pos_target     = trk_pos->get_pos_target();
    ddr.trk_pos.pos_dump       = trk_pos->get_pos_dump();
    
    ddr.trk_neg.charge         = trk_neg->get_charge();
    ddr.trk_neg.road           = road_neg;
    ddr.trk_neg.n_hits         = trk_neg->get_num_hits();
    ddr.trk_neg.chisq          = trk_neg->get_chisq();
    ddr.trk_neg.chisq_target   = trk_neg->get_chisq_target();
    ddr.trk_neg.chisq_dump     = trk_neg->get_chisq_dump();
    ddr.trk_neg.chisq_upstream = trk_neg->get_chsiq_upstream();
    ddr.trk_neg.pos_vtx        = trk_neg->get_pos_vtx();
    ddr.trk_neg.mom_vtx        = trk_neg->get_mom_vtx();
    ddr.trk_neg.pos_st1        = trk_neg->get_pos_st1();
    ddr.trk_neg.mom_st1        = trk_neg->get_mom_st1();
    ddr.trk_neg.pos_st3        = trk_neg->get_pos_st3();
    ddr.trk_neg.mom_st3        = trk_neg->get_mom_st3();
    ddr.trk_neg.pos_target     = trk_neg->get_pos_target();
    ddr.trk_neg.pos_dump       = trk_neg->get_pos_dump();
    
    m_road_list   .push_back(rd);
    m_dim_list    .push_back(ddr);
    //m_trk_pos_list.push_back(td_pos);
    //m_trk_neg_list.push_back(td_neg);
  }
  
  m_tree->Fill();
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaRecoDimuon::End(PHCompositeNode* topNode)
{
  m_file->cd();
  m_file->Write();
  m_file->Close();  
  return Fun4AllReturnCodes::EVENT_OK;
}

void AnaRecoDimuon::AnalyzeTree(TTree* tree, const bool road_match)
{
  string dir_out = "result";
  gSystem->mkdir(dir_out.c_str(), true);
  ofstream ofs(dir_out + "/result.txt");

  TFile* file_out = new TFile((dir_out+"/result.root").c_str(), "RECREATE");
  
  TH1* h1_nhit_pos = new TH1D("h1_nhit_pos", "#mu^{+};N of hits/track;", 6, 12.5, 18.5);
  TH1* h1_chi2_pos = new TH1D("h1_chi2_pos", "#mu^{+};Track #chi^{2};", 100, 0, 10);
  TH1* h1_z_pos    = new TH1D("h1_z_pos"   , "#mu^{+};Track z (cm);"  , 100, -700, 300);
  TH1* h1_px_pos   = new TH1D("h1_px_pos"  , "#mu^{+};Track p_{x} (GeV);", 100, -5, 5);
  TH1* h1_py_pos   = new TH1D("h1_py_pos"  , "#mu^{+};Track p_{y} (GeV);", 100, -5, 5);
  TH1* h1_pz_pos   = new TH1D("h1_pz_pos"  , "#mu^{+};Track p_{z} (GeV);", 100, 0, 100);
  
  TH1* h1_chi2_tgt_pos = new TH1D("h1_chi2_tgt_pos", "#mu^{+};Track #chi^{2} at target;"  , 100, 0, 10);
  TH1* h1_chi2_dum_pos = new TH1D("h1_chi2_dum_pos", "#mu^{+};Track #chi^{2} at dump;"    , 100, 0, 10);
  TH1* h1_chi2_ups_pos = new TH1D("h1_chi2_ups_pos", "#mu^{+};Track #chi^{2} at upstream;", 100, 0, 10);
  TH1* h1_chi2_tmd_pos = new TH1D("h1_chi2_tmd_pos", "#mu^{+};#chi^{2}_{Target} - #chi^{2}_{Dump};"    , 100, -10, 10);
  TH1* h1_chi2_tmu_pos = new TH1D("h1_chi2_tmu_pos", "#mu^{+};#chi^{2}_{Target} - #chi^{2}_{Upstream};", 100, -10, 10);
  
  TH1* h1_nhit_neg = new TH1D("h1_nhit_neg", "#mu^{-};N of hits/track;", 6, 12.5, 18.5);
  TH1* h1_chi2_neg = new TH1D("h1_chi2_neg", "#mu^{-};Track #chi^{2};", 100, 0, 10);
  TH1* h1_z_neg    = new TH1D("h1_z_neg"   , "#mu^{-};Track z (cm);", 100, -700, 300);
  TH1* h1_px_neg   = new TH1D("h1_px_neg"  , "#mu^{-};Track p_{x} (GeV);", 100, -5, 5);
  TH1* h1_py_neg   = new TH1D("h1_py_neg"  , "#mu^{-};Track p_{y} (GeV);", 100, -5, 5);
  TH1* h1_pz_neg   = new TH1D("h1_pz_neg"  , "#mu^{-};Track p_{z} (GeV);", 100, 0, 100);

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
  TH1* h1_dpx_tgt = new TH1D("h1_dpx_tgt", ";Dimuon p_{x} (GeV);", 100, -5, 5);
  TH1* h1_dpy_tgt = new TH1D("h1_dpy_tgt", ";Dimuon p_{y} (GeV);", 100, -5, 5);
  TH1* h1_dpz_tgt = new TH1D("h1_dpz_tgt", ";Dimuon p_{z} (GeV);", 100, 30, 130);
  TH1* h1_m_tgt   = new TH1D("h1_m_tgt"  , ";Dimuon mass (GeV);" , 100, 0, 10);

  //GeomSvc* geom = GeomSvc::instance();
  ostringstream oss;
  
  EventData* evt = 0;
  RoadList* road_list = 0;
  RecoDimuonList* dim_list = 0;
  //TrackList*  trk_pos_list = 0;
  //TrackList*  trk_neg_list = 0;
  tree->SetBranchAddress("event"       , &evt);
  tree->SetBranchAddress("road_list"   , &road_list);
  tree->SetBranchAddress("dimuon_list" , &dim_list);
  //tree->SetBranchAddress("trk_pos_list", &trk_pos_list);
  //tree->SetBranchAddress("trk_neg_list", &trk_neg_list);

  int n_ent = tree->GetEntries();
  cout << "N of entries = " << n_ent << endl;
  for (int i_ent = 0; i_ent < n_ent; i_ent++) {
    if ((i_ent+1) % (n_ent/10) == 0) cout << "  " << 10*(i_ent+1)/(n_ent/10) << "%" << flush;
    tree->GetEntry(i_ent);

    for (size_t idim = 0; idim < dim_list->size(); idim++) {
      RecoDimuonData* dd    = &dim_list->at(idim);
      RoadData*       rd    = &road_list->at(idim);
      RecoTrackData* td_pos = &dd->trk_pos;
      RecoTrackData* td_neg = &dd->trk_neg;

      double trk_sep      = td_pos->pos_vtx.Z() - td_neg->pos_vtx.Z();
      double chi2_tgt_pos = td_pos->chisq_target;
      double chi2_dum_pos = td_pos->chisq_dump;
      double chi2_ups_pos = td_pos->chisq_upstream;
      double chi2_tgt_neg = td_neg->chisq_target;
      double chi2_dum_neg = td_neg->chisq_dump;
      double chi2_ups_neg = td_neg->chisq_upstream;
      if (dd->pos        .Z() < -690 ||
          td_pos->pos_vtx.Z() < -690 || td_neg->pos_vtx.Z() < -690 ||
          td_pos->mom_vtx.Z() <    5 || td_neg->mom_vtx.Z() <    5   ) continue;
      if (fabs(trk_sep) > 200) continue;

      double y_st1_pos = td_pos->pos_st1.Y();
      double y_st1_neg = td_neg->pos_st1.Y();
      if (fabs(y_st1_pos) < 3 || fabs(y_st1_neg) < 3) continue;
      
      if (road_match) {
        //bool top_bot = dd->pos_top && dd->neg_bot;
        //bool bot_top = dd->pos_bot && dd->neg_top;
        bool top_bot = (rd->pos_top.size() > 0 && rd->neg_bot.size() > 0);
        bool bot_top = (rd->pos_bot.size() > 0 && rd->neg_top.size() > 0);
        if (!top_bot && !bot_top) continue;
        //if (top_bot || bot_top) continue; // anti-matching
      }

      if (chi2_tgt_pos < 0 || chi2_dum_pos < 0 || chi2_ups_pos < 0 ||
          chi2_tgt_pos - chi2_dum_pos > 0 || chi2_tgt_pos - chi2_ups_pos > 0) continue;
      if (chi2_tgt_neg < 0 || chi2_dum_neg < 0 || chi2_ups_neg < 0 ||
          chi2_tgt_neg - chi2_dum_neg > 0 || chi2_tgt_neg - chi2_ups_neg > 0) continue;

      h1_nhit_pos->Fill(td_pos->n_hits);
      h1_chi2_pos->Fill(td_pos->chisq);
      h1_z_pos   ->Fill(td_pos->pos_vtx.Z());
      h1_px_pos  ->Fill(td_pos->mom_vtx.X());
      h1_py_pos  ->Fill(td_pos->mom_vtx.Y());
      h1_pz_pos  ->Fill(td_pos->mom_vtx.Z());

      h1_nhit_neg->Fill(td_neg->n_hits);
      h1_chi2_neg->Fill(td_neg->chisq);
      h1_z_neg   ->Fill(td_neg->pos_vtx.Z());
      h1_px_neg  ->Fill(td_neg->mom_vtx.X());
      h1_py_neg  ->Fill(td_neg->mom_vtx.Y());
      h1_pz_neg  ->Fill(td_neg->mom_vtx.Z());
      
      h1_chi2_tgt_pos->Fill(chi2_tgt_pos);
      h1_chi2_dum_pos->Fill(chi2_dum_pos);
      h1_chi2_ups_pos->Fill(chi2_ups_pos);
      h1_chi2_tmd_pos->Fill(chi2_tgt_pos - chi2_dum_pos);
      h1_chi2_tmu_pos->Fill(chi2_tgt_pos - chi2_ups_pos);

      h1_chi2_tgt_neg->Fill(chi2_tgt_neg);
      h1_chi2_dum_neg->Fill(chi2_dum_neg);
      h1_chi2_ups_neg->Fill(chi2_ups_neg);
      h1_chi2_tmd_neg->Fill(chi2_tgt_neg - chi2_dum_neg);
      h1_chi2_tmu_neg->Fill(chi2_tgt_neg - chi2_ups_neg);
      
      h1_dx     ->Fill(dd->pos.X());
      h1_dy     ->Fill(dd->pos.Y());
      h1_dz     ->Fill(dd->pos.Z());
      h1_dpx    ->Fill(dd->mom.X());
      h1_dpy    ->Fill(dd->mom.Y());
      h1_dpz    ->Fill(dd->mom.Z());
      h1_m      ->Fill(dd->mom.M());
      h1_trk_sep->Fill(trk_sep);
      
      h1_dz_sel ->Fill(dd->pos.Z());
      h1_dpz_sel->Fill(dd->mom.Z());
      h1_m_sel  ->Fill(dd->mom.M());
      
      h1_dz_tgt ->Fill(dd->pos.Z());
      h1_dpx_tgt->Fill(dd->mom_target.X());
      h1_dpy_tgt->Fill(dd->mom_target.Y());
      h1_dpz_tgt->Fill(dd->mom_target.Z());
      h1_m_tgt  ->Fill(dd->mom_target.M());
    }
  }
  
  TCanvas* c1 = new TCanvas("c1", "");
  c1->SetGrid();
  //c1->SetLogy(true);

  h1_nhit_pos->Draw();  c1->SaveAs((dir_out+"/h1_nhit_pos.png").c_str());
  h1_chi2_pos->Draw();  c1->SaveAs((dir_out+"/h1_chi2_pos.png").c_str());
  h1_z_pos   ->Draw();  c1->SaveAs((dir_out+"/h1_z_pos.png").c_str());
  h1_px_pos  ->Draw();  c1->SaveAs((dir_out+"/h1_px_pos.png").c_str());
  h1_py_pos  ->Draw();  c1->SaveAs((dir_out+"/h1_py_pos.png").c_str());
  h1_pz_pos  ->Draw();  c1->SaveAs((dir_out+"/h1_pz_pos.png").c_str());

  h1_chi2_tgt_pos->Draw();  c1->SaveAs((dir_out+"/h1_chi2_tgt_pos.png").c_str());
  h1_chi2_dum_pos->Draw();  c1->SaveAs((dir_out+"/h1_chi2_dum_pos.png").c_str());
  h1_chi2_ups_pos->Draw();  c1->SaveAs((dir_out+"/h1_chi2_ups_pos.png").c_str());
  h1_chi2_tmd_pos->Draw();  c1->SaveAs((dir_out+"/h1_chi2_tmd_pos.png").c_str());
  h1_chi2_tmu_pos->Draw();  c1->SaveAs((dir_out+"/h1_chi2_tmu_pos.png").c_str());
  
  h1_nhit_neg->Draw();  c1->SaveAs((dir_out+"/h1_nhit_neg.png").c_str());
  h1_chi2_neg->Draw();  c1->SaveAs((dir_out+"/h1_chi2_neg.png").c_str());
  h1_z_neg   ->Draw();  c1->SaveAs((dir_out+"/h1_z_neg.png").c_str());
  h1_px_neg  ->Draw();  c1->SaveAs((dir_out+"/h1_px_neg.png").c_str());
  h1_py_neg  ->Draw();  c1->SaveAs((dir_out+"/h1_py_neg.png").c_str());
  h1_pz_neg  ->Draw();  c1->SaveAs((dir_out+"/h1_pz_neg.png").c_str());

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

  h1_dpx_tgt->SetLineColor(kBlue);
  h1_dpx_tgt->SetLineWidth(2);
  h1_dpx_tgt->Draw();
  c1->SaveAs((dir_out+"/h1_dpx_tgt.png").c_str());

  h1_dpy_tgt->SetLineColor(kBlue);
  h1_dpy_tgt->SetLineWidth(2);
  h1_dpy_tgt->Draw();
  c1->SaveAs((dir_out+"/h1_dpy_tgt.png").c_str());

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

void AnaRecoDimuon::CheckRoadList(const std::vector<int>& list_road, const int road, const char* name)
{
  int h1, h2, h3, h4, tb;
  UtilTrigger::Road2Hodo(road, h1, h2, h3, h4, tb);
  cout << "  " << name << ": road " << road << " (" << h1 << " " << h2 << " " << h3 << " " << h4 << ")";
  if (find(list_road.begin(), list_road.end(), road) != list_road.end()) {
    cout << " Found." << endl;
  } else {
    cout << " Missed." << endl;
    for (auto it = list_road.begin(); it != list_road.end(); it++) {
      UtilTrigger::Road2Hodo(*it, h1, h2, h3, h4, tb);
      cout << "    " << *it << " (" << h1 << " " << h2 << " " << h3 << " " << h4 << ")\n";
    }
  }
}

// Find a list of roads that are expected to be fired based on the given hit list.
/**
 * This function does _not_ check if each road is in the trigger roadset.
 * The returned list contains all possible combinations of hodo hits.
 */
void AnaRecoDimuon::FindFiredRoads(const SQHitVector* vec, std::vector<int>& list_road_top, std::vector<int>& list_road_bot, const bool in_time)
{
  list_road_top.clear();
  list_road_bot.clear();
  
  unordered_map<int, vector<int> > list_ele_id; // det_id => vector<ele_id>
  for (auto it = vec->begin(); it != vec->end(); it++) {
    SQHit* hit = *it;
    if (in_time && ! hit->is_in_time()) continue;
    list_ele_id[hit->get_detector_id()].push_back(hit->get_element_id());
  }
  
  GeomSvc* geom = GeomSvc::instance();
  vector<int>* list_h1t = &list_ele_id[geom->getDetectorID("H1T")];
  vector<int>* list_h2t = &list_ele_id[geom->getDetectorID("H2T")];
  vector<int>* list_h3t = &list_ele_id[geom->getDetectorID("H3T")];
  vector<int>* list_h4t = &list_ele_id[geom->getDetectorID("H4T")];
  for (auto it1 = list_h1t->begin(); it1 != list_h1t->end(); it1++) {
  for (auto it2 = list_h2t->begin(); it2 != list_h2t->end(); it2++) {
  for (auto it3 = list_h3t->begin(); it3 != list_h3t->end(); it3++) {
  for (auto it4 = list_h4t->begin(); it4 != list_h4t->end(); it4++) {
    list_road_top.push_back(UtilTrigger::Hodo2Road(*it1, *it2, *it3, *it4, +1));
  }}}}

  vector<int>* list_h1b = &list_ele_id[geom->getDetectorID("H1B")];
  vector<int>* list_h2b = &list_ele_id[geom->getDetectorID("H2B")];
  vector<int>* list_h3b = &list_ele_id[geom->getDetectorID("H3B")];
  vector<int>* list_h4b = &list_ele_id[geom->getDetectorID("H4B")];
  for (auto it1 = list_h1b->begin(); it1 != list_h1b->end(); it1++) {
  for (auto it2 = list_h2b->begin(); it2 != list_h2b->end(); it2++) {
  for (auto it3 = list_h3b->begin(); it3 != list_h3b->end(); it3++) {
  for (auto it4 = list_h4b->begin(); it4 != list_h4b->end(); it4++) {
    list_road_top.push_back(UtilTrigger::Hodo2Road(*it1, *it2, *it3, *it4, -1));
  }}}}
}
