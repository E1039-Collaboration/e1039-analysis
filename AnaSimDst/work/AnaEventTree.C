R__LOAD_LIBRARY(ana_sim_dst)

void AnaEventTree(const bool road_match=true)
{
  /// Choose one of analysis classes.
  auto ana = new AnaRecoDimuon();
  //auto ana = new AnaTrack();
  
  string fn_data = ana->GetOutputFileName();
  TFile* file = new TFile(fn_data.c_str());
  TTree* tree = (TTree*)file->Get("tree");
  ana->AnalyzeTree(tree, road_match);

  exit(0);
}
