R__LOAD_LIBRARY(libcalibrator)
//R__LOAD_LIBRARY(libktracker)
R__LOAD_LIBRARY(RecoData2024)

/// Fun4All macro to analyze spill-by-spill DST files of multiple runs.
int Fun4All(const int run_id, const string dir_in, const string fn_list, const int n_evt=0)
{
  recoConsts* rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", run_id);
  rc->set_DoubleFlag("FMAGSTR", -1.044);
  rc->set_DoubleFlag("KMAGSTR", -1.025);
  rc->set_CharFlag("AlignmentMille", "$E1039_RESOURCE/alignment/run0/align_mille_v10_a.txt");  
  rc->set_CharFlag("AlignmentHodo", "");
  rc->set_CharFlag("AlignmentProp", "");
  rc->set_CharFlag("Calibration", "");
  
  Fun4AllServer* se = Fun4AllServer::instance();
  se->setRun(run_id);
  //se->Verbosity(1);

  //CalibHodoInTime* cal_hodo = new CalibHodoInTime();
  //cal_hodo->SkipCalibration();
  //cal_hodo->DeleteOutTimeHit();
  //se->registerSubsystem(cal_hodo);
  
  //SQVertexing* vtx = new SQVertexing();
  //vtx->Verbosity(99);
  //vtx->set_geom_file_name((string)gSystem->Getenv("E1039_RESOURCE") + "/geometry/geom_run005433.root");
  //se->registerSubsystem(vtx);
  
  se->registerSubsystem(new AnaDimuon());
  se->registerSubsystem(new AnaDimuon("AnaDimuonPP", "PP"));
  se->registerSubsystem(new AnaDimuon("AnaDimuonMM", "MM"));
  
  //se->registerSubsystem(new AnaTrack());
  //se->registerSubsystem(new AnaTriggerAndTrack());

  vector<string> list_in;
  ifstream ifs(fn_list);
  int spill;
  string bname;
  while (ifs >> spill >> bname) list_in.push_back(bname);
  ifs.close();

  Fun4AllInputManager *in = new Fun4AllDstInputManager("DST");
  se->registerInputManager(in);
  for (unsigned int ii = 0; ii < list_in.size(); ii++) {
    cout << "Input (" << ii+1 << "/" << list_in.size() << "): " << list_in[ii] << endl;
    string fname = dir_in + "/" + list_in[ii] + ".root";
    in->fileopen(fname);
    se->run(n_evt);
    in->fileclose();
  }
  se->End();
  delete se;
  exit(0); //return 0;
}
