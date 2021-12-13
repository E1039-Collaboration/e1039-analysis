/// Fun4AllReco.C:  Fun4all macro to run the reconstruction.
// /seaquest/users/apun/abi_project/data_manage/e1039-data-mgt_test/RecoE1039Data.C
R__LOAD_LIBRARY(libktracker)
R__LOAD_LIBRARY(libCalibChamXT)

int Fun4AllReco(const char* fn_dst, const int n_evt)
{
  recoConsts* rc = recoConsts::instance();
  rc->init("cosmic");
  rc->set_BoolFlag("COARSE_MODE", true);
  rc->set_DoubleFlag("KMAGSTR", 0.);
  rc->set_DoubleFlag("FMAGSTR", 0.);
  rc->Print();

  Fun4AllServer* se = Fun4AllServer::instance();
  //se->Verbosity(1);

  FilterByTrigger* fbt = new FilterByTrigger();
  fbt->SetTriggerBits( (0x1<<SQEvent::NIM4) );
  fbt->EnableOutput("event_count.root");
  se->registerSubsystem(fbt);

  const bool legacy_rec_container = false;
  SQReco* reco = new SQReco();
  reco->Verbosity(0);
  reco->set_legacy_rec_container(legacy_rec_container);
  reco->set_geom_file_name("geom.root");
  reco->set_enable_KF(true);
  reco->setInputTy(SQReco::E1039);
  reco->setFitterTy(SQReco::KFREF);
  reco->set_evt_reducer_opt("e");
  reco->set_enable_eval(true);
  reco->set_enable_eval_dst(true);
  reco->set_eval_file_name("eval.root");
  reco->add_eval_list(0); // D0, D1
  reco->add_eval_list(1); // D2
  reco->add_eval_list(2); // D3p, D3m
  reco->add_eval_list(3); // D2+3, back-partial tracklets
  reco->add_eval_list(4); // D1+2+3
  se->registerSubsystem(reco);

  Fun4AllInputManager *in = new Fun4AllDstInputManager("DSTIN");
  in->Verbosity(0);
  in->fileopen(fn_dst);
  se->registerInputManager(in);

  Fun4AllDstOutputManager* out = new Fun4AllDstOutputManager("DSTOUT", "DSTreco.root");
  se->registerOutputManager(out);

  se->run(n_evt);
  se->End();
  //se->PrintTimer();
  delete se;
  return 0;
}