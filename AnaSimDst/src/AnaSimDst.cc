#include <iomanip>
#include <TFile.h>
#include <TTree.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQMCEvent.h>
#include <interface_main/SQTrackVector.h>
#include <interface_main/SQDimuonVector.h>
#include <ktracker/SRecEvent.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/getClass.h>
#include <UtilAna/UtilDimuon.h>
#include "AnaSimDst.h"
using namespace std;

AnaSimDst::AnaSimDst()
  : SubsysReco("AnaSimDst")
  , m_rs_id(0)
{
  ;
}

int AnaSimDst::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaSimDst::InitRun(PHCompositeNode* topNode)
{
  mi_evt      = findNode::getClass<SQEvent       >(topNode, "SQEvent");
  mi_evt_true = findNode::getClass<SQMCEvent     >(topNode, "SQMCEvent");
  mi_vec_trk  = findNode::getClass<SQTrackVector >(topNode, "SQTruthTrackVector");
  mi_vec_dim  = findNode::getClass<SQDimuonVector>(topNode, "SQTruthDimuonVector");
  //mi_srec     = findNode::getClass<SRecEvent     >(topNode, "SRecEvent");
  mi_vec_trk_rec = findNode::getClass<SQTrackVector >(topNode, "SQRecTrackVector");
  mi_vec_dim_rec = findNode::getClass<SQDimuonVector>(topNode, "SQRecDimuonVector_PM");
  if (!mi_evt || !mi_evt_true || !mi_vec_trk || !mi_vec_dim) return Fun4AllReturnCodes::ABORTEVENT;
  //if (!mi_srec) {
  if (!mi_vec_trk_rec) cout << "Cannot find the rec. track nodes." << endl;
  if (!mi_vec_dim_rec) cout << "Cannot find the rec. dimuon nodes." << endl;

  mo_file = new TFile("sim_tree.root", "RECREATE");
  mo_tree = new TTree("tree", "Created by AnaSimDst");
  mo_tree->Branch("evt"     , &mo_evt);
  mo_tree->Branch("trk_true", &mo_trk_true);
  mo_tree->Branch("dim_true", &mo_dim_true);
  mo_tree->Branch("trk_reco", &mo_trk_reco);
  mo_tree->Branch("dim_reco", &mo_dim_reco);

  if (m_rs_id != 0) {
    int ret = m_rs.LoadConfig(m_rs_id);
    if (ret != 0) {
      cout << "!!WARNING!!  AnaSimDst::InitRun():  roadset.LoadConfig returned " << ret << ".\n";
    }
    cout <<"Roadset " << m_rs.str(1) << endl;
  }
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaSimDst::process_event(PHCompositeNode* topNode)
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
  mo_evt.n_dim_true = mi_vec_dim->size();
  mo_evt.n_dim_reco = mi_vec_dim_rec ? mi_vec_dim_rec->size() : 0;
  mo_evt.rec_stat   = 0; // mi_srec->getRecStatus();
  //if (mi_srec) {
  //if (mi_vec_dim_rec) {
  //  mo_evt.n_dim_reco = mi_srec->getNDimuons();
  //} else {
  //  mo_evt.n_dim_reco = 0;
  //}

  ///
  /// Track info
  ///
  IdMap_t id_trk_t2r;
  //if (mi_srec) FindTrackRelation(id_trk_t2r);
  if (mi_vec_trk_rec) FindTrackRelation(id_trk_t2r);
  mo_trk_true.clear();
  mo_trk_reco.clear();
  for (unsigned int ii = 0; ii < mi_vec_trk->size(); ii++) {
    SQTrack* trk = mi_vec_trk->at(ii);
    TrackData td;
    td.charge  = trk->get_charge();
    td.pos_vtx = trk->get_pos_vtx();
    td.mom_vtx = trk->get_mom_vtx();
    mo_trk_true.push_back(td);
  
    //if (mi_srec) {
    if (mi_vec_trk_rec) {
      RecoTrackData tdr;
      if (id_trk_t2r[ii] >= 0) {
        //SRecTrack* trk_reco = &mi_srec->getTrack(id_trk_t2r[ii]);
        SRecTrack* trk_reco = dynamic_cast<SRecTrack*>(mi_vec_trk_rec->at(id_trk_t2r[ii]));
        tdr.charge  = trk_reco->getCharge();
        tdr.pos_vtx = trk_reco->getVertex();
        tdr.mom_vtx = trk_reco->getMomentumVertex();
      }
      mo_trk_reco.push_back(tdr);
    }
  }

  ///
  /// Dimuon info
  ///
  IdMap_t id_dim_t2r;
  //if (mi_srec) FindDimuonRelation(id_dim_t2r);
  if (mi_vec_dim_rec) FindDimuonRelation(id_dim_t2r);
  mo_dim_true.clear();
  mo_dim_reco.clear();
  for (unsigned int ii = 0; ii < mi_vec_dim->size(); ii++) {
    SQDimuon* dim = mi_vec_dim->at(ii);
    DimuonData dd;
    dd.pdg_id  = dim->get_pdg_id();
    dd.pos     = dim->get_pos();
    dd.mom     = dim->get_mom();
    dd.mom_pos = dim->get_mom_pos();
    dd.mom_neg = dim->get_mom_neg();
    UtilDimuon::CalcVar(dim, dd.mass, dd.pT, dd.x1, dd.x2, dd.xF, dd.costh, dd.phi);
    mo_dim_true.push_back(dd);

    //if (mi_srec) {
    if (mi_vec_dim_rec) {
      RecoDimuonData ddr;
      if (id_dim_t2r[ii] >= 0) {
        //SRecDimuon dim_reco = mi_srec->getDimuon(id_dim_t2r[ii]);
        SRecDimuon* dim_reco = dynamic_cast<SRecDimuon*>(mi_vec_dim_rec->at(id_dim_t2r[ii]));
        int trk_id_pos = dim_reco->get_track_id_pos();
        int trk_id_neg = dim_reco->get_track_id_neg();
        SRecTrack* trk_pos = dynamic_cast<SRecTrack*>(mi_vec_trk_rec->at(trk_id_pos));
        SRecTrack* trk_neg = dynamic_cast<SRecTrack*>(mi_vec_trk_rec->at(trk_id_neg));
        ddr.road_pos = trk_pos->getTriggerRoad();
        ddr.road_neg = trk_neg->getTriggerRoad();
        ddr.pos_top = m_rs.PosTop()->FindRoad(ddr.road_pos);
        ddr.pos_bot = m_rs.PosBot()->FindRoad(ddr.road_pos);
        ddr.neg_top = m_rs.NegTop()->FindRoad(ddr.road_neg);
        ddr.neg_bot = m_rs.NegBot()->FindRoad(ddr.road_neg);
        
        ddr.pos     = dim_reco->vtx;
        ddr.mom     = dim_reco->p_pos + dim_reco->p_neg;
        //ddr.mom_pos = dim_reco->p_pos;
        //ddr.mom_neg = dim_reco->p_neg;
        //ddr.x1      = dim_reco->x1;
        //ddr.x2      = dim_reco->x2;
        ddr.mom_target = dim_reco->p_pos_target + dim_reco->p_neg_target;
        ddr.mom_dump   = dim_reco->p_pos_dump   + dim_reco->p_neg_dump;
        
        ddr.trk_pos.charge         = trk_pos->get_charge();
        ddr.trk_pos.n_hits         = trk_pos->get_num_hits();
        ddr.trk_pos.chisq          = trk_pos->get_chisq();
        ddr.trk_pos.chisq_target   = trk_pos->get_chisq_target();
        ddr.trk_pos.chisq_dump     = trk_pos->get_chisq_dump();
        ddr.trk_pos.chisq_upstream = trk_pos->get_chsiq_upstream();
        ddr.trk_pos.pos_vtx        = trk_pos->get_pos_vtx();
        ddr.trk_pos.mom_vtx        = trk_pos->get_mom_vtx();
        ddr.trk_pos.pos_st1        = trk_pos->get_pos_st1();
        ddr.trk_pos.mom_st1        = trk_pos->get_mom_st1();
        ddr.trk_pos.pos_st3        = trk_pos->get_pos_st3();
        ddr.trk_pos.mom_st3        = trk_pos->get_mom_st3();
        ddr.trk_pos.pos_target     = trk_pos->get_pos_target();
        ddr.trk_pos.pos_dump       = trk_pos->get_pos_dump();

        ddr.trk_neg.charge         = trk_neg->get_charge();
        ddr.trk_neg.n_hits         = trk_neg->get_num_hits();
        ddr.trk_neg.chisq          = trk_neg->get_chisq();
        ddr.trk_neg.chisq_target   = trk_neg->get_chisq_target();
        ddr.trk_neg.chisq_dump     = trk_neg->get_chisq_dump();
        ddr.trk_neg.chisq_upstream = trk_neg->get_chsiq_upstream();
        ddr.trk_neg.pos_vtx        = trk_neg->get_pos_vtx();
        ddr.trk_neg.mom_vtx        = trk_neg->get_mom_vtx();
        ddr.trk_neg.pos_st1        = trk_neg->get_pos_st1();
        ddr.trk_neg.mom_st1        = trk_neg->get_mom_st1();
        ddr.trk_neg.pos_st3        = trk_neg->get_pos_st3();
        ddr.trk_neg.mom_st3        = trk_neg->get_mom_st3();
        ddr.trk_neg.pos_target     = trk_neg->get_pos_target();
        ddr.trk_neg.pos_dump       = trk_neg->get_pos_dump();
      }
      mo_dim_reco.push_back(ddr);
    }
  }

  mo_tree->Fill();
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaSimDst::End(PHCompositeNode* topNode)
{
  mo_file->cd();
  mo_file->Write();
  mo_file->Close();
  return Fun4AllReturnCodes::EVENT_OK;
}

void AnaSimDst::FindTrackRelation(IdMap_t& id_map)
{
  id_map.clear();
  for (unsigned int i_true = 0; i_true < mi_vec_trk->size(); i_true++) {
    SQTrack* trk_true = mi_vec_trk->at(i_true);
    int     ch_true = trk_true->get_charge();
    double mom_true = trk_true->get_mom_vtx().Mag();

    int i_reco_best = -1;
    double mom_diff_best;
    //for (int i_reco = 0; i_reco < mi_srec->getNTracks(); i_reco++) {
    for (int i_reco = 0; i_reco < mi_vec_trk_rec->size(); i_reco++) {
      //SRecTrack* trk_reco = &mi_srec->getTrack(i_reco);
      SRecTrack* trk_reco = dynamic_cast<SRecTrack*>(mi_vec_trk_rec->at(i_reco));
      if (trk_reco->getCharge() != ch_true) continue;
      double mom_diff = fabs(trk_reco->getMomentumVertex().Mag() - mom_true);
      if (i_reco_best < 0 || mom_diff < mom_diff_best) {
        i_reco_best   = i_reco;
        mom_diff_best = mom_diff;
      }
    }
    id_map[i_true] = i_reco_best;
  }
}

void AnaSimDst::FindDimuonRelation(IdMap_t& id_map)
{
  id_map.clear();
  for (unsigned int i_true = 0; i_true < mi_vec_dim->size(); i_true++) {
    SQDimuon* dim_true = mi_vec_dim->at(i_true);
    double mass_true = dim_true->get_mom().M();

    int i_reco_best = -1;
    double mass_diff_best;
    //for (int i_reco = 0; i_reco < mi_srec->getNDimuons(); i_reco++) {
    for (int i_reco = 0; i_reco < mi_vec_dim_rec->size(); i_reco++) {
      //SRecDimuon dim_reco = mi_srec->getDimuon(i_reco);
      SRecDimuon* dim_reco = dynamic_cast<SRecDimuon*>(mi_vec_dim_rec->at(i_reco));
      double mass_diff = fabs(dim_reco->mass - mass_true);
      if (i_reco_best < 0 || mass_diff < mass_diff_best) {
        i_reco_best   = i_reco;
        mass_diff_best = mass_diff;
      }
    }
    id_map[i_true] = i_reco_best;
  }
}
