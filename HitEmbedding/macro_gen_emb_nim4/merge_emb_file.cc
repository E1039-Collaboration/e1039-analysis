R__LOAD_LIBRARY(interface_main)
R__LOAD_LIBRARY(ana_embedding)
using namespace std;

void merge_emb_file()
{
  const int n_evt_file = 2000; // N of events per merged file
  const string fn_list = "list_run_spill.txt";
  const string dir_in = "scratch";

  string dir_out = (string)"/pnfs/e1039/scratch/users/"+gSystem->Getenv("USER")+"/HitEmbedding/data_emb_nim4_merged";
  if (! gSystem->AccessPathName(dir_out.c_str())) {
    cout << "The output directory exists.  Enter 'GO' to delete it." << endl;
    string input;
    cin >> input;
    if (input != "GO") {
      cout << "Abort." << endl;
      exit(0);
    }
    cout << "Deleting the directory..." << endl;
    gSystem->Exec(("rm -rf "+dir_out).c_str());
  }
  
  vector<int> list_run;
  ifstream ifs(fn_list);
  int run_id;
  int spill_id;
  while (ifs >> run_id >> spill_id) list_run.push_back(run_id);
  ifs.close();
  std::sort(list_run.begin(), list_run.end());
  list_run.erase(std::unique(list_run.begin(), list_run.end()), list_run.end());
  int n_run = list_run.size();
  cout << "N of runs = " << n_run << endl;
  
  ostringstream oss;
  int n_evt_ok = 0;
  int n_evt_ng = 0;
  int n_evt_now = 0;
  int idx_out = 1;
  TFile* file_out = 0;
  TTree* tree_out = 0;
  for (int i_run = 0; i_run < n_run; i_run++) {
    run_id = list_run[i_run];
    cout << "Run " << run_id << " | " << i_run << "/" << n_run << endl;
    oss.str("");
    oss << dir_in << "/run_" << setfill('0') << setw(6) << run_id << "/out/embedding_data.root";
    string fn_in = oss.str();
    TFile* file_in = new TFile(fn_in.c_str());
    cout << "  " << fn_in << endl;
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
    SQEvent*     evt     = 0;
    SQHitVector* vec_hit = 0;
    EmbQAData*   qa_data = 0;
    tree_in->SetBranchAddress("SQEvent"    , &evt);
    tree_in->SetBranchAddress("SQHitVector", &vec_hit);
    tree_in->SetBranchAddress("qa_data"    , &qa_data);
    int n_ent = tree_in->GetEntries();
    cout << "  N of entries = " << n_ent << endl;
    for (int i_ent = 0; i_ent < n_ent; i_ent++) {
      tree_in->GetEntry(i_ent);
      if (! file_out) {
        cout << "  Output file: idx = " << idx_out << endl;
        oss.str("");
        oss << dir_out << "/" << setw(4) << idx_out;
        gSystem->mkdir(oss.str().c_str(), true);
        oss << "/embedding_data.root";
        file_out = new TFile(oss.str().c_str(), "RECREATE");
        tree_out = new TTree("tree", "Created by merge_emb_file.cc.");
        tree_out->Branch("SQEvent"    , &evt);
        tree_out->Branch("SQHitVector", &vec_hit);
      }
      if (qa_data->D1  == 0 || qa_data->D1  > 350 ||
          qa_data->D2  == 0 || qa_data->D2  > 170 || 
          qa_data->D3p == 0 || qa_data->D3p > 140 ||
          qa_data->D3m == 0 || qa_data->D3m > 140   ) {
        n_evt_ng++;
        continue;
      }
      tree_out->Fill();
      n_evt_ok++;
      n_evt_now++;
      if (n_evt_now == n_evt_file) {
        //cout << "  Close the output file." << endl;
        file_out->Write();
        file_out->Close();
        file_out = 0;
        idx_out++;
        n_evt_now = 0;
      }
    }
    file_in->Close();
  }
  if (file_out) {
    file_out->Write();
    file_out->Close();
    file_out = 0;
  }

  ofstream ofs(dir_out+"/00info.txt");
  ofs << "n_evt_file = " << n_evt_file << "\n"
      << "dir_in     = " << dir_in << "\n"
      << "dir_out    = " << dir_out << "\n"
      << "n_evt_ok   = " << n_evt_ok << "\n"
      << "n_evt_ng   = " << n_evt_ng << "\n";
  ofs.close();
  exit(0);
}
