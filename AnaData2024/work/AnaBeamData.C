R__LOAD_LIBRARY(UtilAna)
R__LOAD_LIBRARY(AnaData2024)
using namespace std;
void ListOfRfValues(int& n_value, int*& list_values);
void ListOfRfValues(int& n_value, double*& list_values);
void NormRFHist(TH1* h1);

void AnaBeamData()
{
  const string dir_in = "scratch/default";
  const string fn_list = "list_run_spill.txt";
  vector<int> list_run;
  ifstream ifs(fn_list);
  int run_id;
  int spill_id;
  while (ifs >> run_id >> spill_id) list_run.push_back(run_id);
  ifs.close();
  std::sort(list_run.begin(), list_run.end());
  list_run.erase(std::unique(list_run.begin(), list_run.end()), list_run.end());
  int run_lo = *(list_run.begin());
  int run_hi =   list_run.back () ;
  int run_n  = run_hi - run_lo + 1;
  cout << "Runs " << run_lo << "..." << run_hi << "; n=" << list_run.size() << endl;
  
  TH2* h2_run_turn = new TH2D("h2_turn_run", ";Run ID;Trun ID", run_n, run_lo, run_hi+1, 369, 0.5, 369000.5);
  TH2* h2_run_rf   = new TH2D("h2_rf_run"  , ";Run ID; RF ID", run_n, run_lo, run_hi+1, 588, 0.5,    588.5);

  int num_inte;
  double* list_inte;
  ListOfRfValues(num_inte, list_inte);
  TH1* h1_rfp01 = new TH1D("h1_rfp01", ";RF+01;", num_inte-1, list_inte);
  TH1* h1_rfp00 = new TH1D("h1_rfp00", ";RF+00;", num_inte-1, list_inte);
  TH1* h1_rfm01 = new TH1D("h1_rfm01", ";RF-01;", num_inte-1, list_inte);

  // h_pot_p00 in nim3pot_run6.root: (130, 0, 1.3e6)
  TH1* h1_pot   = new TH1D("h1_pot", ";N of protons/bucket;", 130, -0.05e6, 1.25e6);
  
  TH1* h1_d1    = new TH1D("h1_d1"   , ";D1;" , 100, -0.5, 800-0.5);
  TH1* h1_d2    = new TH1D("h1_d2"   , ";D2;" , 100, -0.5, 500-0.5);
  TH1* h1_d3p   = new TH1D("h1_d3p"  , ";D3p;", 100, -0.5, 500-0.5);
  TH1* h1_d3m   = new TH1D("h1_d3m"  , ";D3m;", 100, -0.5, 500-0.5);

  int n_evt_all = 0;
  int n_evt_trig_ok = 0;
  int n_evt_id_ok = 0;
  int n_evt_rf_ok = 0;
  int n_evt_occ_ok = 0;
  
  TFile* file_out = 0;
  TTree* tree_out = 0;
  for (auto it = list_run.begin(); it != list_run.end(); it++) {
    run_id = *it;
    cout << "Run " << run_id << endl;
    ostringstream oss;
    oss << dir_in << "/run_" << setfill('0') << setw(6) << run_id << "/out/output.root";
    string fn_in = oss.str();
    TFile* file_in = new TFile(fn_in.c_str());
    if (! file_in->IsOpen()) {
      cout << "  Cannot open the input file." << endl;
      delete file_in;
      continue;
    }
    TTree* tree_in = (TTree*)file_in->Get("tree");
    if (! tree_in) {
      cout << "  Cannot get the input tree." << endl;
      delete file_in;
      continue;
    }
    BeamData* beam = 0;
    tree_in->SetBranchAddress("beam_data", &beam);

    for (int i_ent = 0; i_ent < tree_in->GetEntries(); i_ent++) {
      tree_in->GetEntry(i_ent);
      n_evt_all++;
      //if (! (beam->fpga_bits & 0x1)) continue; // FPGA1
      if (! (beam->nim_bits  & 0x8)) continue; // NIM4
      n_evt_trig_ok++;      
      
      if (beam->turn_id == 0 || beam->rf_id == 0) continue;
      n_evt_id_ok++;
      if (beam->rfp00 > 0) n_evt_rf_ok++;

      //if (beam->rf_id >  90) continue; // Select empty buckets
      //if (beam->rf_id < 440) continue; // Select empty buckets
      if (beam->run_id < 6145) { // Select filled buckets
        if (beam->rf_id < 95 || beam->rf_id > 260) continue;
      } else {
        if (beam->rf_id < 95 || beam->rf_id > 430) continue;
      }
      
      h2_run_turn->Fill(beam->run_id, beam->turn_id);
      h2_run_rf  ->Fill(beam->run_id, beam->  rf_id);
      h1_rfp01->Fill(beam->rfp01);
      h1_rfp00->Fill(beam->rfp00);
      h1_rfm01->Fill(beam->rfm01);

      const double qie_ped = 40.0;
      static unordered_map<int, double> list_coef; // spill ID -> PoT/QIE
      double coef;
      auto iter = list_coef.find(beam->spill_id);
      if (iter != list_coef.end()) {
        coef = iter->second;
      } else {
        int sp_id = beam->spill_id;
        UtilSlowCont* usc = UtilSlowCont::instance();
        double g2sem = usc->GetAcnet  ("S:G2SEM", sp_id);
        double qie   = usc->GetBeamDAQ("QIEsum" , sp_id);
        if (g2sem > 0 && qie > 0) coef = g2sem / (qie - qie_ped*588*369000);
        else                      coef = 0;
        list_coef[sp_id] = coef;
      }
      if (coef > 0) {
        double pot = (beam->rfp00 - qie_ped) * coef;
        h1_pot->Fill(pot);
      }
      
      if (beam->d1 > 0 && beam->d2 > 0 && beam->d3p > 0 && beam->d3m > 0) {
        n_evt_occ_ok++;
        h1_d1   ->Fill(beam->d1);
        h1_d2   ->Fill(beam->d2);
        h1_d3p  ->Fill(beam->d3p);
        h1_d3m  ->Fill(beam->d3m);
      }
    }
    file_in->Close();
  }

  gSystem->mkdir("result", true);

  TCanvas* c1 = new TCanvas("c1", "");
  c1->SetGrid();

  h2_run_turn->SetStats(0);
  h2_run_turn->Draw("colz");
  c1->SaveAs("result/h2_run_turn.png");
  
  h2_run_rf->SetStats(0);
  h2_run_rf->Draw("colz");
  c1->SaveAs("result/h2_run_rf.png");
  
  TH1* h1_turn_id = h2_run_turn->ProjectionY("h1_turn_id");
  h1_turn_id->Draw();
  c1->SaveAs("result/h1_turn_id.png");
  
  TH1* h1_rf_id = h2_run_rf->ProjectionY("h1_rf_id");
  h1_rf_id->Draw();
  c1->SaveAs("result/h1_rf_id.png");

  c1->SetLogx(true);
  c1->SetLogy(true);

  NormRFHist(h1_rfp01);
  NormRFHist(h1_rfp00);
  NormRFHist(h1_rfm01);
  h1_rfp01->GetXaxis()->SetRangeUser(1, 20e3);
  h1_rfp00->GetXaxis()->SetRangeUser(1, 20e3);
  h1_rfm01->GetXaxis()->SetRangeUser(1, 20e3);  
  h1_rfp01->Draw();  c1->SaveAs("result/h1_rfp01.png");
  h1_rfp00->Draw();  c1->SaveAs("result/h1_rfp00.png");
  h1_rfm01->Draw();  c1->SaveAs("result/h1_rfm01.png");

  c1->SetLogx(false);

  h1_pot->Draw();
  c1->SaveAs("result/h1_pot.png");

  c1->SetLogy(false);
  
  h1_rfp01->GetXaxis()->SetRangeUser(1, 100);
  h1_rfp00->GetXaxis()->SetRangeUser(1, 100);
  h1_rfm01->GetXaxis()->SetRangeUser(1, 100);
  h1_rfp01->Draw();  c1->SaveAs("result/h1_rfp01_low.png");
  h1_rfp00->Draw();  c1->SaveAs("result/h1_rfp00_low.png");
  h1_rfm01->Draw();  c1->SaveAs("result/h1_rfm01_low.png");
  
  h1_d1   ->Draw();  c1->SaveAs("result/h1_d1.png");
  h1_d2   ->Draw();  c1->SaveAs("result/h1_d2.png");
  h1_d3p  ->Draw();  c1->SaveAs("result/h1_d3p.png");
  h1_d3m  ->Draw();  c1->SaveAs("result/h1_d3m.png");

  ofstream ofs("result/count.txt");
  ofs << "All           = " << n_evt_all << "\n"
      << "Trigger    OK = " << n_evt_trig_ok << "\n"
      << "Turn+RF ID OK = " << n_evt_id_ok << "\n"
      << "RFp00      OK = " << n_evt_rf_ok << "\n"
      << "Occ        OK = " << n_evt_occ_ok << "\n";
  ofs.close();
  
  exit(0);
}

void ListOfRfValues(int& n_value, int*& list_values)
{
  static int idx = 0;
  static int list[256];
  if (idx == 0) {
    int val = 0;
    list[idx++] = val;
    for (int i = 0; i < 16; i++) { val +=    1; list[idx++] = val; }
    for (int i = 0; i < 20; i++) { val +=    2; list[idx++] = val; }
    for (int i = 0; i < 21; i++) { val +=    4; list[idx++] = val; }
    for (int i = 0; i < 20; i++) { val +=    8; list[idx++] = val; }
    for (int i = 0; i < 15; i++) { val +=   16; list[idx++] = val; }
    for (int i = 0; i <  1; i++) { val +=   31; list[idx++] = val; }
    for (int i = 0; i <  4; i++) { val +=   16; list[idx++] = val; }
    for (int i = 0; i < 21; i++) { val +=   32; list[idx++] = val; }
    for (int i = 0; i < 20; i++) { val +=   64; list[idx++] = val; }
    for (int i = 0; i < 20; i++) { val +=  128; list[idx++] = val; }
    for (int i = 0; i < 21; i++) { val +=  256; list[idx++] = val; }
    for (int i = 0; i < 20; i++) { val +=  512; list[idx++] = val; }
    for (int i = 0; i < 20; i++) { val += 1024; list[idx++] = val; }
    for (int i = 0; i < 21; i++) { val += 2048; list[idx++] = val; }
    for (int i = 0; i <  6; i++) { val += 4096; list[idx++] = val; }
  }
  n_value = idx;
  list_values = list;
}

void ListOfRfValues(int& n_value, double*& list_values)
{
  static int idx = 0;
  static double list[256];
  if (idx == 0) {
    int* list_int;
    ListOfRfValues(idx, list_int);
    for (int i = 0; i < idx; i++) list[i] = list_int[i];
  }
  n_value = idx;
  list_values = list;
}

void NormRFHist(TH1* h1)
{
  if (h1->GetSumw2N() == 0) h1->Sumw2();
  for (int ib = 1; ib <= h1->GetNbinsX(); ib++) {
    double width = h1->GetBinWidth(ib);
    h1->SetBinContent(ib, h1->GetBinContent(ib) / width);
    h1->SetBinError  (ib, h1->GetBinError  (ib) / width);
  }
}

//void GetBeamInfo(const int spill_id, double& g2sem, double& qie, double& inhi, double& busy)
//{
//      UtilSlowCont* usc = UtilSlowCont::instance();
//      double g2sem = usc->GetAcnet  ("S:G2SEM", beam->spill_id);
//      double qie   = usc->GetBeamDAQ("QIEsum" , beam->spill_id);
//      double inhi  = usc->GetBeamDAQ("inhibit_block_sum" , beam->spill_id);
//      double busy  = usc->GetBeamDAQ("trigger_sum_no_inhibit" , beam->spill_id);
//}
