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

  std::string m_file_name;
  TFile*      m_file;
  TTree*      m_tree;
  EventData   m_evt;
  DimuonList  m_dim_list;

  UtilTrigger::TrigRoadset m_rs;
  
 public:
  AnaDimuon(const std::string& name="AnaDimuon");
  virtual ~AnaDimuon();
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);
  void SetOutputFileName(const std::string name) { m_file_name = name; }

  static void AnalyzeTree(TChain* tree);
};

#endif // _ANA_DIMUON__H_
