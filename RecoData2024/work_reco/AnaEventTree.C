R__LOAD_LIBRARY(RecoData2024)
using namespace std;

void AnaEventTree(const string dim_type="PM", const char* dir_data_base="data/reco")
{
  TChain* tree = new TChain("tree");

  ifstream ifs("list_run_spill.txt");
  ofstream ofs("list_run_spill.txt.analyzed");
  int run_id, spill_id;
  while (ifs >> run_id >> spill_id) {
    ostringstream oss;
    //oss << setfill('0') << dir_data_base << "/run_" << setw(6) << run_id
    //    << "/spill_" << setw(9) << spill_id << "/out/output";
    //if (dim_type != "PM") oss << "_" << dim_type;
    //oss << ".root";
    oss << setfill('0') << dir_data_base << "/run_" << setw(6) << run_id
        << "/spill_" << setw(9) << spill_id
        << "/out/output_" << dim_type << ".root";
    if (gSystem->AccessPathName(oss.str().c_str())) continue;
    tree->Add(oss.str().c_str());
    ofs << run_id << "\t" << spill_id << "\n";
  }
  ifs.close();
  ofs.close();

  if (dim_type == "PM") AnaDimuon::AnalyzeTree(tree);
  else                  AnaDimuonLikeSign::AnalyzeTree(tree, dim_type);
  
  exit(0);
}
