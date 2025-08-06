R__LOAD_LIBRARY(interface_main)
R__LOAD_LIBRARY(ana_embedding)
using namespace std;

void draw_qa()
{
  const string dir_in = "scratch";
  
  const string fn_list = "list_run_spill.txt";
  vector<int> list_run;
  ifstream ifs(fn_list);
  int run_id;
  int spill_id;
  while (ifs >> run_id >> spill_id) list_run.push_back(run_id);
  ifs.close();
  std::sort(list_run.begin(), list_run.end());
  list_run.erase(std::unique(list_run.begin(), list_run.end()), list_run.end());
  cout << "N of runs = " << list_run.size() << endl;

  TH1* h1_rfp00 = new TH1D("h1_rfp00", ";RF+00;", 100, 0, 1000);
  TH1* h1_D1    = new TH1D("h1_D1"   , ";D1;" , 100, 0, 800);
  TH1* h1_D2    = new TH1D("h1_D2"   , ";D2;" , 100, 0, 500);
  TH1* h1_D3p   = new TH1D("h1_D3p"  , ";D3p;", 100, 0, 500);
  TH1* h1_D3m   = new TH1D("h1_D3m"  , ";D3m;", 100, 0, 500);

  int n_evt_all = 0;
  int n_evt_id_ok = 0;
  int n_evt_rf_ok = 0;
  int n_evt_occ_ok = 0;
  
  TFile* file_out = 0;
  TTree* tree_out = 0;
  for (auto it = list_run.begin(); it != list_run.end(); it++) {
    run_id = *it;
    cout << "Run " << run_id << endl;
    ostringstream oss;
    oss << dir_in << "/run_" << setfill('0') << setw(6) << run_id << "/out/embedding_data.root";
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
    EmbQAData* qa_data = 0;
    tree_in->SetBranchAddress("qa_data", &qa_data);

    for (int i_ent = 0; i_ent < tree_in->GetEntries(); i_ent++) {
      tree_in->GetEntry(i_ent);
      n_evt_all++;
      if (qa_data->turn_id > 0 && qa_data->turn_id > 0) n_evt_id_ok++;
      if (qa_data->rfp00 > 0) n_evt_rf_ok++;
      if (qa_data->D1 > 0 && qa_data->D2 > 0 && qa_data->D3p > 0 && qa_data->D3m > 0) {
        n_evt_occ_ok++;
        h1_rfp00->Fill(qa_data->rfp00);
        h1_D1   ->Fill(qa_data->D1);
        h1_D2   ->Fill(qa_data->D2);
        h1_D3p  ->Fill(qa_data->D3p);
        h1_D3m  ->Fill(qa_data->D3m);
      }
    }
    file_in->Close();
  }

  gSystem->mkdir("qa", true);

  TCanvas* c1 = new TCanvas("c1", "");
  c1->SetGrid();
  c1->SetLogy(true);

  h1_rfp00->Draw();  c1->SaveAs("qa/h1_rfp00.png");
  h1_D1   ->Draw();  c1->SaveAs("qa/h1_D1.png");
  h1_D2   ->Draw();  c1->SaveAs("qa/h1_D2.png");
  h1_D3p  ->Draw();  c1->SaveAs("qa/h1_D3p.png");
  h1_D3m  ->Draw();  c1->SaveAs("qa/h1_D3m.png");

  ofstream ofs("qa/count.txt");
  ofs << "All           = " << n_evt_all << "\n"
      << "Turn+RF ID OK = " << n_evt_id_ok << "\n"
      << "RFp00      OK = " << n_evt_rf_ok << "\n"
      << "Occ        OK = " << n_evt_occ_ok << "\n";
  ofs.close();
  
  exit(0);
}
