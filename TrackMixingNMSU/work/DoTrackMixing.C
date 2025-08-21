/// ROOT macro for the 1st analysis step.
/**
 * This step processes one run given as the 1st argument (run_id).
 * It is executed for multiple runs in parallel.
 * Therefore you should include in this step
 *  - Time-consuming parts of your analysis (like event reconstruction) and
 *  - Stable parts of your analysis (like basic analysis cuts).
 *
 * The contents of the analysis are implemented in the analysis modules.
 * You find or create all analysis modules under `src/`, such as `AnaHodoTime`.
 * You select one of them in `MacroCommon.C`.
 */
R__LOAD_LIBRARY(TrackMixingNMSU)

void DoTrackMixing(const int run_id, const string dir_in, const string fn_list, const int n_evt)
{
  vector<string> list_in;
  ifstream ifs(fn_list);
  if (!ifs.is_open()) {
    std::cout<<"___file "<<fn_list<<" could not be opened"<<std::endl;
    std::exit(1);
  }
  int spill;
  string fname;
  while (ifs >> spill >> fname) list_in.push_back(dir_in+"/"+fname);
  ifs.close();
  
  auto ana = new AnaSortMixVertex();
  ana->Init(run_id);
  ana->Analyze(run_id ,list_in);
  ana->End();
  delete ana;
  exit(0);
}
