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
#include <UtilAna/UtilTrigger.h>
#include "AnaDimuon.h"
#include "UtilTrackX.h"
using namespace std;

AnaDimuon::AnaDimuon(const std::string& name, const std::string& mode)
  : SubsysReco  (name)
  , m_sq_evt    (0)
  , m_sq_hit_vec(0)
  , m_sq_trk_vec(0)
  , m_sq_dim_vec(0)
  , m_node_prefix("SQRecDimuonVector_")
  , m_mode      (mode)
  , m_output_tree(true)
  , m_file      (0)
  , m_tree      (0)
{
  if (m_mode != "PM" && m_mode != "PP" && m_mode != "MM") {
    cout << "AnaDimuon:  ERROR  Mode is not 'PM', 'PP' nor 'MM'.  Abort." << endl;
    exit(1);
  }
  m_file_name = "output_" + m_mode + ".root";
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
  string node_name = m_node_prefix + m_mode;
  
  m_sq_evt     = findNode::getClass<SQEvent       >(topNode, "SQEvent");
  m_sq_hit_vec = findNode::getClass<SQHitVector   >(topNode, "SQHitVector");
  m_sq_trk_vec = findNode::getClass<SQTrackVector >(topNode, "SQRecTrackVector");
  m_sq_dim_vec = findNode::getClass<SQDimuonVector>(topNode, node_name.c_str());
  if (!m_sq_evt || !m_sq_hit_vec || !m_sq_trk_vec || !m_sq_dim_vec) return Fun4AllReturnCodes::ABORTEVENT;

  m_file = new TFile(m_file_name.c_str(), "RECREATE");
  if (m_output_tree) {
    m_tree = new TTree("tree", "Created by AnaDimuon");
    m_tree->Branch("event"       , &m_evt);
    m_tree->Branch("dimuon_list" , &m_dim_list);
    m_tree->Branch("trk_pos_list", &m_trk_pos_list);
    m_tree->Branch("trk_neg_list", &m_trk_neg_list);
    m_tree->Branch("road_list_0" , &m_road_list_0);
    m_tree->Branch("road_list_1" , &m_road_list_1);
    m_tree->Branch("road_list_2" , &m_road_list_2);
  }
  
  SQRun* sq_run = findNode::getClass<SQRun>(topNode, "SQRun");
  if (!sq_run) return Fun4AllReturnCodes::ABORTEVENT;
  int LBtop = sq_run->get_v1495_id(2);
  int LBbot = sq_run->get_v1495_id(3);
  int ret = m_rs.LoadConfig(LBtop, LBbot);
  if (ret != 0) {
    cout << "!!WARNING!!  AnaDimuonLikeSign::InitRun():  roadset.LoadConfig returned " << ret << ".\n";
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
  m_trk_pos_list.clear();
  m_trk_neg_list.clear();
  m_road_list_0.clear();
  m_road_list_1.clear();
  m_road_list_2.clear();
  for (auto it = m_sq_dim_vec->begin(); it != m_sq_dim_vec->end(); it++) {
    SRecDimuon* dim = dynamic_cast<SRecDimuon*>(*it);
    int trk_id_pos = dim->get_track_id_pos();
    int trk_id_neg = dim->get_track_id_neg();
    SRecTrack* trk_pos = dynamic_cast<SRecTrack*>(m_sq_trk_vec->at(trk_id_pos));
    SRecTrack* trk_neg = dynamic_cast<SRecTrack*>(m_sq_trk_vec->at(trk_id_neg));
    int road_pos = trk_pos->getTriggerRoad();
    int road_neg = trk_neg->getTriggerRoad();

    UtilTrigger::TrigRoads* roads_pos_top;
    UtilTrigger::TrigRoads* roads_pos_bot;
    UtilTrigger::TrigRoads* roads_neg_top;
    UtilTrigger::TrigRoads* roads_neg_bot;
    if (m_mode == "PM") {
      roads_pos_top = m_rs.PosTop();
      roads_pos_bot = m_rs.PosBot();
      roads_neg_top = m_rs.NegTop();
      roads_neg_bot = m_rs.NegBot();
    } else if (m_mode == "PP") {
      roads_pos_top = m_rs.PosTop();
      roads_pos_bot = m_rs.PosBot();
      roads_neg_top = m_rs.PosTop();
      roads_neg_bot = m_rs.PosBot();
    } else { // "MM"
      roads_pos_top = m_rs.NegTop();
      roads_pos_bot = m_rs.NegBot();
      roads_neg_top = m_rs.NegTop();
      roads_neg_bot = m_rs.NegBot();
    }
    bool pos_top = roads_pos_top->FindRoad(road_pos);
    bool pos_bot = roads_pos_bot->FindRoad(road_pos);
    bool neg_top = roads_neg_top->FindRoad(road_neg);
    bool neg_bot = roads_neg_bot->FindRoad(road_neg);
    //cout << "T " << road_pos << " " << road_neg << " " << pos_top << pos_bot << neg_top << neg_bot << endl;

    RoadData rd_0;
    RoadData rd_1;
    RoadData rd_2;
    
    const double margin = 0.0; // cm
    //UtilTrackX::verbosity = 4;
    std::vector<int> list_road_pos_0 = UtilTrackX::FindMatchedRoads(trk_pos, margin);
    std::vector<int> list_road_neg_0 = UtilTrackX::FindMatchedRoads(trk_neg, margin);
    //CheckRoadList(list_road_pos, road_pos, "list_road_pos");
    //CheckRoadList(list_road_neg, road_neg, "list_road_neg");
    rd_0.pos_top = FindRoadIDs(roads_pos_top, list_road_pos_0);
    rd_0.pos_bot = FindRoadIDs(roads_pos_bot, list_road_pos_0);
    rd_0.neg_top = FindRoadIDs(roads_neg_top, list_road_neg_0);
    rd_0.neg_bot = FindRoadIDs(roads_neg_bot, list_road_neg_0);

    std::vector<int> list_road_pos_1 = UtilTrackX::FindMatchedRoads(trk_pos, 1.0);
    std::vector<int> list_road_neg_1 = UtilTrackX::FindMatchedRoads(trk_neg, 1.0);
    rd_1.pos_top = FindRoadIDs(roads_pos_top, list_road_pos_1);
    rd_1.pos_bot = FindRoadIDs(roads_pos_bot, list_road_pos_1);
    rd_1.neg_top = FindRoadIDs(roads_neg_top, list_road_neg_1);
    rd_1.neg_bot = FindRoadIDs(roads_neg_bot, list_road_neg_1);

    std::vector<int> list_road_pos_2 = UtilTrackX::FindMatchedRoads(trk_pos, 2.0);
    std::vector<int> list_road_neg_2 = UtilTrackX::FindMatchedRoads(trk_neg, 2.0);
    rd_2.pos_top = FindRoadIDs(roads_pos_top, list_road_pos_2);
    rd_2.pos_bot = FindRoadIDs(roads_pos_bot, list_road_pos_2);
    rd_2.neg_top = FindRoadIDs(roads_neg_top, list_road_neg_2);
    rd_2.neg_bot = FindRoadIDs(roads_neg_bot, list_road_neg_2);

    //cout << "road  : " << pos_top << " " << pos_bot << " " << neg_top << " " << neg_bot << " " << endl;
    //cout << "road_0: " << rd_0.pos_top.size() << " " << rd_0.pos_bot.size() << " " << rd_0.neg_top.size() << " " << rd_0.neg_bot.size() << endl;
    //cout << "road_1: " << rd_1.pos_top.size() << " " << rd_1.pos_bot.size() << " " << rd_1.neg_top.size() << " " << rd_1.neg_bot.size() << endl;
    //cout << "road_2: " << rd_2.pos_top.size() << " " << rd_2.pos_bot.size() << " " << rd_2.neg_top.size() << " " << rd_2.neg_bot.size() << endl;
    
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

    TrackData td_pos;
    td_pos.charge         = +1;
    td_pos.road           = road_pos;
    td_pos.n_hits         = trk_pos->get_num_hits();
    td_pos.chisq          = trk_pos->get_chisq();
    td_pos.chisq_target   = trk_pos->getChisqTarget();//get_chisq_target();
    td_pos.chisq_dump     = trk_pos->get_chisq_dump();
    td_pos.chisq_upstream = trk_pos->get_chsiq_upstream();
    td_pos.pos_vtx        = trk_pos->get_pos_vtx();
    td_pos.mom_vtx        = trk_pos->get_mom_vtx();
    td_pos.pos_st1        = trk_pos->get_pos_st1();
    td_pos.mom_st1        = trk_pos->get_mom_st1();
    td_pos.pos_st3        = trk_pos->get_pos_st3();
    td_pos.mom_st3        = trk_pos->get_mom_st3();
    
    TrackData td_neg;
    td_neg.charge         = +1;
    td_neg.road           = road_neg;
    td_neg.n_hits         = trk_neg->get_num_hits();
    td_neg.chisq          = trk_neg->get_chisq();
    td_neg.chisq_target   = trk_neg->getChisqTarget();//get_chisq_target();
    td_neg.chisq_dump     = trk_neg->get_chisq_dump();
    td_neg.chisq_upstream = trk_neg->get_chsiq_upstream();
    td_neg.pos_vtx        = trk_neg->get_pos_vtx();
    td_neg.mom_vtx        = trk_neg->get_mom_vtx();
    td_neg.pos_st1        = trk_neg->get_pos_st1();
    td_neg.mom_st1        = trk_neg->get_mom_st1();
    td_neg.pos_st3        = trk_neg->get_pos_st3();
    td_neg.mom_st3        = trk_neg->get_mom_st3();
    
    m_dim_list.push_back(dd);
    m_trk_pos_list.push_back(td_pos);
    m_trk_neg_list.push_back(td_neg);
    m_road_list_0.push_back(rd_0);
    m_road_list_1.push_back(rd_1);
    m_road_list_2.push_back(rd_2);
  }
  
  if (m_output_tree) m_tree->Fill();
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaDimuon::End(PHCompositeNode* topNode)
{
  m_file->cd();
  m_file->Write();
  m_file->Close();  
  return Fun4AllReturnCodes::EVENT_OK;
}

void AnaDimuon::AnalyzeTree(TChain* tree, const bool road_match)
{
  string dir_out = "result/" + m_mode;
  cout << "N of trees = " << tree->GetNtrees() << endl;
  gSystem->mkdir(dir_out.c_str(), true);
  ofstream ofs(dir_out + "/result.txt");

  TFile* file_out = new TFile((dir_out+"/result.root").c_str(), "RECREATE");
  
  TH1* h1_D1  = new TH1D("h1_D1" ,  ";D1 occupancy;N of events", 500, -0.5, 499.5);
  TH1* h1_D2  = new TH1D("h1_D2" ,  ";D2 occupancy;N of events", 300, -0.5, 299.5);
  TH1* h1_D3p = new TH1D("h1_D3p", ";D3p occupancy;N of events", 300, -0.5, 299.5);
  TH1* h1_D3m = new TH1D("h1_D3m", ";D3m occupancy;N of events", 300, -0.5, 299.5);
  
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
  DimuonList* dim_list = 0;
  RoadList* road_list_0 = 0;
  RoadList* road_list_1 = 0;
  RoadList* road_list_2 = 0;
  tree->SetBranchAddress("event"      , &evt);
  tree->SetBranchAddress("dimuon_list", &dim_list);
  tree->SetBranchAddress("road_list_0", &road_list_0);
  tree->SetBranchAddress("road_list_1", &road_list_1);
  tree->SetBranchAddress("road_list_2", &road_list_2);

  int fpga_bits_req = (m_mode == "PM"  ?  0x1  :  0x4);
  
  int n_ent = tree->GetEntries();
  cout << "N of entries = " << n_ent << endl;
  for (int i_ent = 0; i_ent < n_ent; i_ent++) {
    if ((i_ent+1) % (n_ent/10) == 0) cout << "  " << 10*(i_ent+1)/(n_ent/10) << "%" << flush;
    tree->GetEntry(i_ent);
    //ofs << evt->run_id << " " << evt->spill_id << " " << evt->event_id << " " << evt->D1 << " " << evt->D2 << " " << evt->D3p << " " << evt->D3m << endl;

    /// Require FPGA1 bit for PM or FPGA3 bit for PP/PM.
    if (! (evt->fpga_bits & fpga_bits_req)) continue;
    //if (! (evt->nim_bits & 0x4)) continue;

    h1_D1 ->Fill(evt->D1 );
    h1_D2 ->Fill(evt->D2 );
    h1_D3p->Fill(evt->D3p);
    h1_D3m->Fill(evt->D3m);
    //if (evt->D1 > 120 || evt->D2 > 60 || evt->D3p > 50 || evt->D3m > 50) continue;
    
    for (size_t idim = 0; idim < dim_list->size(); idim++) {
      DimuonData* dd = &dim_list->at(idim);
      //RoadData*   rd = &road_list_2->at(idim); // Use 0, 1 or 2.

      double trk_sep      = dd->pos_pos.Z() - dd->pos_neg.Z();
      double chi2_tgt_pos = dd->chisq_target_pos;
      double chi2_dum_pos = dd->chisq_dump_pos;
      double chi2_ups_pos = dd->chisq_upstream_pos;
      double chi2_tgt_neg = dd->chisq_target_neg;
      double chi2_dum_neg = dd->chisq_dump_neg;
      double chi2_ups_neg = dd->chisq_upstream_neg;

      if (dd->pos    .Z() < -690 ||
          dd->pos_pos.Z() < -690 || dd->pos_neg.Z() < -690 ||
          dd->mom_pos.Z() <    5 || dd->mom_neg.Z() <    5   ) continue;
      //if (dd->n_hits_pos < 14 || dd->n_hits_neg < 14) continue;
      //if (dd->n_hits_pos < 15 || dd->n_hits_neg < 15) continue;
      if (fabs(trk_sep) > 200) continue;

      if (road_match) {
        bool top_bot = dd->pos_top && dd->neg_bot;
        bool bot_top = dd->pos_bot && dd->neg_top;
        //bool top_bot = (rd->pos_top.size() > 0 && rd->neg_bot.size() > 0);
        //bool bot_top = (rd->pos_bot.size() > 0 && rd->neg_top.size() > 0);
        if (!top_bot && !bot_top) continue;
      }

      if (chi2_tgt_pos < 0 || chi2_dum_pos < 0 || chi2_ups_pos < 0 ||
          chi2_tgt_pos - chi2_dum_pos > 0 || chi2_tgt_pos - chi2_ups_pos > 0) continue;
      if (chi2_tgt_neg < 0 || chi2_dum_neg < 0 || chi2_ups_neg < 0 ||
          chi2_tgt_neg - chi2_dum_neg > 0 || chi2_tgt_neg - chi2_ups_neg > 0) continue;

      h1_nhit_pos->Fill(dd->n_hits_pos);
      h1_chi2_pos->Fill(dd->chisq_pos);
      h1_z_pos   ->Fill(dd->pos_pos.Z());
      h1_px_pos  ->Fill(dd->mom_pos.X());
      h1_py_pos  ->Fill(dd->mom_pos.Y());
      h1_pz_pos  ->Fill(dd->mom_pos.Z());

      h1_nhit_neg->Fill(dd->n_hits_neg);
      h1_chi2_neg->Fill(dd->chisq_neg);
      h1_z_neg   ->Fill(dd->pos_neg.Z());
      h1_px_neg  ->Fill(dd->mom_neg.X());
      h1_py_neg  ->Fill(dd->mom_neg.Y());
      h1_pz_neg  ->Fill(dd->mom_neg.Z());
      
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

  h1_D1 ->Draw();  c1->SaveAs((dir_out+"/h1_D1.png").c_str());
  h1_D2 ->Draw();  c1->SaveAs((dir_out+"/h1_D2.png").c_str());
  h1_D3p->Draw();  c1->SaveAs((dir_out+"/h1_D3p.png").c_str());
  h1_D3m->Draw();  c1->SaveAs((dir_out+"/h1_D3m.png").c_str());
  
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

void AnaDimuon::CheckRoadList(const std::vector<int>& list_road, const int road, const char* name)
{
  if (find(list_road.begin(), list_road.end(), road) == list_road.end()) {
    int h1, h2, h3, h4, tb;
    UtilTrigger::Road2Hodo(road, h1, h2, h3, h4, tb);
    cout << "  road " << road << " (" << h1 << " " << h2 << " " << h3 << " " << h4 << ") is missed in " << name << ".\n";
    for (auto it = list_road.begin(); it != list_road.end(); it++) {
      UtilTrigger::Road2Hodo(*it, h1, h2, h3, h4, tb);
      cout << "    " << *it << " (" << h1 << " " << h2 << " " << h3 << " " << h4 << ")\n";
    }
  }
}

std::vector<const UtilTrigger::TrigRoad*> AnaDimuon::FindRoads(const UtilTrigger::TrigRoads* list_road_all, const std::vector<int> list_road_id) const
{
  vector<const UtilTrigger::TrigRoad*> list_road_ret;
  for (auto it = list_road_id.begin(); it != list_road_id.end(); it++) {
    const UtilTrigger::TrigRoad* rd = list_road_all->FindRoad(*it);
    if (rd) list_road_ret.push_back(rd);
  }
  return list_road_ret;
}

std::vector<int> AnaDimuon::FindRoadIDs(const UtilTrigger::TrigRoads* list_road_all, const std::vector<int> list_road_id) const
{
  vector<const UtilTrigger::TrigRoad*> list_road = FindRoads(list_road_all, list_road_id);
  vector<int> list_road_id_ret;
  for (auto it = list_road.begin(); it != list_road.end(); it++) list_road_id_ret.push_back((*it)->road_id);
  return list_road_id_ret;
}
