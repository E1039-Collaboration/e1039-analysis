
void eval_ppmm_dist()
{
  gStyle->SetOptStat(0);
  gROOT->ForceStyle();
  
  TFile* f_pm = new TFile("20241128-tight-ppmm/PM/result.root");
  TFile* f_pp = new TFile("20241128-tight-ppmm/PP/result.root");
  TFile* f_mm = new TFile("20241128-tight-ppmm/MM/result.root");

  TH1* h1_m_pm = (TH1*)f_pm->Get("h1_m");
  TH1* h1_m_pp = (TH1*)f_pp->Get("h1_m");
  TH1* h1_m_mm = (TH1*)f_mm->Get("h1_m");

  TH1* h1_m_ppmm = (TH1*)h1_m_pp->Clone("h1_m_ppmm");
  h1_m_ppmm->Reset();

  int nbin = h1_m_pm->GetNbinsX();
  for (int ib = 1; ib <= nbin; ib++) {
    double n_pp = h1_m_pp->GetBinContent(ib);
    double e_pp = h1_m_pp->GetBinError  (ib);
    double n_mm = h1_m_mm->GetBinContent(ib);
    double e_mm = h1_m_mm->GetBinError  (ib);
    if (n_pp < 1 || n_mm < 1) continue;
    double n = 2 * sqrt(n_pp * n_mm);
    double e = 2 * sqrt( (e_pp*e_pp*n_mm*n_mm + n_pp*n_pp*e_mm*e_mm) / (4*n_pp*n_mm) );
    h1_m_ppmm->SetBinContent(ib, n);
    h1_m_ppmm->SetBinError  (ib, e);
  }

  TCanvas* c1 = new TCanvas("c1", "");
  c1->SetGrid();
  
  h1_m_pp  ->SetLineColor(kGreen);
  h1_m_mm  ->SetLineColor(kBlue);
  h1_m_ppmm->SetLineColor(kRed);
  h1_m_ppmm->SetMarkerColor(kRed);
  h1_m_ppmm->SetMarkerStyle(7);
  //h1_m_ppmm->SetLineWidth(3);

  THStack* hs = new THStack("hs", ";Dimuon mass (GeV);Yield");
  hs->Add(h1_m_ppmm, "E1");
  hs->Add(h1_m_pp  , "");  
  hs->Add(h1_m_mm  , "");  
  hs->Draw("nostack");

  TLegend* leg = new TLegend(0.65, 0.7, 0.9, 0.9);
  leg->AddEntry(h1_m_pp  ,  "#plus #plus pairs", "l");
  leg->AddEntry(h1_m_mm  ,  "#minus #minus pairs", "l");
  leg->AddEntry(h1_m_ppmm,  "Averaged", "lp");
  leg->Draw();
  
  c1->SaveAs("result/h1_m_ppmm.png");
  delete hs;

  const double prescale = 2.0;
  h1_m_ppmm->Scale(prescale);

  hs = new THStack("hs", ";Dimuon mass (GeV);Yield");
  hs->Add(h1_m_ppmm, "E1");
  hs->Add(h1_m_pm  , "");  
  hs->Draw("nostack");

  TText text;
  text.SetTextColor(kRed);
  text.SetNDC(true);
  ostringstream oss;
  oss << "Prescale = " << prescale;
  text.DrawText(0.6, 0.7, oss.str().c_str());
  
  c1->SaveAs("result/h1_m_pm_ppmm.png");

  exit(0);
}
