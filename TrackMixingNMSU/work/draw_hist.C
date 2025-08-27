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

//void DrawOrgAndMix()
//{
//  TH1* h1_mass_org = new TH1D("h1_mass_org", ";Mass (GeV);Yield", 60, 0, 6);
//  TH1* h1_mass_mix = new TH1D("h1_mass_mix", ";Mass (GeV);Yield", 60, 0, 6);
//
//  tr_org->Project("h1_mass_org", "mass", "D1 < 100");
//  tr_mix->Project("h1_mass_mix", "mass", "D1 < 100");
//
//  TH1* h1_mass_sig = (TH1*)h1_mass_org->Clone("h1_mass_sig");
//  h1_mass_sig->Sumw2();
//  h1_mass_sig->Add(h1_mass_mix, -1.0);
//
//  gStyle->SetOptStat(0);
//  TCanvas* c1 = new TCanvas("c1", "");
//  c1->SetGrid();	
//  
//  h1_mass_org->SetFillColorAlpha(kBlue-7, 0.35);
//  //h1_mass_org->GetYaxis()->SetRangeUser(0,h1_mass_org->GetMaximum()+50);
//  //h1_mass_org->GetYaxis()->SetTitle("Yield");
//  //h1_mass_org->GetXaxis()->SetTitle("Mass (GeV)");
//  h1_mass_org->Draw("HIST");
//  
//  //h1_mass_sig->SetLineColor(kGreen-9);
//  h1_mass_sig->SetFillColorAlpha(kGreen-9, 0.85);
//  h1_mass_sig->Draw("SAME HIST E1");
//  
//  //m_mixed->SetMarkerStyle(kFullCircle);
//  //m_mixed->SetMarkerColor(kRed);
//  h1_mass_mix->SetFillColorAlpha(kRed-9, 0.75);
//  //m_mixed->GetYaxis()->SetRangeUser(0,60);
//  h1_mass_mix->Draw("SAME HIST");
//  
//  auto legend_tar = new TLegend(0.70, 0.70, 0.90, 0.90);
//  //legend->SetHeader("The Legend Title","C"); // option "C" allows to center the header
//  legend_tar->AddEntry(h1_mass_org, "Original"    ,"f");
//  legend_tar->AddEntry(h1_mass_mix, "Mixed"       ,"f");
//  legend_tar->AddEntry(h1_mass_sig, "Org. - Mixed","f");
//  legend_tar->Draw();
//  
//  c1->SaveAs("result/h1_mass.png");
//  delete c1;
//}

//void DrawOrgAndMixMore()
//{
//  //TH1* h1_z_org = new TH1D("h1_z_org", ";Dimuon z (cm);Yield", 100, -500, 500);
//  //TH1* h1_z_mix = new TH1D("h1_z_mix", ";Dimuon z (cm);Yield", 100, -500, 500);
//  //tr_org->Project("h1_z_org", "z");
//  //tr_mix->Project("h1_z_mix", "z");
//  //TH1* h1_z_sig = (TH1*)h1_z_org->Clone("h1_z_sig");
//  //h1_z_sig->Sumw2();
//  //h1_z_sig->Add(h1_z_mix, -1.0);
//
//  TH2* h2_m_z_org = new TH2D("h2_m_z_org", ";Dimuon mass (GeV);Dimuon z (cm);Yield", 8, 0.5, 4.5,  40, -450, -50);
//  TH2* h2_m_z_mix = new TH2D("h2_m_z_mix", ";Dimuon mass (GeV);Dimuon z (cm);Yield", 8, 0.5, 4.5,  40, -450, -50);
//  tr_org->Project("h2_m_z_org", "z:mass");
//  tr_mix->Project("h2_m_z_mix", "z:mass");
//
//
//  // step_target = |Z_UPSTREAM|/NSTEPS_TARGET = 7 cm in GenFitExtrapolator.cxx
//  //TH1* h1_trk_sep_org = new TH1D("h1_trk_sep_org", ";z_{#mu+} #minus z_{#mu#minus} (cm);Yield", 30, -420, 420);
//  //TH1* h1_trk_sep_mix = new TH1D("h1_trk_sep_mix", ";z_{#mu+} #minus z_{#mu#minus} (cm);Yield", 30, -420, 420);
//  //tr_org->Project("h1_trk_sep_org", "trk_sep");
//  //tr_mix->Project("h1_trk_sep_mix", "trk_sep");
//  
//  gStyle->SetOptStat(0);
//  TCanvas* c1 = new TCanvas("c1", "");
//  c1->SetGrid();	
//  
//  //h1_z_org->SetFillColorAlpha(kBlue-7, 0.35);
//  //h1_z_org->Draw("HIST");
//  //h1_z_sig->SetFillColorAlpha(kGreen-9, 0.85);
//  //h1_z_sig->Draw("SAME HIST E1");
//  //h1_z_mix->SetFillColorAlpha(kRed-9, 0.75);
//  //h1_z_mix->Draw("SAME HIST");
//  //
//  //auto legend_tar = new TLegend(0.70, 0.70, 0.90, 0.90);
//  //legend_tar->AddEntry(h1_z_org, "Original"    ,"f");
//  //legend_tar->AddEntry(h1_z_mix, "Mixed"       ,"f");
//  //legend_tar->AddEntry(h1_z_sig, "Org. - Mixed","f");
//  //legend_tar->Draw();
//  //c1->SaveAs("result/h1_z.png");
//
//  h2_m_z_org->Draw("colz");  c1->SaveAs("result/h2_m_z_org.png");
//  h2_m_z_mix->Draw("colz");  c1->SaveAs("result/h2_m_z_mix.png");
//
//  //h1_trk_sep_org->Draw("colz");  c1->SaveAs("result/h1_trk_sep_org.png");
//  //h1_trk_sep_mix->Draw("colz");  c1->SaveAs("result/h1_trk_sep_mix.png");
//  
//  delete c1;
//}

//void DrawFourTypes(TTree* tree, const string label)
//{
//  TH1* h1_UR = new TH1D("h1_UR",   "Up Right;Mass (GeV);Yield", 60, 2, 6);
//  TH1* h1_UL = new TH1D("h1_UL",    "Up Left;Mass (GeV);Yield", 60, 2, 6);
//  TH1* h1_DR = new TH1D("h1_DR", "Down Right;Mass (GeV);Yield", 60, 2, 6);
//  TH1* h1_DL = new TH1D("h1_DL",  "Down Left;Mass (GeV);Yield", 60, 2, 6);
//  tree->Project("h1_UR", "mass", "spin==+1 && px<=0");
//  tree->Project("h1_UL", "mass", "spin==+1 && px>0");
//  tree->Project("h1_DR", "mass", "spin==-1 && px<=0");
//  tree->Project("h1_DL", "mass", "spin==-1 && px>0");
//
//  gStyle->SetOptStat(0);
//  TCanvas* c1 = new TCanvas("c1", "");
//  c1->SetGrid();	
//
//  h1_UR->Draw();  c1->SaveAs(("result/h1_UR_"+label+".png").c_str());
//  h1_UL->Draw();  c1->SaveAs(("result/h1_UL_"+label+".png").c_str());
//  h1_DR->Draw();  c1->SaveAs(("result/h1_DR_"+label+".png").c_str());
//  h1_DL->Draw();  c1->SaveAs(("result/h1_DL_"+label+".png").c_str());
//  delete h1_UR;
//  delete h1_UL;
//  delete h1_DR;
//  delete h1_DL;
//  delete c1;
//}

//void DrawFourTypes()
//{
//  TH1* h1_UR_org = new TH1D("h1_UR_org",   "Up Right;Mass (GeV);Yield", 60, 0, 6);
//  TH1* h1_UL_org = new TH1D("h1_UL_org",    "Up Left;Mass (GeV);Yield", 60, 0, 6);
//  TH1* h1_DR_org = new TH1D("h1_DR_org", "Down Right;Mass (GeV);Yield", 60, 0, 6);
//  TH1* h1_DL_org = new TH1D("h1_DL_org",  "Down Left;Mass (GeV);Yield", 60, 0, 6);
//  tr_org->Project("h1_UR_org", "mass", "spin==+1 && px<=0");
//  tr_org->Project("h1_UL_org", "mass", "spin==+1 && px>0");
//  tr_org->Project("h1_DR_org", "mass", "spin==-1 && px<=0");
//  tr_org->Project("h1_DL_org", "mass", "spin==-1 && px>0");
//
//  TH1* h1_UR_mix = new TH1D("h1_UR_mix",   "Up Right;Mass (GeV);Yield", 60, 0, 6);
//  TH1* h1_UL_mix = new TH1D("h1_UL_mix",    "Up Left;Mass (GeV);Yield", 60, 0, 6);
//  TH1* h1_DR_mix = new TH1D("h1_DR_mix", "Down Right;Mass (GeV);Yield", 60, 0, 6);
//  TH1* h1_DL_mix = new TH1D("h1_DL_mix",  "Down Left;Mass (GeV);Yield", 60, 0, 6);
//  tr_mix->Project("h1_UR_mix", "mass", "spin==+1 && px<=0");
//  tr_mix->Project("h1_UL_mix", "mass", "spin==+1 && px>0");
//  tr_mix->Project("h1_DR_mix", "mass", "spin==-1 && px<=0");
//  tr_mix->Project("h1_DL_mix", "mass", "spin==-1 && px>0");
//  
//  gStyle->SetOptStat(0);
//  TCanvas* c1 = new TCanvas("c1", "");
//  c1->SetGrid();	
//
//  h1_UR_org->SetLineColor(kRed);  
//  h1_UR_org->Draw();
//  h1_UR_mix->Draw("same");
//  c1->SaveAs("result/h1_UR.png");
//
//  h1_UL_org->SetLineColor(kRed);  
//  h1_UL_org->Draw();
//  h1_UL_mix->Draw("same");
//  c1->SaveAs("result/h1_UL.png");
//  
//  h1_DR_org->SetLineColor(kRed);  
//  h1_DR_org->Draw();
//  h1_DR_mix->Draw("same");
//  c1->SaveAs("result/h1_DR.png");
//
//  h1_DL_org->SetLineColor(kRed);  
//  h1_DL_org->Draw();
//  h1_DL_mix->Draw("same");
//  c1->SaveAs("result/h1_DL.png");
//  
//  delete h1_UR_org;
//  delete h1_UL_org;
//  delete h1_DR_org;
//  delete h1_DL_org;
//  delete h1_UR_mix;
//  delete h1_UL_mix;
//  delete h1_DR_mix;
//  delete h1_DL_mix;
//  delete c1;
//}

//// Main ////////////////////////////////////////////////////////////
void draw_hist()
{
  file = new TFile("tree.root");
  tr_org = (TTree*)file->Get("tree_org");
  tr_mix = (TTree*)file->Get("tree_mix");

  gSystem->mkdir("result", true);
  
  DrawHistObj();
  Draw2D();
  //DrawOrgAndMix();
  //DrawOrgAndMixMore();

  DrawOrgAndMixIn1D(";Mass (GeV);Yield", 60, 0, 6, "mass", "D1<100", "result/h1_mass.png");
  DrawOrgAndMixIn1D(";Dimuon z (cm);Yield", 100, -500, 500, "z", "D1<100", "result/h1_z.png");
  DrawOrgAndMixIn1D(";z_{#mu+} #minus z_{#mu#minus} (cm);Yield", 30, -420, 420, "trk_sep", "D1<100", "result/h1_trk_sep.png"); // step_target = |Z_UPSTREAM|/NSTEPS_TARGET = 7 cm in GenFitExtrapolator.cxx
  
  DrawOrgAndMixIn1D(  "Up Right;Mass (GeV);Yield", 60, 0, 6, "mass", "spin==+1 && px<=0 && D1<100", "result/h1_mass_UR.png");
  DrawOrgAndMixIn1D(  "Up Left ;Mass (GeV);Yield", 60, 0, 6, "mass", "spin==+1 && px> 0 && D1<100", "result/h1_mass_UL.png");
  DrawOrgAndMixIn1D("Down Right;Mass (GeV);Yield", 60, 0, 6, "mass", "spin==-1 && px<=0 && D1<100", "result/h1_mass_DR.png");
  DrawOrgAndMixIn1D("Down Left ;Mass (GeV);Yield", 60, 0, 6, "mass", "spin==-1 && px> 0 && D1<100", "result/h1_mass_DL.png");
  
  //DrawFourTypes(tr_org, "org");
  //DrawFourTypes(tr_mix, "mix");
  //DrawFourTypes();
  exit(0);
}
