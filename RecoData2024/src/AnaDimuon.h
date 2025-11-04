#ifndef _ANA_DIMUON__H_
#define _ANA_DIMUON__H_
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

/// An example class to analyze dimuons.
class AnaDimuon: public SubsysReco {
  SQEvent*     m_sq_evt;
  SQHitVector* m_sq_hit_vec;
  SQTrackVector*  m_sq_trk_vec;
  SQDimuonVector* m_sq_dim_vec;

  std::string m_node_prefix;
  std::string m_mode;
  bool m_output_tree;
  std::string m_file_name;
  TFile*      m_file;
  TTree*      m_tree;
  EventData   m_evt;
  DimuonList  m_dim_list;
  TrackList   m_trk_pos_list;
  TrackList   m_trk_neg_list;
  RoadList    m_road_list_0;
  RoadList    m_road_list_1;
  RoadList    m_road_list_2;

  UtilTrigger::TrigRoadset m_rs;
  
 public:
  AnaDimuon(const std::string& name="AnaDimuonPM", const std::string& mode="PM");
  virtual ~AnaDimuon();
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

  //void SetOutputFileName(const std::string name) { m_file_name = name; }
  std::string GetOutputFileName() const   { return m_file_name; }

  void SetNodePrefix(const std::string name) { m_node_prefix = name; }
  std::string GetNodePrefix() const   { return m_node_prefix; }
  
  void EnableTreeOutput() { m_output_tree = true; }
  void DisableTreeOutput() { m_output_tree = false; }

  void AnalyzeTree(TChain* tree, const bool road_match=true);

protected:
  void CheckRoadList(const std::vector<int>& list_road, const int road, const char* name);
  std::vector<const UtilTrigger::TrigRoad*> FindRoads(const UtilTrigger::TrigRoads* list_road_all, const std::vector<int> list_road_id) const;
  std::vector<int> FindRoadIDs(const UtilTrigger::TrigRoads* list_road_all, const std::vector<int> list_road_id) const;
};

#endif // _ANA_DIMUON__H_
