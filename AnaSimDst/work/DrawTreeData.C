/// DrawTreeData.C: An example macro to draw variables using TTree::Draw().
#include "Common.h"
R__LOAD_LIBRARY(libana_sim_dst)
using namespace std;
TFile* file;
TTree* tree;
TCanvas* c1;
double inte_lumi;
void DrawDimTrueKin();
void DrawDimRecoKin();
void DrawDimTrkTrueKin();
void DrawTrkTrueKin();
void DrawTrueVar(const string varname, const string title_x, const int n_x, const double x_lo, const double x_hi);
void FitCosTheta();

void DrawTreeData(const char* fname="sim_tree.root", const char* tname="tree")
{
  file = new TFile(fname);
  tree = (TTree*)file->Get(tname);
  gSystem->mkdir("result", true);
  c1 = new TCanvas("c1", "");
  c1->SetGrid();
  //c1->SetLogy(true);

  inte_lumi = GetInteLumi();
  cout << "Integrated luminosity = " << inte_lumi << endl;

  tree->Draw("rec_stat", "weight"); // cf. GlobalConsts.h.
  c1->SaveAs("result/h1_rec_stat.png");

  /// You can use these functions or add new ones.
  //DrawDimTrueKin();
  //DrawDimRecoKin();
  //DrawDimTrkTrueKin();
  DrawTrkTrueKin();
  //FitCosTheta();

  exit(0);
}

///
/// Functions below
///
void DrawDimTrueKin()
{
  tree->Draw("n_dim_true");
  c1->SaveAs("result/h1_true_n_dim.png");

  const double PI = TMath::Pi();
  DrawTrueVar("dim_true.pdg_id"    , "True dimuon PDG ID", 1000, 0, 0);
  DrawTrueVar("dim_true.mom.X()"   , "True dimuon px (GeV)", 100, -5,   5);
  DrawTrueVar("dim_true.mom.Y()"   , "True dimuon py (GeV)", 100, -5,   5);
  DrawTrueVar("dim_true.mom.Z()"   , "True dimuon pz (GeV)", 100,  0, 100);
  DrawTrueVar("dim_true.mom.M()"   , "True dimuon mass (GeV)", 100, 0, 5);
  DrawTrueVar("dim_true.mom.Eta()" , "True dimuon #eta", 110, 0, 11);
  DrawTrueVar("dim_true.mom.Phi()" , "True dimuon #phi", 100, -PI, PI);
  DrawTrueVar("dim_true.x1"        , "True x1", 50, 0, 1);
  DrawTrueVar("dim_true.x2"        , "True x2", 50, 0, 1);
  DrawTrueVar("dim_true.xF"        , "True xF", 50, -1, 1);
  DrawTrueVar("dim_true.costh"     , "True cos#theta", 50, -1, 1);
  DrawTrueVar("dim_true.phi"       , "True #phi"     , 50, -PI, PI);
}

void DrawDimRecoKin()
{
  tree->Draw("n_dim_reco", "weight");
  c1->SaveAs("result/h1_reco_n_dim.png");

  tree->Draw("rec_stat", "weight"); // cf. GlobalConsts.h.
  c1->SaveAs("result/h1_rec_stat.png");
  
  tree->Draw("trig_bits", "weight * (rec_stat==0)");
  c1->SaveAs("result/h1_trig_bits.png");

  tree->Draw("dim_reco.mom.M()", "weight * (rec_stat==0)");
  c1->SaveAs("result/h1_dim_reco_mass.png");

  tree->Draw("dim_reco.x1", "weight * (rec_stat==0)");
  c1->SaveAs("result/h1_dim_reco_x1.png");

  tree->Draw("dim_reco.x2", "weight * (rec_stat==0)");
  c1->SaveAs("result/h1_dim_reco_x2.png");
}

/// Function to draw tracks of dimuons (not single tracks).
void DrawDimTrkTrueKin()
{
  DrawTrueVar("dim_true.mom_pos.X()", "True px (GeV) of mu+", 100, -5, 5);
  DrawTrueVar("dim_true.mom_pos.Y()", "True py (GeV) of mu+", 100, -5, 5);
  DrawTrueVar("dim_true.mom_pos.Z()", "True pz (GeV) of mu+", 100,  0, 100);
  DrawTrueVar("dim_true.mom_neg.X()", "True px (GeV) of mu-", 100, -5, 5);
  DrawTrueVar("dim_true.mom_neg.Y()", "True py (GeV) of mu-", 100, -5, 5);
  DrawTrueVar("dim_true.mom_neg.Z()", "True pz (GeV) of mu-", 100,  0, 100);

  THStack* hs;
  TH1* h1_all = new TH1D("h1_all", "", 100, -1, 1);
  TH1* h1_rec = new TH1D("h1_rec", "", 100, -1, 1);
  tree->Project("h1_all", "(dim_true.mom_pos.Z() - dim_true.mom_neg.Z())/(dim_true.mom_pos.Z() + dim_true.mom_neg.Z())", "weight");
  tree->Project("h1_rec", "(dim_true.mom_pos.Z() - dim_true.mom_neg.Z())/(dim_true.mom_pos.Z() + dim_true.mom_neg.Z())", "weight * (rec_stat==0)");

  hs = new THStack("hs", "GMC;gpz+gpz (GeV) of tracks;N of tracks");
  hs->Add(h1_all);
  hs->Add(h1_rec);
  h1_rec->SetLineColor(kRed);
  hs->Draw("nostack");
  c1->SaveAs("result/h1_trk_true_pz_asym.png");
}

/// Function to draw single tracks.
void DrawTrkTrueKin()
{
  DrawTrueVar("trk_true.charge"     , "True charge of single tracks"  ,   3, -1.5, 1.5);
  DrawTrueVar("trk_true.mom_vtx.X()", "True px (GeV) of single tracks", 100, -5, 5);
  DrawTrueVar("trk_true.mom_vtx.Y()", "True py (GeV) of single tracks", 100, -5, 5);
  DrawTrueVar("trk_true.mom_vtx.Z()", "True pz (GeV) of single tracks", 100,  0, 100);

  THStack* hs;
  TH1* h1_all = new TH1D("h1_all", "", 3, -1.5, 1.5);
  TH1* h1_rec = new TH1D("h1_rec", "", 3, -1.5, 1.5);
  tree->Project("h1_all", "trk_true.charge", "weight");
  tree->Project("h1_rec", "trk_true.charge", "weight * (rec_stat==0)");

  hs = new THStack("hs", "GMC;True charge of single tracks;N of tracks");
  hs->Add(h1_all);
  hs->Add(h1_rec);
  h1_rec->SetLineColor(kRed);
  hs->Draw("nostack");
  c1->SaveAs("result/h1_trk_true_charge.png");
}

void DrawTrueVar(const string varname, const string title_x, const int n_x, const double x_lo, const double x_hi)
{
  TH1* h1_all = new TH1D("h1_all", "", n_x, x_lo, x_hi);
  TH1* h1_rec = new TH1D("h1_rec", "", n_x, x_lo, x_hi);
  tree->Project("h1_all", varname.c_str(), "weight");
  tree->Project("h1_rec", varname.c_str(), "weight * (rec_stat==0)");

  THStack hs("hs", Form("GMC;%s;Yield", title_x.c_str()));
  hs.Add(h1_all);
  hs.Add(h1_rec);
  h1_rec->SetLineColor(kRed);
  hs.Draw("nostack");

  ostringstream oss;
  oss << "result/h1_";
  for (string::const_iterator it = varname.begin(); it != varname.end(); it++) {
    switch (*it) { // modify bad chars for file name
    case '.': case '*': case '/': oss << '_'; break;
    case '(': case ')': case ' ': /* omit */ break;
    default: oss << *it;
    }
  }
  oss << ".png";
  c1->SaveAs(oss.str().c_str());

  delete h1_all;
  delete h1_rec;
}

void FitCosTheta()
{
  gStyle->SetOptFit(true);
  TH1* h1_costh = new TH1D("h1_costh", "", 100, -1, 1);
  tree->Project("h1_costh", "dim_true.costh", "weight");
  h1_costh->Scale(1/inte_lumi);
  TF1* f1 = new TF1("f1", "[0]*(1 + [1]*pow(x,2))", -0.8, 0.8);
  f1->SetParameters(h1_costh->Integral()/h1_costh->GetNbinsX(), 1.0);
  h1_costh->Fit(f1, "REM");

  h1_costh->SetTitle("GMC;True cos#theta;Yield");
  c1->SaveAs("result/h1_costh_fit.png");
  delete f1;
  delete h1_costh;
}

double GetInteLumi(const char* fn_lumi)
{
  ifstream ifs(fn_lumi);
  if (!ifs.is_open()) {
    cout << "GetInteLumi():  Cannot open '" << fn_lumi << "'.  Just return 1.0." << endl;
    return 1.0;
  }
  double val;
  ifs >> val;
  ifs.close();
  return val;
}
