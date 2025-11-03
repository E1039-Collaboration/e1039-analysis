
// 103: J/psi mass, BG = NIM3 e906 hits
// 113: J/psi mass, BG = full bkg,
// 119: 2.0-9.0 GeV, BG = full bkg, normal  KMag pol
// 123: 2.5-3.5 GeV, BG = full bkg, reverse KMag pol
// 129: 2.0-9.0 GeV, BG = full bkg, reverse KMag pol
// 120: 2.0-9.0 GeV, BG = full bkg, normal  KMag pol, H1X y-gap
// 130: 2.0-9.0 GeV, BG = full bkg, reverse KMag pol, H1X y-gap
// 131: 2.0-9.0 GeV, BG = full bkg, normal  KMag pol, H1X y-gap, symmetric
// 132: 2.0-9.0 GeV, BG = full bkg, reverse KMag pol, H1X y-gap, symmetric
// 133: 2.0-9.0 GeV, BG = full bkg, normal  KMag pol, H1X y-gap, symmetric, not hot-road removal
// 134: 2.0-9.0 GeV, BG = full bkg, reverse KMag pol, H1X y-gap, symmetric, not hot-road removal
// 135: 2.0-9.0 GeV, BG = full bkg, zero    KMag, H1X y-gap, symmetric, not hot-road removal
// 136: 2.0-9.0 GeV, BG = full bkg, normal 50% KMag, H1X y-gap, symmetric

void GetParams(string& rs_id, double& mass_lo, double& mass_hi, int& inte_cut, double& frac_cut, string& list_sig, string& list_bg)
{
  rs_id    = "131"; // Select a roadset here to analyze.
  list_sig = "list_signal.txt";
  list_bg  = "list_bg.txt";
  
  if      (rs_id == "102") {
    mass_lo  = 4.0;
    mass_hi  = 5.0;
    inte_cut = 1200;
    frac_cut = 0.010;
  } else if (rs_id == "103") {
    mass_lo  = 2.5;
    mass_hi  = 3.5;
    inte_cut = /*1200*/10.e3;
    frac_cut = 0.040;
    list_bg  =  "list_bg_NIM3e906.txt";
  } else if (rs_id == "104") {
    mass_lo  = 2.5;
    mass_hi  = 3.5;
    inte_cut = 1200;
    frac_cut = 0.033;
  } else if (rs_id == "105") {
    mass_lo  = 7.0;
    mass_hi  = 8.0;
    inte_cut = 1200;
    frac_cut = 0.010;
  } else if (rs_id == "113") {
    mass_lo  = 2.5;
    mass_hi  = 3.5;
    inte_cut = 42000;
    frac_cut = 0.030;
    list_bg  = "list_bg_fullsimRun06.txt";
  } else if (rs_id == "119") {
    mass_lo  = 2.0;
    mass_hi  = 9.0;
    inte_cut = 42000;
    frac_cut = 0.040;
    list_bg  =  "list_bg_fullsimRun06.txt";
  } else if (rs_id == "120") {
    mass_lo  = 2.0;
    mass_hi  = 9.0;
    inte_cut = 42000;
    frac_cut = 0.043;
    list_bg  =  "list_bg_fullsimRun06.txt";
  } else if (rs_id == "123") { //J/Psi roadset for reverse KMAG polarity
    mass_lo  = 2.5;
    mass_hi  = 3.5;
    inte_cut = 43000;
    frac_cut = 0.0925; 
    list_sig = "list_signal_reverseKMAG.txt";    
    list_bg  = "list_bg_fullsimRun06_reverseKMAG.txt";
  } else if (rs_id == "129") {
    mass_lo  = 2.0;
    mass_hi  = 9.0;
    inte_cut = 42000;
    frac_cut = 0.12;
    list_sig = "list_signal_reverseKMAG.txt";
    list_bg  = "list_bg_fullsimRun06_reverseKMAG.txt";
  } else if (rs_id == "130") {
    mass_lo  = 2.0;
    mass_hi  = 9.0;
    inte_cut = 42000;
    frac_cut = 0.10;
    list_sig = "list_signal_reverseKMAG.txt";
    list_bg  = "list_bg_fullsimRun06_reverseKMAG.txt";
  } else if (rs_id == "131") {
    mass_lo  = 2.0;
    mass_hi  = 9.0;
    inte_cut = 42000;
    frac_cut = 0.086; // 0.100 too larger, 0.080, too small?
    list_bg  =  "list_bg_fullsimRun06.txt";
  } else if (rs_id == "132") {
    mass_lo  = 2.0;
    mass_hi  = 9.0;
    inte_cut = 42000;
    frac_cut = 0.36;
    list_sig = "list_signal_reverseKMAG.txt";
    list_bg  = "list_bg_fullsimRun06_reverseKMAG.txt";
  } else if (rs_id == "133") {
    mass_lo  = 2.0;
    mass_hi  = 9.0;
    inte_cut = 1000; // Does not matter
    frac_cut = 0;
    list_bg  =  "list_bg_fullsimRun06.txt";
  } else if (rs_id == "134") {
    mass_lo  = 2.0;
    mass_hi  = 9.0;
    inte_cut = 1000; // Does not matter
    frac_cut = 0;
    list_sig = "list_signal_reverseKMAG.txt";
    list_bg  = "list_bg_fullsimRun06_reverseKMAG.txt";
  } else if (rs_id == "135") {
    mass_lo  = 2.0;
    mass_hi  = 9.0;
    inte_cut = 0; // Does not matter
    frac_cut = 0;
    list_sig = "list_signal_zeroKMAG.txt";
    //list_bg  = "list_bg_fullsimRun06_reverseKMAG.txt"; // Does not matter
  } else if (rs_id == "136") {
    mass_lo  = 2.0;
    mass_hi  = 9.0;
    inte_cut = 80500; // with 45000
    frac_cut = 0.140; // 0.140 too small, 0.141 too large
    list_sig = "list_signal_KMagNormal050.txt";
    list_bg  = "list_bg_KMagNormal050.txt";
  }
}
