using namespace std;

TFile* file;
TTree* tr_org;
TTree* tr_mix;

void DrawOrgAndMix()
{
  TH1* h1_mass_org = new TH1D("h1_mass_org", ";Mass (GeV);Yield", 90, 0, 9);
  TH1* h1_mass_mix = new TH1D("h1_mass_mix", ";Mass (GeV);Yield", 90, 0, 9);

  tr_org->Project("h1_mass_org", "mass");
  tr_mix->Project("h1_mass_mix", "mass");

  TH1* h1_mass_sig = (TH1*)h1_mass_org->Clone("h1_mass_sig");
  h1_mass_sig->Sumw2();
  h1_mass_sig->Add(h1_mass_mix, -1.0);

  gSystem->mkdir("result", true);
  gStyle->SetOptStat(0);
  TCanvas* c1 = new TCanvas("c1", "");
  c1->SetGrid();	
  
  h1_mass_org->SetFillColorAlpha(kBlue-7, 0.35);
  //h1_mass_org->GetYaxis()->SetRangeUser(0,h1_mass_org->GetMaximum()+50);
  //h1_mass_org->GetYaxis()->SetTitle("Yield");
  //h1_mass_org->GetXaxis()->SetTitle("Mass (GeV)");
  h1_mass_org->Draw("HIST");
  
  //h1_mass_sig->SetLineColor(kGreen-9);
  h1_mass_sig->SetFillColorAlpha(kGreen-9, 0.85);
  h1_mass_sig->Draw("SAME HIST E1");
  
  //m_mixed->SetMarkerStyle(kFullCircle);
  //m_mixed->SetMarkerColor(kRed);
  h1_mass_mix->SetFillColorAlpha(kRed-9, 0.75);
  //m_mixed->GetYaxis()->SetRangeUser(0,60);
  h1_mass_mix->Draw("SAME HIST");
  
  auto legend_tar = new TLegend(0.70, 0.70, 0.90, 0.90);
  //legend->SetHeader("The Legend Title","C"); // option "C" allows to center the header
  legend_tar->AddEntry(h1_mass_org, "Original"    ,"f");
  legend_tar->AddEntry(h1_mass_mix, "Mixed"       ,"f");
  legend_tar->AddEntry(h1_mass_sig, "Org. - Mixed","f");
  legend_tar->Draw();
  
  c1->SaveAs("result/h1_mass.png");
  delete c1;
}

void DrawFourTypes(TTree* tree, const string label)
{
  //TTree* tree = (TTree*)file->Get("tree_org");
  //TTree* tree = (TTree*)file->Get("tree_mix");

  TH1* h1_UR = new TH1D("h1_UR",   "Up Right;Mass (GeV);Yield", 60, 2, 6);
  TH1* h1_UL = new TH1D("h1_UL",    "Up Left;Mass (GeV);Yield", 60, 2, 6);
  TH1* h1_DR = new TH1D("h1_DR", "Down Right;Mass (GeV);Yield", 60, 2, 6);
  TH1* h1_DL = new TH1D("h1_DL",  "Down Left;Mass (GeV);Yield", 60, 2, 6);
  tree->Project("h1_UR", "mass", "spin==+1 && px<=0");
  tree->Project("h1_UL", "mass", "spin==+1 && px>0");
  tree->Project("h1_DR", "mass", "spin==-1 && px<=0");
  tree->Project("h1_DL", "mass", "spin==-1 && px>0");

  gSystem->mkdir("result", true);
  gStyle->SetOptStat(0);
  TCanvas* c1 = new TCanvas("c1", "");
  c1->SetGrid();	

  h1_UR->Draw();  c1->SaveAs(("result/h1_UR_"+label+".png").c_str());
  h1_UL->Draw();  c1->SaveAs(("result/h1_UL_"+label+".png").c_str());
  h1_DR->Draw();  c1->SaveAs(("result/h1_DR_"+label+".png").c_str());
  h1_DL->Draw();  c1->SaveAs(("result/h1_DL_"+label+".png").c_str());
  delete h1_UR;
  delete h1_UL;
  delete h1_DR;
  delete h1_DL;
  delete c1;
}

void draw_hist()
{
  file = new TFile("tree.root");
  tr_org = (TTree*)file->Get("tree_org");
  tr_mix = (TTree*)file->Get("tree_mix");
  
  DrawOrgAndMix();
  DrawFourTypes(tr_org, "org");
  DrawFourTypes(tr_mix, "mix");
  exit(0);
}
