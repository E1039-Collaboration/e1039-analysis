/// Fun4SimTree.C:  Macro to analyze true dimuons created by AnaTrueDimuon via Fun4SimMicroDst.C.
#include <UtilAna/UtilDimuon.h>
#include "Common.h"
R__LOAD_LIBRARY(libana_sim_dst)
using namespace std;

void AnaTrueDimuon(const char* fname="sim_tree.root", const char* tname="tree")
{
  TFile* file = new TFile(fname);
  TTree* tree = (TTree*)file->Get(tname);
  gSystem->mkdir("result", true);
  ofstream ofs("result/result.txt");
  
  TFile* file_out = new TFile("result/result.root", "RECREATE");

  TH1* h1_dz_t  = new TH1D("h1_dz_t" ,  ";True dimuon z (cm);", 50, -700, 0);
  TH1* h1_dz_r  = new TH1D("h1_dz_r" , ";Reco. dimuon z (cm);", 50, -700, 0);
  TH1* h1_dz_d  = new TH1D("h1_dz_d" , ";Dimuon z_{reco} - z_{true} (cm);", 50, -200, 200);
  
  TH1* h1_m_t   = new TH1D("h1_m_t"  ,  ";True dimuon mass (GeV);", 40, 1, 5);
  TH1* h1_m_r   = new TH1D("h1_m_r"  , ";Reco. dimuon mass (GeV);", 40, 1, 5);
  TH1* h1_m_d   = new TH1D("h1_m_d"  , ";Dimuon mass_{reco} - mass_{true} (GeV);", 50, -1, 1);
  
  TH1* h1_pT_t  = new TH1D("h1_pT_t"  ,  ";True dimuon pT (GeV);", 50, 0, 2.5);
  TH1* h1_pT_r  = new TH1D("h1_pT_r"  , ";Reco. dimuon pT (GeV);", 50, 0, 2.5);
  TH1* h1_pT_d  = new TH1D("h1_pT_d"  , ";Dimuon pT_{reco} - pT_{true} (GeV);", 50, -1, 1);
  
  TH1* h1_xF_t   = new TH1D("h1_xF_t"  ,  ";True dimuon xF;", 50, 0, 1);
  TH1* h1_xF_r   = new TH1D("h1_xF_r"  , ";Reco. dimuon xF;", 50, 0, 1);
  TH1* h1_xF_d   = new TH1D("h1_xF_d"  , ";Dimuon xF_{reco} - xF_{true};", 50, -0.1, 0.1);
  
  TH1* h1_x1_t   = new TH1D("h1_x1_t"  ,  ";True dimuon x1;", 35, 0.3, 1);
  TH1* h1_x1_r   = new TH1D("h1_x1_r"  , ";Reco. dimuon x1;", 35, 0.3, 1);
  TH1* h1_x1_d   = new TH1D("h1_x1_d"  , ";Dimuon x1_{reco} - x1_{true};", 50, -0.1, 0.1);
  
  TH1* h1_x2_t   = new TH1D("h1_x2_t"  ,  ";True dimuon x2;", 30, 0, 0.3);
  TH1* h1_x2_r   = new TH1D("h1_x2_r"  , ";Reco. dimuon x2;", 30, 0, 0.3);
  TH1* h1_x2_d   = new TH1D("h1_x2_d"  , ";Dimuon x2_{reco} - x2_{true};", 50, -0.05, 0.05);

  EventData* evt = 0;
  DimuonList* dim_list_true = 0;
  RecoDimuonList* dim_list_reco = 0;
  tree->SetBranchAddress("evt"     , &evt);
  tree->SetBranchAddress("dim_true", &dim_list_true);
  tree->SetBranchAddress("dim_reco", &dim_list_reco);

  int n_ent = tree->GetEntries();
  cout << "N of entries = " << n_ent << endl;
  for (int i_ent = 0; i_ent < n_ent; i_ent++) {
    if ((i_ent+1) % (n_ent/10) == 0) cout << "  " << 10*(i_ent+1)/(n_ent/10) << "%" << flush;
    tree->GetEntry(i_ent);
    double weight = evt->weight;
    for (size_t idim = 0; idim < dim_list_true->size(); idim++) {
      DimuonData* dd = &dim_list_true->at(idim);
      double dz   = dd->pos.Z();
      double mass = dd->mom.M();
      double pT   = dd->mom.Pt();
      double xF   = dd->xF;
      double x1   = dd->x1;
      double x2   = dd->x2;
      h1_dz_t->Fill(dz  , weight);
      h1_m_t ->Fill(mass, weight);
      h1_pT_t->Fill(pT  , weight);
      h1_xF_t->Fill(xF  , weight);
      h1_x1_t->Fill(x1  , weight);
      h1_x2_t->Fill(x2  , weight);

      RecoDimuonData* ddr = &dim_list_reco->at(idim);
      TLorentzVector p_pos = ddr->mom_pos_target;
      TLorentzVector p_neg = ddr->mom_neg_target;
      if (p_pos.Z() > 0 && p_neg.Z() > 0) { // If reconstructed
        double mass_r;
        double pT_r;
        double x1_r;
        double x2_r;
        double xF_r;
        UtilDimuon::CalcVar(p_pos, p_neg, mass_r, pT_r, x1_r, x2_r, xF_r);
        double dz_r = ddr->pos.Z();
        h1_dz_r->Fill(dz_r  , weight);
        h1_m_r ->Fill(mass_r, weight);
        h1_pT_r->Fill(pT_r  , weight);
        h1_xF_r->Fill(xF_r  , weight);
        h1_x1_r->Fill(x1_r  , weight);
        h1_x2_r->Fill(x2_r  , weight);
        h1_dz_d->Fill(dz_r   - dz  , weight);
        h1_m_d ->Fill(mass_r - mass, weight);
        h1_pT_d->Fill(pT_r   - pT  , weight);
        h1_xF_d->Fill(xF_r   - xF  , weight);
        h1_x1_d->Fill(x1_r   - x1  , weight);
        h1_x2_d->Fill(x2_r   - x2  , weight);
      }
    }    
  }
  cout << endl;

  double inte_lumi = GetInteLumi();
  ofs  << "Integrated luminosity = " << inte_lumi << endl;

  h1_dz_t->Scale(1/inte_lumi);
  h1_dz_r->Scale(1/inte_lumi);
  h1_dz_d->Scale(1/inte_lumi);
  h1_m_t ->Scale(1/inte_lumi);
  h1_m_r ->Scale(1/inte_lumi);
  h1_m_d ->Scale(1/inte_lumi);
  h1_pT_t->Scale(1/inte_lumi);
  h1_pT_r->Scale(1/inte_lumi);
  h1_pT_d->Scale(1/inte_lumi);
  h1_xF_t->Scale(1/inte_lumi);
  h1_xF_r->Scale(1/inte_lumi);
  h1_xF_d->Scale(1/inte_lumi);
  h1_x1_t->Scale(1/inte_lumi);
  h1_x1_r->Scale(1/inte_lumi);
  h1_x1_d->Scale(1/inte_lumi);
  h1_x2_t->Scale(1/inte_lumi);
  h1_x2_r->Scale(1/inte_lumi);
  h1_x2_d->Scale(1/inte_lumi);
  
  TCanvas* c1 = new TCanvas("c1", "");
  c1->SetGrid();
  //c1->SetLogy(true);

  h1_dz_t->Draw("E1");  c1->SaveAs("result/h1_dz_t.pdf");
  h1_dz_r->Draw("E1");  c1->SaveAs("result/h1_dz_r.pdf");
  h1_dz_d->Draw("E1");  c1->SaveAs("result/h1_dz_d.pdf");
  
  h1_m_t->Draw("E1");  c1->SaveAs("result/h1_m_t.pdf");
  h1_m_r->Draw("E1");  c1->SaveAs("result/h1_m_r.pdf");
  h1_m_d->Draw("E1");  c1->SaveAs("result/h1_m_d.pdf");
  
  h1_pT_t->Draw("E1");  c1->SaveAs("result/h1_pT_t.pdf");
  h1_pT_r->Draw("E1");  c1->SaveAs("result/h1_pT_r.pdf");
  h1_pT_d->Draw("E1");  c1->SaveAs("result/h1_pT_d.pdf");
  
  h1_xF_t->Draw("E1");  c1->SaveAs("result/h1_xF_t.pdf");
  h1_xF_r->Draw("E1");  c1->SaveAs("result/h1_xF_r.pdf");
  h1_xF_d->Draw("E1");  c1->SaveAs("result/h1_xF_d.pdf");
  
  h1_x1_t->Draw("E1");  c1->SaveAs("result/h1_x1_t.pdf");
  h1_x1_r->Draw("E1");  c1->SaveAs("result/h1_x1_r.pdf");
  h1_x1_d->Draw("E1");  c1->SaveAs("result/h1_x1_d.pdf");
  
  h1_x2_t->Draw("E1");  c1->SaveAs("result/h1_x2_t.pdf");
  h1_x2_r->Draw("E1");  c1->SaveAs("result/h1_x2_r.pdf");
  h1_x2_d->Draw("E1");  c1->SaveAs("result/h1_x2_d.pdf");
    
  file_out->Write();
  file_out->Close();
  ofs.close();
  exit(0);
}
