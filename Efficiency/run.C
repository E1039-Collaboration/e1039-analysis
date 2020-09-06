#include <TSystem.h>

R__LOAD_LIBRARY(libinterface_main)
R__LOAD_LIBRARY(libfun4all)
R__LOAD_LIBRARY(libg4detectors)
R__LOAD_LIBRARY(libg4eval)
R__LOAD_LIBRARY(libktracker)
R__LOAD_LIBRARY(libanamodule)

int run(const int nEvents = 1)
{
  const double FMAGSTR = -1.054;
  const double KMAGSTR = -0.951;
  const bool cosmic = true;

  recoConsts* rc = recoConsts::instance();
  rc->set_DoubleFlag("FMAGSTR", FMAGSTR);
  rc->set_DoubleFlag("KMAGSTR", KMAGSTR);
  if(cosmic)
  {
    rc->init("cosmic");
    rc->set_BoolFlag("COARSE_MODE", true);
    rc->set_DoubleFlag("KMAGSTR", 0.);
    rc->set_DoubleFlag("FMAGSTR", 0.);
  }
  rc->Print();

  GeomSvc::UseDbSvc(true);  
  GeomSvc* geom_svc = GeomSvc::instance();

  Fun4AllServer* se = Fun4AllServer::instance();
  se->Verbosity(0);

  AnaModule* ana = new AnaModule();
  ana->set_output_filename("ana.root");
  ana->registerDetector("DP1TL");     //register detector by its name, all detectors that do not directly partipate the tracking can be used
  ana->registerDetector("DP1TR");
  ana->registerDetector("DP1BL");
  ana->registerDetector("DP1BR");
  ana->registerDetector("DP2TL");
  ana->registerDetector("DP2TR");
  ana->registerDetector("DP2BL");
  ana->registerDetector("DP2BR");
  ana->registerDetector("P1X1");
  ana->registerDetector("P1X2");
  ana->registerDetector("P2X1");
  ana->registerDetector("P2X2");
  ana->registerDetector("P1Y1");
  ana->registerDetector("P1Y2");
  ana->registerDetector("P2Y1");
  ana->registerDetector("P2Y2");
  se->registerSubsystem(ana);

  Fun4AllInputManager* in = new Fun4AllDstInputManager("DSTIN");
  in->Verbosity(0);
  in->fileopen("data.root");
  se->registerInputManager(in);

  //we do not really need an output manager
  //Fun4AllDstOutputManager* out = new Fun4AllDstOutputManager("DSTOUT", "result.root");
  //se->registerOutputManager(out);

  se->run(nEvents);

  // finish job - close and save output files
  se->End();
  se->PrintTimer();
  std::cout << "All done" << std::endl;

  delete se;
  gSystem->Exit(0);
  return 0;
}
