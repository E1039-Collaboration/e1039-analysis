/// Fun4SimMicroDst.C:  Fun4all macro to analyze your uDST.
//R__LOAD_LIBRARY(libktracker)
R__LOAD_LIBRARY(libana_sim_dst)

int Fun4SimMicroDst(const char* fn_udst="uDST.root")
{
  recoConsts *rc = recoConsts::instance();
  rc->init("Run1");
  
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllInputManager *in = new Fun4AllDstInputManager("SimMicroDst");
  se->registerInputManager(in);
  in->fileopen(fn_udst);

  //SQReco* reco = new SQReco();
  //reco->Verbosity(0);
  //reco->set_legacy_rec_container(false);
  //reco->set_geom_file_name("geom_20251209.root");
  //reco->set_enable_KF(true);
  //reco->setInputTy(SQReco::E1039);
  //reco->setFitterTy(SQReco::KFREF);
  //reco->set_evt_reducer_opt("none");
  //se->registerSubsystem(reco);
  //
  //SQVertexing* vtx = new SQVertexing();
  //se->registerSubsystem(vtx);
  
  auto ana = new AnaTrueDimuon();
  //auto ana = new AnaRecoDimuon();
  //ana->SetRoadsetID(131);
  se->registerSubsystem(ana);

  se->run();
  se->End();
  delete se;
  exit(0);//return 0;
}
