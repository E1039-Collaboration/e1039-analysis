#ifndef _ANA_RECO_DIMUON__H_
#define _ANA_RECO_DIMUON__H_
#include <fstream>
#include <fun4all/SubsysReco.h>
#include <UtilAna/TrigRoadset.h>
#include "TreeData.h"
class TFile;
class TTree;
class TChain;
class SQEvent;
class SQHitVector;
class SQTrackVector;
class SQDimuonVector;

class AnaRecoDimuon: public SubsysReco {
  int m_rs_id;
  
  SQEvent*     m_sq_evt;
  SQHitVector* m_sq_hit_vec;
  SQTrackVector*  m_sq_trk_vec;
  SQDimuonVector* m_sq_dim_vec;

  std::string m_file_name;
  TFile*      m_file;
  TTree*      m_tree;
  EventData   m_evt;
  RoadData    m_road_emu; //< Fired roads emulated with hodo hits
  RoadList    m_road_list; //< Expected fired roads based on track trajectory
  RoadList    m_road_list_all; //< All roads, including not-enabled roads, matched to track trajectory
  RecoDimuonList m_dim_list;

  std::ofstream m_ofs;
  unsigned int n_evt_all;
  unsigned int n_evt_ok;
  
  UtilTrigger::TrigRoadset m_rs;
  
 public:
  AnaRecoDimuon(const std::string& name="AnaRecoDimuon");
  virtual ~AnaRecoDimuon();
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

  void SetRoadsetID(const int id)  { m_rs_id = id; }
  int  GetRoadsetID() const { return m_rs_id; }
  
  //void SetOutputFileName(const std::string name) { m_file_name = name; }
  std::string GetOutputFileName() const   { return m_file_name; }

  void AnalyzeTree(TTree* tree, const bool road_match=true);

protected:
  void CheckRoadList(const std::vector<int>& list_road, const int road, const char* name, std::ostream& os=std::cout);
};

#endif // _ANA_RECO_DIMUON__H_
