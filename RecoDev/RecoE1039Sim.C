#include <TSystem.h>

#include <top/G4_Beamline.C>
#include <top/G4_Target.C>
#include <top/G4_InsensitiveVolumes.C>
#include <top/G4_SensitiveDetectors.C>

R__LOAD_LIBRARY(libfun4all)
R__LOAD_LIBRARY(libPHPythia8)
R__LOAD_LIBRARY(libg4detectors)
R__LOAD_LIBRARY(libg4testbench)
R__LOAD_LIBRARY(libg4eval)
R__LOAD_LIBRARY(libg4dst)
R__LOAD_LIBRARY(libdptrigger)
R__LOAD_LIBRARY(libktracker)
R__LOAD_LIBRARY(libevt_filter)
R__LOAD_LIBRARY(libanamodule)

using namespace std;

/*
This is an example script intended to demonstrate how to run SQReco in a minimalistic fashion, it is NOT
suitable for production use and users should develop their own reconstruction macro for their own analysis.
*/

int RecoE1039Sim(const int nevent = 10, TString prefix = "run", int seed = 12345)
{
  //! particle gen flag
  const bool cosmic = false;
  const bool dimuon = false && (!cosmic);
  const bool single = (!dimuon) && (!cosmic);
  const bool jpsi   = false;
  
  //! vtx gen flag
  const bool legacyVtxGen = true;

  const bool legacy_rec_container = false;

  const bool do_collimator = true;
  const bool do_target     = true;
  const bool do_shielding  = true;
  const bool do_fmag       = true;
  const bool do_kmag       = true;
  const bool do_absorber   = true;

  const double collimator_pos_z = -602.36;
  const double target_coil_pos_z = -300.;
  const double target_l = 7.9; //cm
  const double target_z = (7.9-target_l)/2.; //cm

  const double FMAGSTR = -1.054;
  const double KMAGSTR = -0.951;

  recoConsts *rc = recoConsts::instance();
  rc->set_DoubleFlag("FMAGSTR", FMAGSTR);
  rc->set_DoubleFlag("KMAGSTR", KMAGSTR);
  rc->set_IntFlag("RANDOMSEED", seed);
  rc->set_DoubleFlag("SIGX_BEAM", .3);
  rc->set_DoubleFlag("SIGY_BEAM", .3);
  if(cosmic)
  {
    rc->init("cosmic");
    rc->set_BoolFlag("COARSE_MODE", true);
    rc->set_DoubleFlag("KMAGSTR", 0.);
    rc->set_DoubleFlag("FMAGSTR", 0.);
  }

  //! flag to choose target only or dump only for vtx gen
  if(legacyVtxGen)
  {
    rc->set_BoolFlag("TARGETONLY", false);
    rc->set_BoolFlag("DUMPONLY", false);
  }

  rc->Print();

  GeomSvc* geom_svc = GeomSvc::instance();
  geom_svc->printTable();

  Fun4AllServer *se = Fun4AllServer::instance();
  se->Verbosity(0);

  if(dimuon)  //change the pythia configuration file to change the dimuons generated
  {
    PHPythia8 *pythia8 = new PHPythia8();
    //pythia8->Verbosity(99);
    if(jpsi) 
      pythia8->set_config_file("support/phpythia8_Jpsi.cfg");
    else
      pythia8->set_config_file("support/phpythia8_DY.cfg");

    if (legacyVtxGen) pythia8->enableLegacyVtxGen();
    else{
      pythia8->set_vertex_distribution_mean(0, 0, target_coil_pos_z, 0);
    }
    se->registerSubsystem(pythia8);

    pythia8->set_trigger_AND();

    PHPy8ParticleTrigger* trigger_mup = new PHPy8ParticleTrigger();
    trigger_mup->AddParticles("-13");
    trigger_mup->SetPzHighLow(120, 15); 
    pythia8->register_trigger(trigger_mup);

    PHPy8ParticleTrigger* trigger_mum = new PHPy8ParticleTrigger();
    trigger_mum->AddParticles("13");
    trigger_mum->SetPzHighLow(120, 15); 
    pythia8->register_trigger(trigger_mum);

    HepMCNodeReader* hr = new HepMCNodeReader();
    hr->set_particle_filter_on(true);
    hr->insert_particle_filter_pid(13);
    hr->insert_particle_filter_pid(-13);
    se->registerSubsystem(hr);
  }

  if(single)   //change the hard-coded numbers to change the initial vertex/momentum distribution
  {
    PHG4SimpleEventGenerator* genp = new PHG4SimpleEventGenerator("MUP"); 
    genp->add_particles("mu+", 1);  // mu+,e+,proton,pi+,Upsilon
    if (legacyVtxGen) genp->enableLegacyVtxGen();
    else{
      genp->set_vertex_distribution_function(PHG4SimpleEventGenerator::Uniform, PHG4SimpleEventGenerator::Uniform, PHG4SimpleEventGenerator::Uniform);
      genp->set_vertex_distribution_mean(0.0, 0.0, target_coil_pos_z);
      genp->set_vertex_distribution_width(0.0, 0.0, 0.0);
      genp->set_vertex_size_function(PHG4SimpleEventGenerator::Uniform);
      genp->set_vertex_size_parameters(0.0, 0.0);
    }
    genp->set_pxpypz_range(-6., 6., -3. ,3., 25., 100.);
    se->registerSubsystem(genp);
  }

  if(cosmic)
  {
    SQCosmicGen* cosmicGen = new SQCosmicGen();
    se->registerSubsystem(cosmicGen);
  }

  // Fun4All G4 module
  PHG4Reco* g4Reco = new PHG4Reco();
  //PHG4Reco::G4Seed(123);
  g4Reco->set_field_map(
      rc->get_CharFlag("fMagFile")+" "+
      rc->get_CharFlag("kMagFile")+" "+
      Form("%f",FMAGSTR) + " " +
      Form("%f",KMAGSTR) + " " +
      "5.0",
      PHFieldConfig::RegionalConst);

  // size of the world - every detector has to fit in here
  g4Reco->SetWorldSizeX(1000);
  g4Reco->SetWorldSizeY(1000);
  g4Reco->SetWorldSizeZ(5000);
  // shape of our world - it is a box
  g4Reco->SetWorldShape("G4BOX");
  // this is what our world is filled with
  g4Reco->SetWorldMaterial("G4_AIR"); //G4_Galactic, G4_AIR
  // Geant4 Physics list to use
  g4Reco->SetPhysicsList("FTFP_BERT");

  SetupBeamline(g4Reco, do_collimator, collimator_pos_z);
  SetupTarget(g4Reco, target_coil_pos_z, target_l, target_z, 1, 0);
  SetupInsensitiveVolumes(g4Reco, do_shielding, do_fmag, do_kmag, do_absorber);
  SetupSensitiveDetectors(g4Reco);
  se->registerSubsystem(g4Reco);

  // save truth info to the Node Tree
  PHG4TruthSubsystem* truth = new PHG4TruthSubsystem();
  g4Reco->registerSubsystem(truth);

  // digitizer
  SQDigitizer* digitizer = new SQDigitizer("Digitizer", 0);
  //digitizer->Verbosity(99);
  se->registerSubsystem(digitizer);

  // apply in-acceptance cut
  SQGeomAcc* geom_acc = new SQGeomAcc();
  if     (dimuon) geom_acc->SetMuonMode(SQGeomAcc::PAIR);
  else if(single) geom_acc->SetMuonMode(SQGeomAcc::SINGLE);
  geom_acc->SetPlaneMode(SQGeomAcc::HODO_CHAM);
  geom_acc->SetNumOfH1EdgeElementsExcluded(4);
  se->registerSubsystem(geom_acc);

  // trakcing module
  SQReco* reco = new SQReco();
  reco->Verbosity(0);
  reco->set_legacy_rec_container(legacy_rec_container);
  //reco->set_geom_file_name("support/geom.root"); //not needed as it's created on the fly
  reco->set_enable_KF(true);           //Kalman filter not needed for the track finding, disabling KF saves a lot of initialization time
  reco->setInputTy(SQReco::E1039);     //options are SQReco::E906 and SQReco::E1039
  reco->setFitterTy(SQReco::KFREF);    //not relavant for the track finding
  reco->set_evt_reducer_opt("none");   //if not provided, event reducer will be using JobOptsSvc to intialize; to turn off, set it to "none", for normal tracking, set to something like "aoc"
  reco->set_enable_eval(true);          //set to true to generate evaluation file which includes final track candidates 
  reco->set_eval_file_name(Form("eval_%s_%d.root", prefix.Data(), seed));
  reco->set_enable_eval_dst(false);     //set to true to include final track cnadidates in the DST tree
  if(cosmic) reco->add_eval_list(3);    //output of cosmic reco is contained in the eval output for now
  //reco->add_eval_list(3);             //include back partial tracks in eval tree for debuging
  //reco->add_eval_list(2);             //include station-3+/- in eval tree for debuging
  //reco->add_eval_list(1);             //include station-2 in eval tree for debugging
  se->registerSubsystem(reco);

  TruthNodeMaker* truthMaker = new TruthNodeMaker();
  truthMaker->set_legacy_rec_container(legacy_rec_container);
  se->registerSubsystem(truthMaker);

  SQTruthVertexing* truthVtx = new SQTruthVertexing();
  truthVtx->set_legacy_rec_container(legacy_rec_container);
  se->registerSubsystem(truthVtx);

  //A simple analysis module for single muon tracking QA
  AnaModule* ana = new AnaModule();
  ana->set_output_filename(Form("ana_%s_%d.root", prefix.Data(), seed));
  ana->set_legacy_rec_container(legacy_rec_container);
  se->registerSubsystem(ana);

  // Vertexing is not tested and probably does not work yet
  // VertexFit* vertexing = new VertexFit();
  // se->registerSubsystem(vertexing);

  // input - we need a dummy to drive the event loop
  Fun4AllInputManager *in = new Fun4AllDummyInputManager("DUMMY");
  se->registerInputManager(in);

  ///////////////////////////////////////////
  // Output
  ///////////////////////////////////////////
  // DST output manager, tunred off to save disk by default
  Fun4AllDstOutputManager* out = new Fun4AllDstOutputManager("DSTOUT", Form("DST_%s_%d.root", prefix.Data(), seed));
  se->registerOutputManager(out);

  se->run(nevent);
  PHGeomUtility::ExportGeomtry(se->topNode(), "geom.root");

  // finish job - close and save output files
  se->End();
  se->PrintTimer();
  std::cout << "All done" << std::endl;

  // cleanup - delete the server and exit
  delete se;
  gSystem->Exit(0);

  return 0;
}
