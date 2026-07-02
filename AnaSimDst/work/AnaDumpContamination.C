/// AnaDumpContamination.C:  ROOT macro to evaluate the amount of dump dimuons in a sample of target-like dimuons.
R__LOAD_LIBRARY(libana_sim_dst)
using namespace std;

void AnaDumpContamination(const char* fname="sim_tree.root", const char* tname="tree")
{
  TFile* file = new TFile(fname);
  TTree* tree = (TTree*)file->Get(tname);

  EventData*      event         = new EventData();
  DimuonList*     list_dim_true = new DimuonList();
  RecoDimuonList* list_dim_reco = new RecoDimuonList();
  tree->SetBranchAddress("evt"     , &event);
  tree->SetBranchAddress("dim_true", &list_dim_true);
  tree->SetBranchAddress("dim_reco", &list_dim_reco);

  TH1* h1_x_true_all = new TH1D("h1_x_true_all", ";Dimuon x_{true} (cm);Yield", 100, -5, 5);
  TH1* h1_y_true_all = new TH1D("h1_y_true_all", ";Dimuon y_{true} (cm);Yield", 100, -5, 5);
  TH1* h1_z_true_all = new TH1D("h1_z_true_all", ";Dimuon z_{true} (cm);Yield", 120, -600, 600);
  TH1* h1_m_true_all = new TH1D("h1_m_true_all", ";Mass_{true} (GeV);Yield", 60, 0, 6);
  TH1* h1_x_true     = new TH1D("h1_x_true"    , ";Dimuon x_{true} (cm);Yield", 100, -5, 5);
  TH1* h1_y_true     = new TH1D("h1_y_true"    , ";Dimuon y_{true} (cm);Yield", 100, -5, 5);
  TH1* h1_z_true     = new TH1D("h1_z_true"    , ";Dimuon z_{true} (cm);Yield", 120, -600, 600);
  TH1* h1_m_true     = new TH1D("h1_m_true"    , ";Mass_{true} (GeV);Yield", 60, 0, 6);
  TH1* h1_m_reco     = new TH1D("h1_m_reco"    , ";Mass_{reco} (GeV);Yield", 60, 0, 6);
  TH1* h1_m_diff     = new TH1D("h1_m_diff"    , ";Mass_{reco} #minus Mass_{true} (GeV);Yield", 40, -2, 2);

  TH1* h1_x_true_dump = new TH1D("h1_x_true_dump", ";Dimuon x_{true} (cm);Yield", 100, -5, 5);
  TH1* h1_y_true_dump = new TH1D("h1_y_true_dump", ";Dimuon y_{true} (cm);Yield", 100, -5, 5);
  TH1* h1_m_reco_dump = new TH1D("h1_m_reco_dump", ";Mass_{reco} (GeV);Yield", 60, 0, 6);  
  //TH1* h1_x_st1_pos = new TH1D("h1_x_st1_pos", ";x of #mu^{+} at St.1 (cm);Yield", 70, -50, 20);
  //TH1* h1_x_st1_neg = new TH1D("h1_x_st1_neg", ";x of #mu^{#minus} at St.1 (cm);Yield", 70, -20, 50);
  //TH2* h2_x_st1_pos_m = new TH2D("h2_x_st1_pos_m", ";x of #mu^{+} at St.1 (cm);Mass (GeV);Yield", 70, -50, 20,  5, 1, 6);
  //TH2* h2_x_st1_neg_m = new TH2D("h2_x_st1_neg_m", ";x of #mu^{#minus} at St.1 (cm);Mass (GeV);Yield", 70, -20, 50,  5, 1, 6);
  
  int n_ent = tree->GetEntries();
  cout << "N of entries = " << n_ent << endl;
  for (int i_ent = 0; i_ent < n_ent; i_ent++) {
    if ((i_ent+1) % (n_ent/10) == 0) cout << "  " << 100*(i_ent+1)/n_ent << "%" << flush;
    tree->GetEntry(i_ent);
    double weight = event->weight;

    for (int i_dim = 0; i_dim < list_dim_true->size(); i_dim++) {
      DimuonData* dd_true = &list_dim_true->at(i_dim);
      h1_x_true_all->Fill(dd_true->pos.X(), weight);
      h1_y_true_all->Fill(dd_true->pos.Y(), weight);
      h1_z_true_all->Fill(dd_true->pos.Z(), weight);
      h1_m_true_all->Fill(dd_true->mom.M(), weight);
    }
    
    int n_dim = list_dim_reco->size();
    //cout << "n_dim = " << n_dim << " vs " << list_dim_reco->size() << "\n";
    for (int i_dim = 0; i_dim < n_dim; i_dim++) {
      RecoDimuonData* dd_reco = &list_dim_reco->at(i_dim);
      bool TB = dd_reco->pos_top && dd_reco->neg_bot;
      bool BT = dd_reco->pos_bot && dd_reco->neg_top;
      if (!TB && !BT) continue;

      RecoTrackData* trk_pos = &dd_reco->trk_pos;
      RecoTrackData* trk_neg = &dd_reco->trk_neg;
      
      double z_pos = trk_pos->pos_vtx.Z();
      double z_neg = trk_neg->pos_vtx.Z();
      if (z_pos < -600 || z_neg < -600) continue;
      ////if (fabs(z1-z2)>200) return false;
  
      if (fabs(trk_pos->pos_st1.Y()) < 3 ||
          fabs(trk_neg->pos_st1.Y()) < 3   ) continue;

      double pos_chisq_t  = trk_pos->chisq_target;
      double pos_chisq_d  = trk_pos->chisq_dump;
      double pos_chisq_us = trk_pos->chisq_upstream;
      double neg_chisq_t  = trk_neg->chisq_target;
      double neg_chisq_d  = trk_neg->chisq_dump;
      double neg_chisq_us = trk_neg->chisq_upstream;
      if (pos_chisq_t < 0 || pos_chisq_t < 0) continue;
      if (pos_chisq_d - pos_chisq_t < 0 || pos_chisq_us - pos_chisq_t < 0 ||
          neg_chisq_d - neg_chisq_t < 0 || neg_chisq_us - neg_chisq_t < 0   ) continue;

      double py_st1_pos = trk_pos->mom_st1.Y();
      double py_st1_neg = trk_neg->mom_st1.Y();
      if (py_st1_pos * py_st1_neg > 0) continue;

      //double x_st1_pos = trk_pos->pos_st1.X();
      //double x_st1_neg = trk_neg->pos_st1.X();
      //if (x_st1_pos > 25 || x_st1_neg > 25) continue;

      DimuonData* dd_true = &list_dim_true->at(i_dim);
      //if (dd_true->mom.M() > 3.0) continue;
      //if (dd_true->pos.Z() < 0) continue;

      double x_true = dd_true->pos.X();
      double y_true = dd_true->pos.Y();
      double z_true = dd_true->pos.Z();
      double m_true = dd_true->mom.M();
      double m_reco = dd_reco->mom_target.M();
      h1_x_true->Fill(x_true, weight);
      h1_y_true->Fill(y_true, weight);
      h1_z_true->Fill(z_true, weight);
      h1_m_true->Fill(m_true, weight);
      h1_m_reco->Fill(m_reco, weight);
      h1_m_diff->Fill(m_reco - m_true, weight);
      if (z_true > 0) {
        h1_x_true_dump->Fill(x_true, weight);
        h1_y_true_dump->Fill(y_true, weight);
        h1_m_reco_dump->Fill(m_reco, weight);
      }
      //h1_x_st1_pos->Fill(trk_pos->pos_st1.X(), weight);
      //h1_x_st1_neg->Fill(trk_neg->pos_st1.X(), weight);
      //h2_x_st1_pos_m->Fill(trk_pos->pos_st1.X(), dd_reco->mom.M(), weight);
      //h2_x_st1_neg_m->Fill(trk_neg->pos_st1.X(), dd_reco->mom.M(), weight);
    }
  }

  gSystem->mkdir("result", true);
  TCanvas* c1 = new TCanvas("c1", "");
  c1->SetGrid();

  h1_m_true_all->Draw("E1");  c1->SaveAs("result/h1_m_true_all.png");
  h1_m_true    ->Draw("E1");  c1->SaveAs("result/h1_m_true.png");
  h1_m_reco    ->Draw("E1");  c1->SaveAs("result/h1_m_reco.png");
  h1_m_diff    ->Draw("E1");  c1->SaveAs("result/h1_m_diff.png");

  //h1_m_reco->SetLineWidth(2);
  h1_m_reco_dump->SetLineColor(kRed);
  h1_m_reco->Draw("E1");
  h1_m_reco_dump->Draw("E1same");
  c1->SaveAs("result/h1_m_reco_dump.png");
  
  c1->SetLogy(true);
  h1_x_true_all->Draw("E1");  c1->SaveAs("result/h1_x_true_all.png");
  h1_y_true_all->Draw("E1");  c1->SaveAs("result/h1_y_true_all.png");
  h1_z_true_all->Draw("E1");  c1->SaveAs("result/h1_z_true_all.png");
  h1_x_true    ->Draw("E1");  c1->SaveAs("result/h1_x_true.png");
  h1_y_true    ->Draw("E1");  c1->SaveAs("result/h1_y_true.png");
  h1_z_true    ->Draw("E1");  c1->SaveAs("result/h1_z_true.png");

  h1_x_true->SetLineColor(kRed);
  h1_x_true->Draw("E1");
  h1_x_true_dump->Draw("E1same");
  c1->SaveAs("result/h1_x_true_dump.png");
  
  h1_y_true->SetLineColor(kRed);
  h1_y_true->Draw("E1");
  h1_y_true_dump->Draw("E1same");
  c1->SaveAs("result/h1_y_true_dump.png");
  
  //h1_x_st1_pos->Draw();  c1->SaveAs("result/h1_x_st1_pos.png");  
  //h1_x_st1_neg->Draw();  c1->SaveAs("result/h1_x_st1_neg.png");  
  //gStyle->SetOptStat(0);
  //h2_x_st1_pos_m->Draw("colz");  c1->SaveAs("result/h2_x_st1_pos_m.png");  
  //h2_x_st1_neg_m->Draw("colz");  c1->SaveAs("result/h2_x_st1_neg_m.png");  

  exit(0);
}
