#ifndef __UTIL_SLOW_CONT_H__
#define __UTIL_SLOW_CONT_H__
#include <string>
#include <unordered_map>
class DbSvc;

class UtilSlowCont {
  static UtilSlowCont* m_inst;
  int m_spill_id_window;
  DbSvc* m_db;

  typedef std::unordered_map<int, double> Spill2Value_t;
  std::unordered_map<std::string, Spill2Value_t> m_list_acnet;
  std::unordered_map<std::string, Spill2Value_t> m_list_beamdaq;
  
  UtilSlowCont();
  
public:
  virtual ~UtilSlowCont() {;}
  static UtilSlowCont* instance();

  void SetSpillIDWindow(const int window) { m_spill_id_window = window; }
  int  GetSpillIDWindow() const    { return m_spill_id_window; }

  bool GetSpillTime(const int spill_id, int& utime, double& lvtime);
  double GetAcnet(const std::string name, const int spill_id);
  double GetBeamDAQ(const std::string name, const int spill_id);

protected:
  DbSvc* GetDb();
};

#endif // __UTIL_SLOW_CONT_H__
