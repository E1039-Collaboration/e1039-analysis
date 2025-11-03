/// Fun4SimMicroDst.C:  Fun4all macro to analyze you uDST.
R__LOAD_LIBRARY(libana_sim_dst)

int Fun4SimMicroDst(const char* fn_udst="uDST.root")
{
  Fun4AllServer* se = Fun4AllServer::instance();
  //se->Verbosity(1);
  Fun4AllInputManager *in = new Fun4AllDstInputManager("SimMicroDst");
  se->registerInputManager(in);
  in->fileopen(fn_udst);

  auto ana = new AnaSimDst();
  ana->SetRoadsetID(131);
  se->registerSubsystem(ana);

  se->run();
  se->End();
  delete se;
  exit(0); // return 0;
}
