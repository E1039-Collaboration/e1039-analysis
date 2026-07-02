#include <iomanip>
#include <TFile.h>
#include <TTree.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQMCEvent.h>
#include <interface_main/SQTrackVector.h>
#include <ktracker/SRecEvent.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/getClass.h>
#include "AnaTrueTrack.h"
using namespace std;

AnaTrueTrack::AnaTrueTrack() : SubsysReco("AnaTrueTrack")
{
  ;
}

int AnaTrueTrack::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaTrueTrack::InitRun(PHCompositeNode* topNode)
{
  mi_evt         = findNode::getClass<SQEvent       >(topNode, "SQEvent");
  mi_evt_true    = findNode::getClass<SQMCEvent     >(topNode, "SQMCEvent");
  mi_vec_trk     = findNode::getClass<SQTrackVector >(topNode, "SQTruthTrackVector");
  if (!mi_evt || !mi_evt_true || !mi_vec_trk) return Fun4AllReturnCodes::ABORTEVENT;

  mi_trk_vec_rec = findNode::getClass<SQTrackVector >(topNode, "SQRecTrackVector");
  if (!mi_trk_vec_rec) {
    cout << "The SQRecTrackVector node cannot be found in DST and thus won't be analyzed." << endl;
  }

  mo_file = new TFile("sim_tree.root", "RECREATE");
  mo_tree = new TTree("tree", "Created by AnaTrueTrack");
  mo_tree->Branch("evt"     , &mo_evt);
  mo_tree->Branch("trk_true", &mo_trk_true);
  mo_tree->Branch("trk_reco", &mo_trk_reco);

  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaTrueTrack::process_event(PHCompositeNode* topNode)
{
  static unsigned int n_evt = 0;
  if    (++n_evt % 100000 == 0) cout << n_evt << endl;
  else if (n_evt %  10000 == 0) cout << " . " << flush;

  ///
  /// Event info
  ///
  mo_evt.proc_id = mi_evt_true->get_process_id();
  for (int ii = 0; ii < 4; ii++) {
    mo_evt.par_id [ii] = mi_evt_true->get_particle_id      (ii);
    mo_evt.par_mom[ii] = mi_evt_true->get_particle_momentum(ii);
  }
  mo_evt.weight     = mi_evt_true->get_weight();
  mo_evt.trig_bits  = mi_evt->get_trigger();
  mo_evt.rec_stat   = 0; // mi_srec->getRecStatus();

  ///
  /// Track info
  ///
  mo_trk_true.clear();
  mo_trk_reco.clear();
  for (unsigned int i_true = 0; i_true < mi_vec_trk->size(); i_true++) {
    SQTrack* trk_true = mi_vec_trk->at(i_true);
    TrackData td;
    td.charge  = trk_true->get_charge();
    td.pos_vtx = trk_true->get_pos_vtx();
    td.mom_vtx = trk_true->get_mom_vtx();
    mo_trk_true.push_back(td);
  
    if (mi_trk_vec_rec) {
      int     ch_true = trk_true->get_charge();
      double mom_true = trk_true->get_mom_vtx().Mag();
      
      int i_reco_best = -1;
      double mom_diff_best;
      for (unsigned int i_reco = 0; i_reco < mi_trk_vec_rec->size(); i_reco++) {
        SRecTrack* trk_reco = dynamic_cast<SRecTrack*>(mi_trk_vec_rec->at(i_reco));
        if (trk_reco->getCharge() != ch_true) continue;
        double mom_diff = fabs(trk_reco->getMomentumVertex().Mag() - mom_true);
        if (i_reco_best < 0 || mom_diff < mom_diff_best) {
          i_reco_best   = i_reco;
          mom_diff_best = mom_diff;
        }
      }
      
      RecoTrackData tdr;
      if (i_reco_best >= 0) {
        SRecTrack* trk_reco = dynamic_cast<SRecTrack*>(mi_trk_vec_rec->at(i_reco_best));
        tdr.charge  = trk_reco->getCharge();
        tdr.road    = trk_reco->getTriggerRoad();
        tdr.n_hits  = trk_reco->getNHits();
        tdr.chisq   = trk_reco->getChisq();
        tdr.pos_vtx = trk_reco->getVertex();
        tdr.mom_vtx = trk_reco->getMomentumVertex();
        // Set more variables as you need
      }
      mo_trk_reco.push_back(tdr); // Added even if no matched track was found.
    }
  }

  mo_tree->Fill();
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaTrueTrack::End(PHCompositeNode* topNode)
{
  mo_file->cd();
  mo_file->Write();
  mo_file->Close();
  return Fun4AllReturnCodes::EVENT_OK;
}
