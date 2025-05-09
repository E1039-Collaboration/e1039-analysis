R__LOAD_LIBRARY(RecoData2024)

void AnaEventTree(const char* dir_data_base="data/ana")
{
  const bool ana_dimuon = true; // false when analyzing track
  string fn_data = ana_dimuon ? "output.root" : "output_track.root";
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

  /// Old version:  Read all exisiting ROOT files.
  //void* dirp = gSystem->OpenDirectory(dir_data_base);
  //while (const char* dir_name = gSystem->GetDirEntry(dirp)) {
  //  string file_name = (string)dir_data_base + "/" + dir_name + "/out/" << fn_data;
  //  if (gSystem->AccessPathName(file_name.c_str())) continue;
  //  tree->Add(file_name.c_str());
  //}
  //gSystem->FreeDirectory(dirp);

  if (ana_dimuon) AnaDimuon::AnalyzeTree(tree);
  else            AnaTrack::AnalyzeTree(tree);
  exit(0);
}
