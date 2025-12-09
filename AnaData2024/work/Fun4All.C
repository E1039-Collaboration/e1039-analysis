R__LOAD_LIBRARY(calibrator)
R__LOAD_LIBRARY(AnaData2024)

/// Fun4All macro to analyze spill-by-spill DST files of multiple runs.
int Fun4All(const int run_id, const string dir_in, const string fn_list, const int n_evt=0)
{
  recoConsts* rc = recoConsts::instance();
  rc->set_CharFlag("AlignmentMille", "");
  rc->set_CharFlag("AlignmentHodo", "");
  rc->set_CharFlag("AlignmentProp", "");
  rc->set_CharFlag("Calibration", "");
  
  Fun4AllServer* se = Fun4AllServer::instance();
  se->setRun(run_id);
  //se->Verbosity(1);

  se->registerSubsystem(new CalibDriftDist());

  auto ana = new AnaBeam();
  ana->SetSpillListName("list_run_spill.txt");
  se->registerSubsystem(ana);

  //se->registerSubsystem(new AnaHodoHit());
  //se->registerSubsystem(new AnaChamHit());
  //se->registerSubsystem(new AnaTriggerHit());
  //se->registerSubsystem(new AnaTrigSignal());
  //se->registerSubsystem(new AnaHardEvent());

  vector<string> list_in;
  ifstream ifs(fn_list);
  int spill;
  string fname;
  while (ifs >> spill >> fname) list_in.push_back(dir_in+"/"+fname);
  ifs.close();

  Fun4AllInputManager *in = new Fun4AllDstInputManager("DST");
  se->registerInputManager(in);
  for (auto it = list_in.begin(); it != list_in.end(); it++) {
    cout << "Input = " << *it << endl;
    in->fileopen(*it);
    se->run(n_evt);
  }
  se->End();
  delete se;
  exit(0); //return 0;
}
