#ifndef _ANA_RECO_DIMUON__H_
#define _ANA_RECO_DIMUON__H_
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
  RoadList    m_road_list;
  RecoDimuonList m_dim_list;
  //TrackList   m_trk_pos_list;
  //TrackList   m_trk_neg_list;

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
  void CheckRoadList(const std::vector<int>& list_road, const int road, const char* name);
  void FindFiredRoads(const SQHitVector* vec, std::vector<int>& list_road_top, std::vector<int>& list_road_bot, const bool in_time=true);
};

#endif // _ANA_RECO_DIMUON__H_
