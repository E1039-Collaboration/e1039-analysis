#include <iostream>
#include <sstream>
#include <TSQLStatement.h>
#include <db_svc/DbSvc.h>
#include "UtilSlowCont.h"
using namespace std;

UtilSlowCont* UtilSlowCont::m_inst = 0;

UtilSlowCont* UtilSlowCont::instance()
{
  if (! m_inst) m_inst = new UtilSlowCont();
  return m_inst;
}

UtilSlowCont::UtilSlowCont()
  : m_spill_id_window(100)
  , m_db(0)
{
  ;
}

bool UtilSlowCont::GetSpillTime(const int spill_id, int& utime, double& lvtime)
{
  DbSvc* db = GetDb();
  db->UseSchema("user_e1039_slow_cont");

  ostringstream oss;
  oss << "select utime, lvtime from spill_counter where spill_id = " << spill_id;
  TSQLStatement* stmt = db->Process(oss.str());
  bool found = stmt->NextResultRow();
  if (found) {
    utime  = stmt->GetInt   (0);
    lvtime = stmt->GetDouble(1);
  } else {
    utime  = 0;
    lvtime = 0;
  }
  delete stmt;
  return found;
}

double UtilSlowCont::GetAcnet(const std::string name, const int spill_id)
{
  auto iter = m_list_acnet[name].find(spill_id);
  if (iter != m_list_acnet[name].end()) return iter->second;
  
  DbSvc* db = GetDb();
  db->UseSchema("user_e1039_acnet");
  
  ostringstream oss;
  oss << "select table_name from variable_list where var_name = '" << name << "'";
  TSQLStatement* stmt = db->Process(oss.str());
  string table_name = stmt->NextResultRow() ? stmt->GetString(0) : "";
  delete stmt;
  if (table_name == "") return 0;

  oss.str("");
  oss << "select spill_id, value from " << table_name << " where spill_id between " << (spill_id-m_spill_id_window) << " and " << (spill_id+m_spill_id_window);
  stmt = db->Process(oss.str());
  while (stmt->NextResultRow()) {
    int     sp = stmt->GetInt   (0);
    double val = stmt->GetDouble(1);
    iter = m_list_acnet[name].find(sp);
    if (iter == m_list_acnet[name].end()) m_list_acnet[name][sp] = val;
  }
  delete stmt;
  
  return m_list_acnet[name][spill_id];
}

double UtilSlowCont::GetBeamDAQ(const std::string name, const int spill_id)
{
  auto iter = m_list_beamdaq[name].find(spill_id);
  if (iter != m_list_beamdaq[name].end()) return iter->second;
  
  DbSvc* db = GetDb();
  db->UseSchema("user_e1039_beamdaq");
  
  ostringstream oss;
  oss << "select table_name from variable_list where var_name = '" << name << "'";
  TSQLStatement* stmt = db->Process(oss.str());
  string table_name = stmt->NextResultRow() ? stmt->GetString(0) : "";
  delete stmt;
  if (table_name == "") return 0;

  oss.str("");
  oss << "select spill_id, value from " << table_name << " where spill_id between " << (spill_id-m_spill_id_window) << " and " << (spill_id+m_spill_id_window);
  stmt = db->Process(oss.str());
  while (stmt->NextResultRow()) {
    int     sp = stmt->GetInt   (0);
    double val = stmt->GetDouble(1);
    iter = m_list_beamdaq[name].find(sp);
    if (iter == m_list_beamdaq[name].end()) m_list_beamdaq[name][sp] = val;
  }
  delete stmt;
  
  return m_list_beamdaq[name][spill_id];
}

DbSvc* UtilSlowCont::GetDb()
{
  if (! m_db) m_db = new DbSvc(DbSvc::DB1);
  return m_db;
}
