R__LOAD_LIBRARY(RecoData2024)

void AnaEventTree(const char* dir_data_base="data/ana", const char* mode="PM", const bool road_match=true)
{
  /// Choose one of analysis classes.
  auto ana = new AnaDimuon("AnaDimuon", mode);
  //auto ana = new AnaTrack();
  
  string fn_data = ana->GetOutputFileName();
  TChain* tree = new TChain("tree");
  ifstream ifs("list_run_spill.txt");
  int run_id, spill_id;
  vector<int> list_run_id;
  while (ifs >> run_id >> spill_id) list_run_id.push_back(run_id);
  ifs.close();
  sort(list_run_id.begin(), list_run_id.end());
  list_run_id.erase(unique(list_run_id.begin(), list_run_id.end()), list_run_id.end());

  for (auto it = list_run_id.begin(); it != list_run_id.end(); it++) {
    ostringstream oss;
    oss << setfill('0') << dir_data_base << "/run_" << setw(6) << *it << "/out/" << fn_data;
    if (gSystem->AccessPathName(oss.str().c_str())) continue;
    tree->Add(oss.str().c_str());
  }

  ana->AnalyzeTree(tree, road_match);
  //ana->AnalyzeTree(tree);

  exit(0);
}
