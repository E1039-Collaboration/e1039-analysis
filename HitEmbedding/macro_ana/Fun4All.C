R__LOAD_LIBRARY(libana_embedding)

int Fun4All(const int n_dst_ana=0, const char* fn_list_dst="list_dst.txt")
{
  recoConsts *rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", 5433); /// The geometry is selected based on run number.
  rc->set_IntFlag("ROADSET_ID", 131); /// We have to manually select it...
  rc->set_CharFlag("AlignmentMille", "$E1039_RESOURCE/alignment/run0/align_mille_v10_a.txt");
  rc->set_CharFlag("AlignmentHodo", "");
  rc->set_CharFlag("AlignmentProp", "");
  rc->set_CharFlag("Calibration", "");
  
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllInputManager* man_in = new Fun4AllDstInputManager("DSTIN");
  se->registerInputManager(man_in);

  se->registerSubsystem(new AnaEmbeddedData());

  vector<string> list_dst;
  string fn_dst;
  ifstream ifs(fn_list_dst);
  while (ifs >> fn_dst) list_dst.push_back(fn_dst);
  ifs.close();

  int n_dst = list_dst.size();
  cout << "N of DSTs: all = " << n_dst;
  if (n_dst_ana > 0 && n_dst > n_dst_ana) n_dst = n_dst_ana;
  cout << ", to be analyzed = " << n_dst << endl;
  for (int i_dst = 0; i_dst < n_dst; i_dst++) {
    string fn_dst = list_dst[i_dst];
    cout << "DST: " << i_dst+1 << "/" << n_dst << ": " << fn_dst << endl;
    man_in->fileopen(fn_dst);
    se->run();
  }

  se->End();
  delete se;
  exit(0);
}
