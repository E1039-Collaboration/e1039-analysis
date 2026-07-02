#ifndef _ANA_TRUE_TRACK__H_
#define _ANA_TRUE_TRACK__H_
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

/// An example class to analyze true tracks, together with reco. tracks if available.
class AnaTrueTrack: public SubsysReco {
  /// Input
  SQEvent*        mi_evt;
  SQMCEvent*      mi_evt_true;
  SQTrackVector*  mi_vec_trk;
  SQTrackVector*  mi_trk_vec_rec;
  
  /// Output
  TFile*         mo_file;
  TTree*         mo_tree;
  EventData      mo_evt;
  TrackList      mo_trk_true;
  RecoTrackList  mo_trk_reco;

 public:
  AnaTrueTrack();
  virtual ~AnaTrueTrack() {;}
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);
};

#endif // _ANA_TRUE_TRACK__H_
