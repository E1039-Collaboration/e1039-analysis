#ifndef _ANA_TRUE_DIMUON__H_
#define _ANA_TRUE_DIMUON__H_
#include <map>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <fun4all/SubsysReco.h>
#include "TreeData.h"
class TFile;
class TTree;
class SQEvent;
class SRecEvent;
class SQMCEvent;
class SQTrackVector;
class SQDimuonVector;

/// An example class to analyze true dimuons, together with reco. dimuons if available.
class AnaTrueDimuon: public SubsysReco {
  /// Input
  SQEvent*        mi_evt;
  SQMCEvent*      mi_evt_true;
  SQDimuonVector* mi_vec_dim;
  SQDimuonVector* mi_dim_vec_rec;
  
  /// Output
  TFile*         mo_file;
  TTree*         mo_tree;
  EventData      mo_evt;
  DimuonList     mo_dim_true;
  RecoDimuonList mo_dim_reco;

 public:
  AnaTrueDimuon();
  virtual ~AnaTrueDimuon() {;}
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);
};

#endif // _ANA_TRUE_DIMUON__H_
