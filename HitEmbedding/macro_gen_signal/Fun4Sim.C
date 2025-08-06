#include <top/G4_Beamline.C>
#include <top/G4_Target.C>
#include <top/G4_InsensitiveVolumes.C>
#include <top/G4_SensitiveDetectors.C>
R__LOAD_LIBRARY(libfun4all)
R__LOAD_LIBRARY(libg4detectors)
R__LOAD_LIBRARY(libg4testbench)
R__LOAD_LIBRARY(libg4eval)
R__LOAD_LIBRARY(libg4dst)
R__LOAD_LIBRARY(libktracker)
R__LOAD_LIBRARY(libSQPrimaryGen)
using namespace std;

int Fun4Sim(const string gen_mode="dy", const int job_id=0, const int n_evt=0)
{
  recoConsts *rc = recoConsts::instance();
  Fun4AllServer *se = Fun4AllServer::instance();

  ///
  /// Global parameters
  ///
  rc->set_IntFlag("RUNNUMBER", 5433); /// The geometry is selected based on run number.
  rc->set_DoubleFlag("FMAGSTR", -1.044);
  rc->set_DoubleFlag("KMAGSTR", -1.025);
  rc->set_DoubleFlag("SIGX_BEAM", 0.3);
  rc->set_DoubleFlag("SIGY_BEAM", 0.3);
  rc->set_BoolFlag("COARSE_MODE", false);
  rc->set_BoolFlag("REQUIRE_MUID", false);
  rc->set_CharFlag("HIT_MASK_MODE", "X");
  rc->set_CharFlag("AlignmentMille", "$E1039_RESOURCE/alignment/run0/align_mille_v10_a.txt");
  rc->set_CharFlag("AlignmentHodo", "");
  rc->set_CharFlag("AlignmentProp", "");
  rc->set_CharFlag("Calibration", "");
  rc->set_CharFlag("VTX_GEN_MATERIAL_MODE", "Target");

  ///
  /// Event generator
  ///
  SQPrimaryParticleGen* sq_gen = new SQPrimaryParticleGen();
  if (gen_mode == "dy") { // 5000 events = 12 hours, 3000 events = 7 hours
    sq_gen->set_massRange(1.5, 8.0);
    sq_gen->set_xfRange(0.05, 0.95);
    sq_gen->enableDrellYanGen();
  } else if (gen_mode == "jpsi") { // 5000 events = 20 h,  4000 events = 16 h
    sq_gen->set_xfRange(0.2, 1.0);
    sq_gen->enableJPsiGen();
  } else if (gen_mode == "psip") { // same as jpsi
    sq_gen->set_xfRange(0.2, 1.0);
    sq_gen->enablePsipGen();
  } else {
    cout << "Unknown gen_mode (" << gen_mode << ").  Abort." << endl;
    exit(1);
  }
  se->registerSubsystem(sq_gen);

  ///
  /// Detector setting
  ///
  PHG4Reco *g4Reco = new PHG4Reco();
  g4Reco->set_field_map();
  g4Reco->SetWorldSizeX(1000);
  g4Reco->SetWorldSizeY(1000);
  g4Reco->SetWorldSizeZ(5000);
  g4Reco->SetWorldShape("G4BOX");
  g4Reco->SetWorldMaterial("G4_AIR");
  g4Reco->SetPhysicsList("FTFP_BERT");

  SetupInsensitiveVolumes(g4Reco);
  SetupBeamline(g4Reco);
  SetupTarget(g4Reco);
  SetupSensitiveDetectors(g4Reco);

  se->registerSubsystem(g4Reco);

  PHG4TruthSubsystem *truth = new PHG4TruthSubsystem();
  g4Reco->registerSubsystem(truth);

  /// digitizer
  SQDigitizer *digitizer = new SQDigitizer("DPDigitizer", 0);
  se->registerSubsystem(digitizer);

  /// Save only events that are in the geometric acceptance.
  SQGeomAcc* geom_acc = new SQGeomAcc();
  geom_acc->SetMuonMode(SQGeomAcc::PAIR_TBBT);
  geom_acc->SetPlaneMode(SQGeomAcc::HODO_CHAM);
  geom_acc->SetNumOfH1EdgeElementsExcluded(4);
  se->registerSubsystem(geom_acc);

  auto cal_cham_real = new SQChamberRealization();
  //cal_cham_real->Verbosity(10);
  cal_cham_real->FixChamReso(0.04, 0.04, 0.04, 0.04, 0.04);
  se->registerSubsystem(cal_cham_real);
  
  // Make SQ nodes for truth info
  TruthNodeMaker* tnm = new TruthNodeMaker();
  tnm->SetJobID(job_id);
  se->registerSubsystem(tnm);

  ///
  /// Reconstruction
  ///
  SQReco* reco = new SQReco();
  reco->set_legacy_rec_container(false);
  reco->set_enable_KF(true);
  reco->setInputTy(SQReco::E1039);
  reco->setFitterTy(SQReco::KFREF);
  reco->set_evt_reducer_opt("none");
  se->registerSubsystem(reco);

  SQVertexing* vtx = new SQVertexing();
  se->registerSubsystem(vtx);

  ///
  /// Input, output and execution
  ///
  Fun4AllInputManager* man_in = new Fun4AllDummyInputManager("DUMMY");
  se->registerInputManager(man_in);

  Fun4AllDstOutputManager *man_out = new Fun4AllDstOutputManager("DSTOUT", "DST.root");
  se->registerOutputManager(man_out);
  man_out->AddNode("SQEvent");
  man_out->AddNode("SQHitVector");
  man_out->AddNode("SQRecTrackVector");
  man_out->AddNode("SQRecDimuonVector_PM");
  man_out->AddNode("SQMCEvent");
  man_out->AddNode("SQTruthTrackVector");
  man_out->AddNode("SQTruthDimuonVector");

  const bool count_only_good_events = true;
  se->run(n_evt, count_only_good_events);
  
  se->End();
  se->PrintTimer();
  rc->WriteToFile("recoConsts.tsv");
  std::cout << "All done" << std::endl;
  delete se;
  gSystem->Exit(0);
  return 0;
}
