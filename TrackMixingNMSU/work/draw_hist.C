using namespace std;

TFile* file;
TTree* tr_org;
TTree* tr_mix;

void DrawHistObj()
{
  TH1* h1_D1_org    = (TH1*)file->Get("h1_D1_org");
  TH1* h1_D1_mix    = (TH1*)file->Get("h1_D1_mix");
  TH1* h1_dD1_mix   = (TH1*)file->Get("h1_dD1_mix");
  TH1* h1_dD2_mix   = (TH1*)file->Get("h1_dD2_mix");
  TH1* h1_dD3p_mix  = (TH1*)file->Get("h1_dD3p_mix");
  TH1* h1_dD3m_mix  = (TH1*)file->Get("h1_dD3m_mix");
  TH1* h1_n_dim_org = (TH1*)file->Get("h1_n_dim_org");
  TH1* h1_n_dim_mix = (TH1*)file->Get("h1_n_dim_mix");

  TCanvas* c1 = new TCanvas("c1", "");
  c1->SetGrid();	

  h1_D1_org   ->Draw();  c1->SaveAs("result/h1_D1_org.png");
  h1_D1_mix   ->Draw();  c1->SaveAs("result/h1_D1_mix.png");
  h1_dD1_mix  ->Draw();  c1->SaveAs("result/h1_dD1_mix.png");
  h1_dD2_mix  ->Draw();  c1->SaveAs("result/h1_dD2_mix.png");
  h1_dD3p_mix ->Draw();  c1->SaveAs("result/h1_dD3p_mix.png");
  h1_dD3m_mix ->Draw();  c1->SaveAs("result/h1_dD3m_mix.png");

  c1->SetLogy(true);
  
  h1_n_dim_org->Draw();  c1->SaveAs("result/h1_n_dim_org.png");
  h1_n_dim_mix->Draw();  c1->SaveAs("result/h1_n_dim_mix.png");

  delete c1;
}

void DrawOrgAndMixIn1D(const char* title, const int n_bin, const double bin_lo, const double bin_hi, const char* var, const char* sel, const char* fname)
{
  TH1* h1_org = new TH1D("h1_org", title, n_bin, bin_lo, bin_hi);
  TH1* h1_mix = new TH1D("h1_mix", title, n_bin, bin_lo, bin_hi);
  tr_org->Project("h1_org", var, sel);
  tr_mix->Project("h1_mix", var, sel);

  TH1* h1_sig = (TH1*)h1_org->Clone("h1_sig");
  h1_sig->Sumw2();
  h1_sig->Add(h1_mix, -1.0);

  gStyle->SetOptStat(0);
  gStyle->SetErrorX(0);
  TCanvas* c1 = new TCanvas("c1", "");
  c1->SetGrid();	

  h1_org->SetLineColor(kBlue);
  h1_org->SetFillColorAlpha(kBlue-7, 0.35);
  h1_org->Draw("HIST");

  h1_mix->SetLineColor(kRed);
  h1_mix->SetFillColorAlpha(kRed-9, 0.75);
  h1_mix->Draw("HISTsame");

  h1_sig->SetMarkerStyle(7);
  h1_sig->SetMarkerColor(kBlack);
  h1_sig->SetLineColor(kBlack);
  h1_sig->Draw("E1same");
  
  auto leg = new TLegend(0.70, 0.70, 0.90, 0.90);
  leg->AddEntry(h1_org, "Original"    ,"f");
  leg->AddEntry(h1_mix, "Mixed"       ,"f");
  leg->AddEntry(h1_sig, "Org. - Mixed","lp");
  leg->Draw();
  
  c1->SaveAs(fname);
  delete c1;
  delete leg;
  delete h1_sig;
  delete h1_org;
  delete h1_mix;
}

void Draw2D()
{
  TH2* h2_m_z_org = new TH2D("h2_m_z_org", ";Dimuon mass (GeV);Dimuon z (cm);Yield", 8, 0.5, 4.5,  40, -450, -50);
  TH2* h2_m_z_mix = new TH2D("h2_m_z_mix", ";Dimuon mass (GeV);Dimuon z (cm);Yield", 8, 0.5, 4.5,  40, -450, -50);
  tr_org->Project("h2_m_z_org", "z:mass");
  tr_mix->Project("h2_m_z_mix", "z:mass");
  
  gStyle->SetOptStat(0);
  TCanvas* c1 = new TCanvas("c1", "");
  c1->SetGrid();	
  
  h2_m_z_org->Draw("colz");  c1->SaveAs("result/h2_m_z_org.png");
  h2_m_z_mix->Draw("colz");  c1->SaveAs("result/h2_m_z_mix.png");

  delete c1;
  delete h2_m_z_org;
  delete h2_m_z_mix;
}

//// Main ////////////////////////////////////////////////////////////
void draw_hist()
{
  file = new TFile("tree.root");
  tr_org = (TTree*)file->Get("tree_org");
  tr_mix = (TTree*)file->Get("tree_mix");

  gSystem->mkdir("result", true);
  
  DrawHistObj();
  Draw2D();

  DrawOrgAndMixIn1D(";Mass (GeV);Yield", 60, 0, 6, "mass", "D1<9999", "result/h1_mass.png");
  DrawOrgAndMixIn1D(";Dimuon z (cm);Yield", 100, -500, 500, "z", "D1<9999", "result/h1_z.png");
  DrawOrgAndMixIn1D(";z_{#mu+} #minus z_{#mu#minus} (cm);Yield", 30, -420, 420, "trk_sep", "D1<9999", "result/h1_trk_sep.png"); // step_target = |Z_UPSTREAM|/NSTEPS_TARGET = 7 cm in GenFitExtrapolator.cxx
  
  DrawOrgAndMixIn1D(  "Up Right;Mass (GeV);Yield", 60, 0, 6, "mass", "spin==+1 && px<=0 && D1<9999", "result/h1_mass_UR.png");
  DrawOrgAndMixIn1D(  "Up Left ;Mass (GeV);Yield", 60, 0, 6, "mass", "spin==+1 && px> 0 && D1<9999", "result/h1_mass_UL.png");
  DrawOrgAndMixIn1D("Down Right;Mass (GeV);Yield", 60, 0, 6, "mass", "spin==-1 && px<=0 && D1<9999", "result/h1_mass_DR.png");
  DrawOrgAndMixIn1D("Down Left ;Mass (GeV);Yield", 60, 0, 6, "mass", "spin==-1 && px> 0 && D1<9999", "result/h1_mass_DL.png");
  
  exit(0);
}
