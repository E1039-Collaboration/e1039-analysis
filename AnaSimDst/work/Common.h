#ifndef _COMMON__H_
#define _COMMON__H_

double GetInteLumi(const char* fn_lumi="lumi_tot.txt")
{
  ifstream ifs(fn_lumi);
  if (!ifs.is_open()) {
    cout << "GetInteLumi():  Cannot open '" << fn_lumi << "'.  Just return 1.0." << endl;
    return 1.0;
  }
  double val;
  ifs >> val;
  ifs.close();
  return val;
}

#endif // _COMMON__H_
