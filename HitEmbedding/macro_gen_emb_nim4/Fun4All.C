//R__LOAD_LIBRARY(libfun4all)
R__LOAD_LIBRARY(libcalibrator)
R__LOAD_LIBRARY(libana_embedding)
using namespace std;

int Fun4All(const int run, const string fname, const string list_name, const int n_evt=0)
{
  recoConsts* rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", run);
  rc->set_CharFlag("AlignmentMille", "$E1039_RESOURCE/alignment/run0/align_mille_v10_a.txt");
  rc->set_CharFlag("AlignmentHodo", "");
  rc->set_CharFlag("AlignmentProp", "");
  rc->set_CharFlag("Calibration", "");
  
  Fun4AllServer *se = Fun4AllServer::instance();

  CalibHitElementPos* cal_ele_pos = new CalibHitElementPos();
  cal_ele_pos->CalibTriggerHit(false);
  se->registerSubsystem(cal_ele_pos);

  CalibDriftDist* cal_dd = new CalibDriftDist();
  //cal_dd->Verbosity(999);
  //cal_dd->DeleteOutTimeHit();
  se->registerSubsystem(cal_dd);

  se->registerSubsystem(new GenEmbeddingDataNim4(list_name));

  Fun4AllInputManager* in = new Fun4AllDstInputManager("DSTIN");
  se->registerInputManager(in);
  in->fileopen(fname);
  se->run(n_evt);
  se->End();
  std::cout << "All done" << std::endl;
  delete se;
  gSystem->Exit(0);
  return 0;
}
