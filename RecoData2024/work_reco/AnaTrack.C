R__LOAD_LIBRARY(RecoData2024)
using namespace std;

void AnaTrack(const char* dir_data_base="data/kmag-off")
{
  TChain* tree = new TChain("tree");

  ifstream ifs("list_run_spill.txt");
  int run_id, spill_id;
  while (ifs >> run_id >> spill_id) {
    ostringstream oss;
    oss << setfill('0') << dir_data_base << "/run_" << setw(6) << run_id
        << "/spill_" << setw(9) << spill_id
        << "/out/output_track.root";
    if (gSystem->AccessPathName(oss.str().c_str())) continue;
    tree->Add(oss.str().c_str());
  }
  ifs.close();

  AnaTrack::AnalyzeTree(tree);
  
  exit(0);
}
