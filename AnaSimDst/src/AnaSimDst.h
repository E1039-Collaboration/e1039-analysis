#ifndef _ANA_SIM_DST__H_
#define _ANA_SIM_DST__H_
#include <map>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <fun4all/SubsysReco.h>
#include <UtilAna/TrigRoadset.h>
#include "TreeData.h"
class TFile;
class TTree;
class SQEvent;
class SRecEvent;
class SQMCEvent;
class SQTrackVector;
class SQDimuonVector;

/// An example class to analyze the simulated uDST file.
class AnaSimDst: public SubsysReco {
  int m_rs_id;
  
  /// Input
  SQEvent       * mi_evt;
  //SRecEvent     * mi_srec;
  SQMCEvent     * mi_evt_true;
  SQTrackVector * mi_vec_trk;
  SQDimuonVector* mi_vec_dim;
  SQTrackVector * mi_vec_trk_rec;
  SQDimuonVector* mi_vec_dim_rec;

  /// Output
  TFile*     mo_file;
  TTree*     mo_tree;
  EventData  mo_evt;
  TrackList  mo_trk_true;
  DimuonList mo_dim_true;
  RecoTrackList  mo_trk_reco;
  RecoDimuonList mo_dim_reco;

  UtilTrigger::TrigRoadset m_rs;
  
 public:
  AnaSimDst();
  virtual ~AnaSimDst() {;}
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);
  
  void SetRoadsetID(const int id)  { m_rs_id = id; }
  int  GetRoadsetID() const { return m_rs_id; }
  
 private:
  typedef std::map<int, int> IdMap_t; // For now the key is not ID but index.
  void FindTrackRelation (IdMap_t& id_map);
  void FindDimuonRelation(IdMap_t& id_map);
};

#endif /* _ANA_SIM_DST__H_ */
