R__LOAD_LIBRARY(libdimu_ana_rus)
R__LOAD_LIBRARY(libcalibrator)
R__LOAD_LIBRARY(libfun4all)
R__LOAD_LIBRARY(libktracker)
R__LOAD_LIBRARY(fun4all_rus_file_manager)

int Fun4Reco(const int nevent = 10)
{
    // --- Run and geometry parameters ---
    const double FMAGSTR = -1.044;
    const double KMAGSTR = -1.025;
	const bool is_mc = true;

    recoConsts* rc = recoConsts::instance();
    rc->set_IntFlag("RUNNUMBER", 5433);
    rc->set_DoubleFlag("FMAGSTR", FMAGSTR);
    rc->set_DoubleFlag("KMAGSTR", KMAGSTR);
    rc->set_DoubleFlag("SIGX_BEAM", 0.3);
    rc->set_DoubleFlag("SIGY_BEAM", 0.3);
    rc->set_DoubleFlag("Z_UPSTREAM", -700.);

    // --- Fun4All server setup ---
    Fun4AllServer* se = Fun4AllServer::instance();
    se->setRun(5433);

    // --- Hit calibration ---
    CalibHitElementPos* cal_ele_pos = new CalibHitElementPos();
    cal_ele_pos->CalibTriggerHit(false);
    se->registerSubsystem(cal_ele_pos);

    // --- Tracking reconstruction ---
    SQReco* reco = new SQReco();
    reco->Verbosity(1);
    reco->set_legacy_rec_container(false);
    reco->set_geom_file_name(
        (string)gSystem->Getenv("E1039_RESOURCE") + "/geometry/geom_run005433.root"
    );
    reco->set_enable_KF(true);
    reco->setInputTy(SQReco::E1039);
    reco->setFitterTy(SQReco::KFREF);
    reco->set_evt_reducer_opt("none");
    reco->set_enable_eval_dst(true);
    for (int ii = 0; ii <= 3; ii++) reco->add_eval_list(ii);
    reco->set_enable_eval(true);
    se->registerSubsystem(reco);

    // --- Vertexing ---
    SQVertexing* vtx = new SQVertexing();
    se->registerSubsystem(vtx);

    /*

    Fun4AllInputManager *in = new Fun4AllDstInputManager("SimMicroDst");
    se->registerInputManager(in);
    const char* fn_udst="/seaquest/users/mhossain/DimuAnaRUS/mc_gen/data/MC_JPsi_Pythia8_Target_Sep26/100/out/DST.root";
    in->fileopen(fn_udst);
    */

    
    // --- Input Manager ---
    Fun4AllRUSInputManager* in = new Fun4AllRUSInputManager("VectIn");
    in->set_tree_name("tree");
    //in->fileopen("MC_BKG_Tuned.root");
    in->fileopen("/seaquest/users/mhossain/DimuAnaRUS/mc_gen/RUS_out.root");
    se->registerInputManager(in);
    


    // DST output manager
    //Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", "DST.root");
    //se->registerOutputManager(out);

    // --- Output Manager ---
    Fun4AllRUSOutputManager* tree = new Fun4AllRUSOutputManager();
    tree->SetTreeName("tree");
    tree->SetFileName("RUS_out2.root");
    se->registerOutputManager(tree);

    // --- Dimuon analysis ---
    DimuAnaRUS* dimuAna = new DimuAnaRUS();
    dimuAna->SetTreeName("tree");
    dimuAna->SetMCTrueMode(is_mc);  // keep consistent with input
    dimuAna->SetOutputFileName("RUS_reco.root");
    dimuAna->SetSaveOnlyDimuon(true);
    dimuAna->SetRecoMode(true);
    dimuAna->SetRecoDimuMode(true);
    se->registerSubsystem(dimuAna);

    // --- Run ---
    se->run(nevent);
    se->End();
    delete se;
    return 0;
}

