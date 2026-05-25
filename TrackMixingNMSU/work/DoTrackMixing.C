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
  //initializing the reco constants used during original reconstruction	
  recoConsts* rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", run_id);
  rc->set_DoubleFlag("FMAGSTR", -1.044);
  rc->set_DoubleFlag("KMAGSTR", -1.025);
  rc->set_DoubleFlag("X_BEAM", -0.2); 
  rc->set_BoolFlag("COARSE_MODE", false);
  rc->set_BoolFlag("REQUIRE_MUID", false);
  rc->set_CharFlag("HIT_MASK_MODE", "X");
  
  rc->set_CharFlag("AlignmentMille", "$E1039_RESOURCE/alignment/run0/align_mille_v10_a.txt");
  rc->set_CharFlag("AlignmentHodo", "");
  rc->set_CharFlag("AlignmentProp", "");
  rc->set_CharFlag("Calibration", "");
  rc->set_IntFlag ("MaxHitsDC0" , 350);
  rc->set_IntFlag ("MaxHitsDC1" , 350);
  rc->set_IntFlag ("MaxHitsDC2" , 170);
  rc->set_IntFlag ("MaxHitsDC3p", 140);
  rc->set_IntFlag ("MaxHitsDC3m", 140);
  rc->set_DoubleFlag("RejectWinDC0" , 0.3);
  rc->set_DoubleFlag("RejectWinDC1" , 0.5);
  rc->set_DoubleFlag("RejectWinDC2" , 0.35);
  rc->set_DoubleFlag("RejectWinDC3p", 0.24);
  rc->set_DoubleFlag("RejectWinDC3m", 0.24);

  rc->set_CharFlag("GeomPlane", "20260050501"); // Only for a test
  
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
