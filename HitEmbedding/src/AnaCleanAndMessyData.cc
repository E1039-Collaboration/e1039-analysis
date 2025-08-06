#include <iostream>
#include <iomanip>
#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TEfficiency.h>
#include <TCanvas.h>
#include "AnaCleanAndMessyData.h"
using namespace std;

AnaCleanAndMessyData::AnaCleanAndMessyData()
  : m_req_fpga1(false)
  , m_verb(0)
  , m_cl_file(0)
  , m_cl_tree(0)
  , m_me_file(0)
  , m_me_tree(0)
{
  ;
}

AnaCleanAndMessyData::~AnaCleanAndMessyData()
{
  ;
}

void AnaCleanAndMessyData::Init(const char* fn_clean, const char* fn_messy)
{
  m_cl_file = new TFile(fn_clean);
  m_cl_tree = (TTree*)m_cl_file->Get("tree");
  m_cl_evt      = new EventData ();
  m_cl_occ      = new OccData   ();
  m_cl_trk_true = new TrackList ();
  m_cl_trk_reco = new TrackList ();
  m_cl_dim_true = new DimuonList();
  m_cl_dim_reco = new DimuonList();
  m_cl_tree->SetBranchAddress("evt"     , &m_cl_evt);
  m_cl_tree->SetBranchAddress("occ"     , &m_cl_occ);
  m_cl_tree->SetBranchAddress("trk_true", &m_cl_trk_true);
  m_cl_tree->SetBranchAddress("trk_reco", &m_cl_trk_reco);
  m_cl_tree->SetBranchAddress("dim_true", &m_cl_dim_true);
  m_cl_tree->SetBranchAddress("dim_reco", &m_cl_dim_reco);

  m_me_file = new TFile(fn_messy);
  m_me_tree = (TTree*)m_me_file->Get("tree");
  m_me_evt      = new EventData ();
  m_me_occ      = new OccData   ();
  m_me_trk_true = new TrackList ();
  m_me_trk_reco = new TrackList ();
  m_me_dim_true = new DimuonList();
  m_me_dim_reco = new DimuonList();
  m_me_tree->SetBranchAddress("evt"     , &m_me_evt);
  m_me_tree->SetBranchAddress("occ"     , &m_me_occ);
  m_me_tree->SetBranchAddress("trk_true", &m_me_trk_true);
  m_me_tree->SetBranchAddress("trk_reco", &m_me_trk_reco);
  m_me_tree->SetBranchAddress("dim_true", &m_me_dim_true);
  m_me_tree->SetBranchAddress("dim_reco", &m_me_dim_reco);

  gSystem->mkdir("result", true);
  m_out_file = new TFile("result/output.root", "RECREATE");
  m_h1_rfp00 = new TH1D("h1_rfp00", ";RF+00;", 100, 0, 1000);
  m_h1_D1    = new TH1D("h1_D1"   , ";D1;" , 100, 0, 800);
  m_h1_D2    = new TH1D("h1_D2"   , ";D2;" , 100, 0, 500);
  m_h1_D3p   = new TH1D("h1_D3p"  , ";D3p;", 100, 0, 500);
  m_h1_D3m   = new TH1D("h1_D3m"  , ";D3m;", 100, 0, 500);
  m_h1_trk_pos_cl = new TH1D("h1_trk_pos_cl", ";D1;", 40, 0, 400);
  m_h1_trk_pos_me = new TH1D("h1_trk_pos_me", ";D1;", 40, 0, 400);
  m_h1_trk_neg_cl = new TH1D("h1_trk_neg_cl", ";D1;", 40, 0, 400);
  m_h1_trk_neg_me = new TH1D("h1_trk_neg_me", ";D1;", 40, 0, 400);
  m_h1_dim_cl     = new TH1D("h1_dim_cl"    , ";D1;", 40, 0, 400);
  m_h1_dim_me     = new TH1D("h1_dim_me"    , ";D1;", 40, 0, 400);

  m_h2_ndim = new TH2D("h2_ndim", ";N of clean dimuons; N of messy dimuons", 5, -0.5, 4.5,  5, -0.5, 4.5);
  
  m_h1_mass_cl   = new TH1D("h1_mass_cl"  , ";Dimuon mass (GeV);Yield", 120, 1, 7);
  m_h1_mass_me   = new TH1D("h1_mass_me"  , ";Dimuon mass (GeV);Yield", 120, 1, 7);
  m_h1_mass_diff = new TH1D("h1_mass_diff", ";Messy - Clean of dimuon mass (GeV);Yield", 100, -0.25, 0.25);
  //m_h1_pz_cl;
  //m_h1_pz_me;
  //m_h1_pz_diff;
  //m_h1_xF_cl;
  //m_h1_xF_me;
  //m_h1_xF_diff;
}

/// Function to analyze a pair of non-embedded and embedded (i.e. clean and messy) data.
/**
 * The main part of this function is to match the event between the two data.
 */
void AnaCleanAndMessyData::Analyze()
{
  int n_cl_evt = m_cl_tree->GetEntries();
  int n_me_evt = m_me_tree->GetEntries();
  int i_cl_evt = 0;
  int i_me_evt = 0;

  bool no_event = false;
  while (! no_event) {
    if (i_cl_evt >= n_cl_evt || i_me_evt >= n_me_evt) return;
    m_cl_tree->GetEntry(i_cl_evt);
    m_me_tree->GetEntry(i_me_evt);
    pair<int, int> job_evt_cl(m_cl_evt->job_id, m_cl_evt->event_id);
    pair<int, int> job_evt_me(m_me_evt->job_id, m_me_evt->event_id);

    while (job_evt_cl != job_evt_me) { // job+event IDs are different
      if (job_evt_cl < job_evt_me) {
        i_cl_evt++;
        if (i_cl_evt >= n_cl_evt) return;
        m_cl_tree->GetEntry(i_cl_evt);
        job_evt_cl = pair<int, int>(m_cl_evt->job_id, m_cl_evt->event_id);
      } else { // >
        i_me_evt++;
        if (i_me_evt >= n_me_evt) return;
        m_me_tree->GetEntry(i_me_evt);
        job_evt_me = pair<int, int>(m_me_evt->job_id, m_me_evt->event_id);
      }
    }

    if (Verbosity() > 9) {
      cout << "AnaCleanAndMessyData::Analyze():  Job ID " << m_cl_evt->job_id << ", Event ID " << m_cl_evt->event_id << ": Clean " << i_cl_evt << "/" << n_cl_evt << ", Messy " << i_me_evt << "/" << n_me_evt << endl;
    }
    AnalyzeEvent();
    i_cl_evt++;
    i_me_evt++;
  }
}

void AnaCleanAndMessyData::End()
{
  if (m_cl_file) m_cl_file->Close();
  if (m_me_file) m_me_file->Close();
  if (m_out_file) DrawAndWriteOutput();
}

/// Function to analyze one event.
/**
 * This function should be modified as your analysis needs.
 */
void AnaCleanAndMessyData::AnalyzeEvent()
{
  double ww = m_cl_evt->weight;
  int rfp01 = m_me_evt->rfp01;
  int rfp00 = m_me_evt->rfp00;
  int rfm01 = m_me_evt->rfm01;
  int n_h1x = m_me_evt->n_h1x;
  int n_h2x = m_me_evt->n_h2x;
  int n_h3x = m_me_evt->n_h3x;
  int n_h4x = m_me_evt->n_h4x;
  int D1    = m_me_occ->D1;
  int D2    = m_me_occ->D2;
  int D3p   = m_me_occ->D3p;
  int D3m   = m_me_occ->D3m;
  
  int n_trk_pos_cl = 0;
  int n_trk_neg_cl = 0;
  for (int ii = 0; ii < m_cl_trk_reco->size(); ii++) {
    TrackData* td = &m_cl_trk_reco->at(ii);
    if (td->charge > 0) n_trk_pos_cl++;
    else                n_trk_neg_cl++;
  }

  int n_trk_pos_me = 0;
  int n_trk_neg_me = 0;
  for (int ii = 0; ii < m_me_trk_reco->size(); ii++) {
    TrackData* td = &m_me_trk_reco->at(ii);
    if (td->charge > 0) n_trk_pos_me++;
    else                n_trk_neg_me++;
  }

  m_h1_rfp00->Fill(rfp00, ww);
  m_h1_D1   ->Fill(D1,    ww);
  m_h1_D2   ->Fill(D2,    ww);
  m_h1_D3p  ->Fill(D3p,   ww);
  m_h1_D3m  ->Fill(D3m,   ww);
  
  //m_h1_trk_pos_cl->Fill(D1, n_trk_pos_cl);
  //m_h1_trk_pos_me->Fill(D1, n_trk_pos_me);
  //m_h1_trk_neg_cl->Fill(D1, n_trk_neg_cl);
  //m_h1_trk_neg_me->Fill(D1, n_trk_neg_me);
  
  int n_dim_cl = m_cl_dim_reco->size();
  int n_dim_me = m_me_dim_reco->size();
  m_h2_ndim ->Fill(n_dim_cl, n_dim_me);

  for (int i_cl = 0; i_cl < n_dim_cl; i_cl++) {
    DimuonData* dd_cl = &m_cl_dim_reco->at(i_cl);
    bool fpga1_cl = dd_cl->pos_top && dd_cl->neg_bot || dd_cl->pos_bot && dd_cl->neg_top;
    double mass_cl = dd_cl->mom.M();
    if (m_req_fpga1 && !fpga1_cl) continue;

    int i_me_match = -1;
    double mass_me_match = -1;
    for (int i_me = 0; i_me < n_dim_me; i_me++) {
      DimuonData* dd_me = &m_me_dim_reco->at(i_me);
      bool fpga1_me = dd_me->pos_top && dd_me->neg_bot || dd_me->pos_bot && dd_me->neg_top;
      if (m_req_fpga1 && !fpga1_me) continue;
      
      double mass_me = dd_me->mom.M();
      if (i_me_match < 0 ||
          fabs(mass_me - mass_cl) < fabs(mass_me_match - mass_cl)) {
        i_me_match = i_me;
        mass_me_match = mass_me;
      }
    }
    if (i_me_match >= 0) {
      DimuonData* dd_me = &m_me_dim_reco->at(i_me_match);
      double mass_me = dd_me->mom.M();
      //double   pz_me = dd_me->mom.Z();
      //double   xF_me = dd_me->xF;
      m_h1_mass_cl  ->Fill(mass_cl, ww);
      m_h1_mass_me  ->Fill(mass_me, ww);
      m_h1_mass_diff->Fill(mass_me - mass_cl, ww);
      if (fabs(mass_me - mass_cl) < 0.05) m_h1_dim_me->Fill(D1);
    }
    m_h1_dim_cl->Fill(D1);
  }
}

/// Function to be called in End() to make, draw and write output objects.
void AnaCleanAndMessyData::DrawAndWriteOutput()
{
  m_out_file->cd();
  m_out_file->Write();

  TCanvas* c1 = new TCanvas("c1", "");
  c1->SetGrid();

  //TEfficiency* teff_trk_pos = new TEfficiency(*m_h1_trk_pos_me, *m_h1_trk_pos_cl);
  //TEfficiency* teff_trk_neg = new TEfficiency(*m_h1_trk_neg_me, *m_h1_trk_neg_cl);
  //teff_trk_pos->SetName("teff_trk_pos");
  //teff_trk_neg->SetName("teff_trk_neg");
  //teff_trk_pos->SetTitle(";RF+00;Reco. efficiency of #mu^{#plus}");
  //teff_trk_neg->SetTitle(";RF+00;Reco. efficiency of #mu^{#minus}");
  //teff_trk_pos->Draw();
  //c1->SaveAs("result/teff_trk_pos.png");
  //teff_trk_neg->Draw();
  //c1->SaveAs("result/teff_trk_neg.png");

  TEfficiency* teff_dim = new TEfficiency(*m_h1_dim_me, *m_h1_dim_cl);
  teff_dim->SetName("teff_dim");
  teff_dim->SetTitle(";D1;Messy / Clean of dimuon");
  teff_dim->Draw();
  c1->SaveAs("result/teff_dim.png");

  m_h2_ndim->Draw("colz");  c1->SaveAs("result/h2_ndim.png");
  
  m_h1_rfp00->Draw("E1");  c1->SaveAs("result/h1_rfp00.png");
  m_h1_D1   ->Draw("E1");  c1->SaveAs("result/h1_D1.png");
  m_h1_D2   ->Draw("E1");  c1->SaveAs("result/h1_D2.png");
  m_h1_D3p  ->Draw("E1");  c1->SaveAs("result/h1_D3p.png");
  m_h1_D3m  ->Draw("E1");  c1->SaveAs("result/h1_D3m.png");


  m_h1_mass_cl->SetLineColor(kRed);
  m_h1_mass_cl->Draw("E1");
  m_h1_mass_me->Draw("E1same");
  c1->SaveAs("result/h1_mass.png");

  m_h1_mass_diff->Draw("E1");
  c1->SaveAs("result/h1_mass_diff.png");

  delete c1;
  //teff_trk_pos->Write();
  //teff_trk_neg->Write();
  teff_dim    ->Write();
  m_out_file->Close();
}
