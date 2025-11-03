#include <fstream>
#include <iomanip>
#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TH1D.h>
#include <TH2D.h>
#include <THStack.h>
#include <TCanvas.h>
#include <TLatex.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQHitVector.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQTrackVector.h>
#include <ktracker/SRecEvent.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <geom_svc/GeomSvc.h>
//#include <UtilAna/UtilHist.h>
#include "AnaTrack.h"
using namespace std;

AnaTrack::AnaTrack(const std::string& name)
  : SubsysReco  (name)
  , m_sq_evt    (0)
  , m_sq_hit_vec(0)
  , m_sq_trk_vec(0)
  , m_file_name ("output_track.root")
  , m_file      (0)
  , m_tree      (0)
{
  ;
}

AnaTrack::~AnaTrack()
{
  ;
}

int AnaTrack::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaTrack::InitRun(PHCompositeNode* topNode)
{
  //GeomSvc* geom = GeomSvc::instance();

  m_sq_evt     = findNode::getClass<SQEvent       >(topNode, "SQEvent");
  m_sq_hit_vec = findNode::getClass<SQHitVector   >(topNode, "SQHitVector");
  m_sq_trk_vec = findNode::getClass<SQTrackVector >(topNode, "SQRecTrackVector");
  if (!m_sq_evt || !m_sq_hit_vec || !m_sq_trk_vec) return Fun4AllReturnCodes::ABORTEVENT;

  m_file = new TFile(m_file_name.c_str(), "RECREATE");
  m_tree = new TTree("tree", "Created by AnaTrack");
  m_tree->Branch("event"      , &m_evt);
  m_tree->Branch("track_list", &m_trk_list);

  SQRun* sq_run = findNode::getClass<SQRun>(topNode, "SQRun");
  if (!sq_run) return Fun4AllReturnCodes::ABORTEVENT;

  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaTrack::process_event(PHCompositeNode* topNode)
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

  m_trk_list.clear();
  for (auto it = m_sq_trk_vec->begin(); it != m_sq_trk_vec->end(); it++) {
    SRecTrack* trk = dynamic_cast<SRecTrack*>(*it);
    TrackData td;
    td.charge = trk->get_charge();
    td.road   = trk->getTriggerRoad();
    td.n_hits =  trk->get_num_hits();
    td.chisq  =  trk->get_chisq();
    td.chisq_target   = trk->get_chisq_target();
    td.chisq_upstream = trk->get_chisq_upstream();
    td.chisq_dump     = trk->get_chisq_dump();
    td.pos_vtx = trk->get_pos_vtx();
    td.mom_vtx = trk->get_mom_vtx();
    td.pos_st1 = trk->get_pos_st1();
    td.mom_st1 = trk->get_mom_st1();
    td.pos_st3 = trk->get_pos_st3();
    td.mom_st3 = trk->get_mom_st3();
    m_trk_list.push_back(td);
  }
  
  m_tree->Fill();
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaTrack::End(PHCompositeNode* topNode)
{
  m_file->cd();
  m_file->Write();
  m_file->Close();  
  return Fun4AllReturnCodes::EVENT_OK;
}

void AnaTrack::AnalyzeTree(TChain* tree)
{
  string dir_out = "result/track";
  cout << "N of trees = " << tree->GetNtrees() << endl;
  gSystem->mkdir(dir_out.c_str(), true);
  ofstream ofs(dir_out + "/result.txt");

  TFile* file_out = new TFile((dir_out+"/result.root").c_str(), "RECREATE");
  
  TH2* h2_nhit = new TH2D("h2_nhit", ";N of hits/track;", 6, 12.5, 18.5,  2, -2, 2);
  TH2* h2_chi2 = new TH2D("h2_chi2", ";Track #chi^{2};", 100, 0, 10    ,  2, -2, 2);  
  TH2* h2_chi2_tgt = new TH2D("h2_chi2_tgt", ";Track #chi^{2} at target;"  , 100, 0, 10,  2, -2, 2);
  TH2* h2_chi2_dum = new TH2D("h2_chi2_dum", ";Track #chi^{2} at dump;"    , 100, 0, 10,  2, -2, 2);
  TH2* h2_chi2_ups = new TH2D("h2_chi2_ups", ";Track #chi^{2} at upstream;", 100, 0, 10,  2, -2, 2);

  TH2* h2_x_vtx  = new TH2D("h2_x_vtx" , ";Track x (cm) @ Vertex;Charge;",    100,   -1,   1,  2, -2, 2);
  TH2* h2_y_vtx  = new TH2D("h2_y_vtx" , ";Track y (cm) @ Vertex;Charge;",    100,   -1,   1,  2, -2, 2);
  TH2* h2_z_vtx  = new TH2D("h2_z_vtx" , ";Track z (cm) @ Vertex;Charge;",    120, -700, 500,  2, -2, 2);
  TH2* h2_px_vtx = new TH2D("h2_px_vtx", ";Track p_{x} (GeV) @ Vertex;Charge;", 100, -5,   5,  2, -2, 2);
  TH2* h2_py_vtx = new TH2D("h2_py_vtx", ";Track p_{y} (GeV) @ Vertex;Charge;", 100, -5,   5,  2, -2, 2);
  TH2* h2_pz_vtx = new TH2D("h2_pz_vtx", ";Track p_{z} (GeV) @ Vertex;Charge;", 100,  0, 100,  2, -2, 2);
  TH2* h2_pT_vtx = new TH2D("h2_pT_vtx", ";Track p_{T} (GeV) @ Vertex;Charge;", 100,  0,   5,  2, -2, 2);

  TH2* h2_x_st1  = new TH2D("h2_x_st1" , ";Track x (cm) @ St 1;Charge;",    100,  -50,  50,  2, -2, 2);
  TH2* h2_y_st1  = new TH2D("h2_y_st1" , ";Track y (cm) @ St 1;Charge;",    100,  -50,  50,  2, -2, 2);
  TH2* h2_z_st1  = new TH2D("h2_z_st1" , ";Track z (cm) @ St 1;Charge;",    100,  550, 650,  2, -2, 2);
  TH2* h2_px_st1 = new TH2D("h2_px_st1", ";Track p_{x} (GeV) @ St 1;Charge;", 100, -5,   5,  2, -2, 2);
  TH2* h2_py_st1 = new TH2D("h2_py_st1", ";Track p_{y} (GeV) @ St 1;Charge;", 100, -5,   5,  2, -2, 2);
  TH2* h2_pz_st1 = new TH2D("h2_pz_st1", ";Track p_{z} (GeV) @ St 1;Charge;", 100,  0, 100,  2, -2, 2);

  TH2* h2_x_st3  = new TH2D("h2_x_st3" , ";Track x (cm) @ St 3;Charge;",   100, -150,  150,  2, -2, 2);
  TH2* h2_y_st3  = new TH2D("h2_y_st3" , ";Track y (cm) @ St 3;Charge;",   100, -150,  150,  2, -2, 2);
  TH2* h2_z_st3  = new TH2D("h2_z_st3" , ";Track z (cm) @ St 3;Charge;",   100, 1900, 2000,  2, -2, 2);
  TH2* h2_px_st3 = new TH2D("h2_px_st3", ";Track p_{x} (GeV) @ St 3;Charge;", 100, -5,   5,  2, -2, 2);
  TH2* h2_py_st3 = new TH2D("h2_py_st3", ";Track p_{y} (GeV) @ St 3;Charge;", 100, -5,   5,  2, -2, 2);
  TH2* h2_pz_st3 = new TH2D("h2_pz_st3", ";Track p_{z} (GeV) @ St 3;Charge;", 100,  0, 100,  2, -2, 2);

  //GeomSvc* geom = GeomSvc::instance();
  ostringstream oss;
  
  EventData* evt = 0;
  TrackList* trk_list = 0;
  tree->SetBranchAddress("event"      , &evt);
  tree->SetBranchAddress("track_list", &trk_list);

  int n_ent = tree->GetEntries();
  cout << "N of entries = " << n_ent << endl;
  for (int i_ent = 0; i_ent < n_ent; i_ent++) {
    if (n_ent > 10 && (i_ent+1) % (n_ent/10) == 0) cout << "  " << 10*(i_ent+1)/(n_ent/10) << "%" << flush;
    tree->GetEntry(i_ent);
    
    //if (evt->run_id < 6111 || evt->run_id > 6118) continue; // Range #1
    //if (evt->run_id < 6135 || evt->run_id > 6139) continue; // Range #2
    //if (evt->run_id < 6149 || evt->run_id > 6155) continue; // Range #3

    //if (! (evt->fpga_bits & 0x1)) continue;
    //if (! (evt->fpga_bits & 0x4)) continue;
    if (! (evt->fpga_bits & 0x8)) continue;
    //if (! (evt->nim_bits & 0x1)) continue;

    for (auto it = trk_list->begin(); it != trk_list->end(); it++) {
      TrackData* td = &(*it);
      int charge = td->charge;
      if (abs(charge) != 1) continue;
      //int    road     = td->road;
      int    nhit     = td->n_hits;
      double chi2     = td->chisq;
      double chi2_tgt = td->chisq_target;
      double chi2_dum = td->chisq_dump;
      double chi2_ups = td->chisq_upstream;
      TVector3*       pos_vtx = &td->pos_vtx;
      TLorentzVector* mom_vtx = &td->mom_vtx;
      TVector3*       pos_st1 = &td->pos_st1;
      TLorentzVector* mom_st1 = &td->mom_st1;
      TVector3*       pos_st3 = &td->pos_st3;
      TLorentzVector* mom_st3 = &td->mom_st3;
      if (pos_vtx->Z() < -600 || pos_vtx->Z() >  100 ||
          mom_vtx->X() <   -3 || mom_vtx->X() >    3 ||
          mom_vtx->Z() <    5 || mom_vtx->Z() >  100   ) continue;
      
      h2_nhit    ->Fill(nhit, charge);
      h2_chi2    ->Fill(chi2, charge);
      h2_chi2_tgt->Fill(chi2_tgt, charge);
      h2_chi2_dum->Fill(chi2_dum, charge);
      h2_chi2_ups->Fill(chi2_ups, charge);
      
      h2_x_vtx   ->Fill(pos_vtx->X(), charge);
      h2_y_vtx   ->Fill(pos_vtx->Y(), charge);
      h2_z_vtx   ->Fill(pos_vtx->Z(), charge);
      h2_px_vtx  ->Fill(mom_vtx->X(), charge);
      h2_py_vtx  ->Fill(mom_vtx->Y(), charge);
      h2_pz_vtx  ->Fill(mom_vtx->Z(), charge);
      h2_pT_vtx  ->Fill(mom_vtx->Pt(),charge);

      h2_x_st1   ->Fill(pos_st1->X(), charge);
      h2_y_st1   ->Fill(pos_st1->Y(), charge);
      h2_z_st1   ->Fill(pos_st1->Z(), charge);
      h2_px_st1  ->Fill(mom_st1->X(), charge);
      h2_py_st1  ->Fill(mom_st1->Y(), charge);
      h2_pz_st1  ->Fill(mom_st1->Z(), charge);

      h2_x_st3   ->Fill(pos_st3->X(), charge);
      h2_y_st3   ->Fill(pos_st3->Y(), charge);
      h2_z_st3   ->Fill(pos_st3->Z(), charge);
      h2_px_st3  ->Fill(mom_st3->X(), charge);
      h2_py_st3  ->Fill(mom_st3->Y(), charge);
      h2_pz_st3  ->Fill(mom_st3->Z(), charge);
    }
  }
  
  DrawHistIn1D(h2_nhit, "nhit", dir_out);
  DrawHistIn1D(h2_chi2, "chi2", dir_out);
  DrawHistIn1D(h2_chi2_tgt, "chi2_tgt", dir_out);
  DrawHistIn1D(h2_chi2_dum, "chi2_dum", dir_out);
  DrawHistIn1D(h2_chi2_ups, "chi2_ups", dir_out);

  DrawHistIn1D(h2_x_vtx ,  "x_vtx", dir_out);
  DrawHistIn1D(h2_y_vtx ,  "y_vtx", dir_out);
  DrawHistIn1D(h2_z_vtx ,  "z_vtx", dir_out);
  DrawHistIn1D(h2_px_vtx, "px_vtx", dir_out);
  DrawHistIn1D(h2_py_vtx, "py_vtx", dir_out);
  DrawHistIn1D(h2_pz_vtx, "pz_vtx", dir_out);
  DrawHistIn1D(h2_pT_vtx, "pT_vtx", dir_out);

  DrawHistIn1D(h2_x_st1 ,  "x_st1", dir_out);
  DrawHistIn1D(h2_y_st1 ,  "y_st1", dir_out);
  DrawHistIn1D(h2_z_st1 ,  "z_st1", dir_out);
  DrawHistIn1D(h2_px_st1, "px_st1", dir_out);
  DrawHistIn1D(h2_py_st1, "py_st1", dir_out);
  DrawHistIn1D(h2_pz_st1, "pz_st1", dir_out);

  DrawHistIn1D(h2_x_st3 ,  "x_st3", dir_out);
  DrawHistIn1D(h2_y_st3 ,  "y_st3", dir_out);
  DrawHistIn1D(h2_z_st3 ,  "z_st3", dir_out);
  DrawHistIn1D(h2_px_st3, "px_st3", dir_out);
  DrawHistIn1D(h2_py_st3, "py_st3", dir_out);
  DrawHistIn1D(h2_pz_st3, "pz_st3", dir_out);

  ofs.close();
  file_out->Write();
  file_out->Close();
}

void AnaTrack::DrawHistIn1D(TH2* h2, const std::string label, const std::string dir_out)
{
  TCanvas* c1 = new TCanvas("c1", "");
  c1->SetGrid();
  //c1->SetLogy(true);

  TH1* h1_neg = h2->ProjectionX("h1_neg", 1, 1);
  TH1* h1_pos = h2->ProjectionX("h1_pos", 2, 2);
  h1_pos->SetLineColor(kRed);
  h1_neg->SetLineColor(kBlue);

  ostringstream oss;
  oss << ";" << h2->GetXaxis()->GetTitle() << ";" << h2->GetZaxis()->GetTitle();
  THStack hs("hs", oss.str().c_str());
  hs.Add(h1_pos);
  hs.Add(h1_neg);
  hs.Draw("nostack");

  TLatex tex;
  tex.SetNDC(true);
  tex.SetTextColor(kRed);
  tex.DrawLatex(0.25, 0.94, "Red: #mu^{+}");
  tex.SetTextColor(kBlue);
  tex.DrawLatex(0.75, 0.94, "Blue: #mu^{-}");

  oss.str("");
  oss << dir_out << "/h1_" << label << ".png";
  c1->SaveAs(oss.str().c_str());

  delete h1_neg;
  delete h1_pos;
  delete c1;
}
