#include <top/G4_Beamline.C>
#include <top/G4_Target.C>
#include <top/G4_InsensitiveVolumes.C>
#include <top/G4_SensitiveDetectors.C>
R__LOAD_LIBRARY(libfun4all)
R__LOAD_LIBRARY(libg4detectors)
R__LOAD_LIBRARY(libg4eval)
R__LOAD_LIBRARY(libg4dst)
R__LOAD_LIBRARY(libktracker)
//R__LOAD_LIBRARY(fun4all_rus_file_manager)
R__LOAD_LIBRARY(libJpsiGen)
using namespace std;

int Fun4All(const string tgt="proton", const int nevent=10)
{
  recoConsts *rc = recoConsts::instance();
  rc->init("Run1");
  rc->set_CharFlag("VTX_GEN_MATERIAL_MODE", "Target");
  rc->Print();

  Fun4AllServer *se = Fun4AllServer::instance();
  se->Verbosity(0);

  ///
  /// Event Generator
  ///
  JpsiNRQCDGen* gen_jpsi = new JpsiNRQCDGen();
  if      (tgt == "proton"  ) gen_jpsi->SetTarget(JpsiNRQCDGen::PROTON);
  else if (tgt == "deuteron") gen_jpsi->SetTarget(JpsiNRQCDGen::DEUTERON);
  else if (tgt == "nitrogen") gen_jpsi->SetTarget(JpsiNRQCDGen::NITROGEN);
  else { cout << "Unknown taret.  Abort.\n"; exit(1);  }
  gen_jpsi->SetPDF(JpsiNRQCDGen::CTEQ); // CTEQ, NNPDF
  gen_jpsi->SetLDME(JpsiNRQCDGen::SMRS_FIT); // SMRS_REF, SMRS_FIT, etc.
  gen_jpsi->set_xfRange(-0.1, 0.95); // default = (-0.95, 0.95)
  gen_jpsi->set_pT0(1.69); // <pT^2>=p0^2/4=0.714 in PLB 858, 139032.
  se->registerSubsystem(gen_jpsi);
  
  ///
  /// Detector Simulation
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

  SQDigitizer *digitizer = new SQDigitizer("DPDigitizer", 0);
  se->registerSubsystem(digitizer);

  /// Save only events that are in the geometric acceptance.
  SQGeomAcc* geom_acc = new SQGeomAcc();
  geom_acc->SetMuonMode(SQGeomAcc::PAIR_TBBT); // PAIR, PAIR_TBBT, SINGLE, SINGLE_T, etc.
  geom_acc->SetPlaneMode(SQGeomAcc::HODO_CHAM); // HODO, CHAM or HODO_CHAM
  geom_acc->SetNumOfH1EdgeElementsExcluded(4); // Exclude 4 elements at H1 edges
  se->registerSubsystem(geom_acc);

  // Make SQ nodes for truth info
  se->registerSubsystem(new TruthNodeMaker());

  ///
  /// Reconstruction
  ///
  SQReco* reco = new SQReco();
  reco->Verbosity(0);
  reco->set_legacy_rec_container(false); // default = true
  //reco->set_geom_file_name((string)gSystem->Getenv("E1039_RESOURCE") + "/geometry/geom_run005433.root");
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
  Fun4AllInputManager *in = new Fun4AllDummyInputManager("DUMMY");
  se->registerInputManager(in);

  Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", "DST.root");
  se->registerOutputManager(out);

  //Fun4AllRUSOutputManager* out2 = new Fun4AllRUSOutputManager();
  //out2->SetFileName("RUS.root");
  //se->registerOutputManager(out2);
  
  const bool count_only_good_events = true;
  se->run(nevent, count_only_good_events);

  //PHGeomUtility::ExportGeomtry(se->topNode(),"geom.root");
  se->End();
  se->PrintTimer();
  rc->WriteToFile("recoConsts.tsv");
  std::cout << "All done" << std::endl;
  delete se;
  gSystem->Exit(0);
  return 0;
}
