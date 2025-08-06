R__LOAD_LIBRARY(libana_embedding)
using namespace std;
TFile* file;
TTree* tree;
TCanvas* c1;
void Draw1D(const string name, const string var, const string cut, const string title_x, const int n_x, const double x_lo, const double x_hi);

void AnaOneData(const bool req_fpga1=true, const char* fname="ana_tree.root", const char* tname="tree")
{
  TH1* h1_n_trk_true    = new TH1D("h1_n_trk_true"   , ";N of tracks;Yield" , 10, -0.5, 9.5);
  TH1* h1_n_dim_true    = new TH1D("h1_n_dim_true"   , ";N of dimuons;Yield" , 5, -0.5, 4.5);
  //TH1* h1_trk_px_true   = new TH1D("h1_trk_px_true"  , ";p_{x} of tracks;Yield", 50, -5, 5);
  //TH1* h1_trk_py_true   = new TH1D("h1_trk_py_true"  , ";p_{y} of tracks;Yield", 50, -5, 5);
  //TH1* h1_trk_pz_true   = new TH1D("h1_trk_pz_true"  , ";p_{z} of tracks;Yield", 45, 10, 100);
  TH1* h1_dim_mass_true = new TH1D("h1_dim_mass_true", ";Mass of dimuons;Yield", 60, 1, 7);
  TH1* h1_dim_pz_true   = new TH1D("h1_dim_pz_true"  , ";p_{z} of dimuons;Yield", 45, 30, 120);
  TH1* h1_dim_xF_true   = new TH1D("h1_dim_xF_true"  , ";x_{F} of dimuons;Yield", 75, -0.5, 1.0);

  TH1* h1_n_trk_reco    = new TH1D("h1_n_trk_reco"   , ";N of tracks;Yield" , 10, -0.5, 9.5);
  TH1* h1_n_dim_reco    = new TH1D("h1_n_dim_reco"   , ";N of dimuons;Yield" , 5, -0.5, 4.5);
  TH1* h1_trk_nhit_reco = new TH1D("h1_trk_nhit_reco"  , ";N of hits/track;Yield", 10, 10.5, 20.5);
  TH1* h1_trk_chi2_reco = new TH1D("h1_trk_chi2_reco"  , ";#chi^{2} of track;Yield", 100, 0, 20);
  //TH1* h1_trk_px_reco   = new TH1D("h1_trk_px_reco"  , ";p_{x} of tracks;Yield", 50, -5, 5);
  //TH1* h1_trk_py_reco   = new TH1D("h1_trk_py_reco"  , ";p_{y} of tracks;Yield", 50, -5, 5);
  //TH1* h1_trk_pz_reco   = new TH1D("h1_trk_pz_reco"  , ";p_{z} of tracks;Yield", 45, 10, 100);
  TH1* h1_dim_mass_reco = new TH1D("h1_dim_mass_reco", ";Mass of dimuons;Yield", 60, 1, 7);
  TH1* h1_dim_pz_reco   = new TH1D("h1_dim_pz_reco"  , ";p_{z} of dimuons;Yield", 45, 30, 120);
  TH1* h1_dim_xF_reco   = new TH1D("h1_dim_xF_reco"  , ";x_{F} of dimuons;Yield", 75, -0.5, 1.0);

  //TH1* h1_trk_px_diff   = new TH1D("h1_trk_px_diff"  , ";p_{x} of diff tracks;Yield", 50, -5, 5);
  //TH1* h1_trk_py_diff   = new TH1D("h1_trk_py_diff"  , ";p_{y} of diff tracks;Yield", 50, -5, 5);
  //TH1* h1_trk_pz_diff   = new TH1D("h1_trk_pz_diff"  , ";p_{z} of diff tracks;Yield", 45, 10, 100);
  TH1* h1_dim_mass_diff = new TH1D("h1_dim_mass_diff", ";Reco - True of Mass;Yield",50, -2.5, 2.5);
  TH1* h1_dim_pz_diff   = new TH1D("h1_dim_pz_diff"  , ";Reco - True of p_{z};Yield", 50, -10, 10);
  TH1* h1_dim_xF_diff   = new TH1D("h1_dim_xF_diff"  , ";Reco - True of x_{F};Yield", 50, -0.25, 0.25);
  
  file = new TFile(fname);
  tree = (TTree*)file->Get(tname);
  EventData* evt = 0;
  TrackList*  trk_true = 0;
  DimuonList* dim_true = 0;
  TrackList*  trk_reco = 0;
  DimuonList* dim_reco = 0;
  tree->SetBranchAddress("evt"     , &evt);
  tree->SetBranchAddress("trk_true", &trk_true);
  tree->SetBranchAddress("dim_true", &dim_true);
  tree->SetBranchAddress("trk_reco", &trk_reco);
  tree->SetBranchAddress("dim_reco", &dim_reco);

  for (int i_ent = 0; i_ent < tree->GetEntries(); i_ent++) {
    tree->GetEntry(i_ent);
    double weight  = evt->weight;
    h1_n_trk_true->Fill(trk_true->size(), weight);
    h1_n_trk_reco->Fill(trk_reco->size(), weight);

    for (auto it = trk_reco->begin(); it != trk_reco->end(); it++) {
      TrackData* td = &*it;
      h1_trk_nhit_reco->Fill(td->n_hits);
      h1_trk_chi2_reco->Fill(td->chi2  );
    }
    
    int n_dim_true = dim_true->size();
    int n_dim_reco = dim_reco->size();
    h1_n_dim_true->Fill(n_dim_true, weight);
    h1_n_dim_reco->Fill(n_dim_reco, weight);

    for (int i_true = 0; i_true < n_dim_true; i_true++) {
      DimuonData* dd_true = &dim_true->at(i_true);
      double mass_true = dd_true->mom.M();
      double   pz_true = dd_true->mom.Z();
      double   xF_true = dd_true->xF;

      int i_reco_match = -1;
      double mass_reco_match = -1;
      for (int i_reco = 0; i_reco < n_dim_reco; i_reco++) {
        DimuonData* dd_reco = &dim_reco->at(i_reco);
        bool fpga1 = dd_reco->pos_top && dd_reco->neg_bot || dd_reco->pos_bot && dd_reco->neg_top;
        if (req_fpga1 && ! fpga1) continue;
        double mass_reco = dd_reco->mom.M();
        if (i_reco_match < 0 ||
            fabs(mass_reco - mass_true) < fabs(mass_reco_match - mass_true)) {
          i_reco_match = i_reco;
          mass_reco_match = mass_reco;
        }
      }
      if (i_reco_match >= 0) {
        DimuonData* dd_reco = &dim_reco->at(i_reco_match);
        double mass_reco = dd_reco->mom.M();
        double   pz_reco = dd_reco->mom.Z();
        double   xF_reco = dd_reco->xF;
        h1_dim_mass_true->Fill(mass_true, weight);
        h1_dim_pz_true  ->Fill(  pz_true, weight);
        h1_dim_xF_true  ->Fill(  xF_true, weight);
        h1_dim_mass_reco->Fill(mass_reco, weight);
        h1_dim_pz_reco  ->Fill(  pz_reco, weight);
        h1_dim_xF_reco  ->Fill(  xF_reco, weight);
        h1_dim_mass_diff->Fill(mass_reco - mass_true, weight);
        h1_dim_pz_diff  ->Fill(  pz_reco -   pz_true, weight);
        h1_dim_xF_diff  ->Fill(  xF_reco -   xF_true, weight);
      }
    }
  }

  gSystem->mkdir("result", true);
  c1 = new TCanvas("c1", "");
  c1->SetGrid();

  h1_n_trk_true->SetLineColor(kRed);
  h1_n_trk_true->Draw("E1");
  h1_n_trk_reco->Draw("E1same");
  c1->SaveAs("result/h1_n_trk.png");

  h1_n_dim_true->SetLineColor(kRed);
  h1_n_dim_true->Draw("E1");
  h1_n_dim_reco->Draw("E1same");
  c1->SaveAs("result/h1_n_dim.png");

  h1_trk_nhit_reco->Draw();
  c1->SaveAs("result/h1_trk_nhit_reco.png");
  h1_trk_chi2_reco->Draw();
  c1->SaveAs("result/h1_trk_chi2_reco.png");
  
  h1_dim_mass_true->SetLineColor(kRed);
  h1_dim_mass_true->Draw("E1");
  h1_dim_mass_reco->Draw("E1same");
  c1->SaveAs("result/h1_dim_mass.png");

  h1_dim_mass_diff->Draw("E1");
  c1->SaveAs("result/h1_dim_mass_diff.png");
  
  h1_dim_pz_true->SetLineColor(kRed);
  h1_dim_pz_true->Draw("E1");
  h1_dim_pz_reco->Draw("E1same");
  c1->SaveAs("result/h1_dim_pz.png");

  h1_dim_pz_diff->Draw("E1");
  c1->SaveAs("result/h1_dim_pz_diff.png");
  
  h1_dim_xF_true->SetLineColor(kRed);
  h1_dim_xF_true->Draw("E1");
  h1_dim_xF_reco->Draw("E1same");
  c1->SaveAs("result/h1_dim_xF.png");
  
  h1_dim_xF_diff->Draw("E1");
  c1->SaveAs("result/h1_dim_xF_diff.png");

  if (h1_dim_mass_reco->GetRMS() < 0.5) {
    gStyle->SetOptFit(1);
    h1_dim_mass_reco->Draw("E1");
    h1_dim_mass_reco->Fit("gaus");
    c1->SaveAs("result/h1_dim_mass_reco_fit.png");
  }

  exit(0);
}
