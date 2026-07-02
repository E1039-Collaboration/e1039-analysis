#include <iomanip>
#include <TFile.h>
#include <TTree.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQMCEvent.h>
//#include <interface_main/SQTrackVector.h>
#include <interface_main/SQDimuonVector.h>
#include <ktracker/SRecEvent.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/getClass.h>
#include <UtilAna/UtilDimuon.h>
#include "AnaTrueDimuon.h"
using namespace std;

AnaTrueDimuon::AnaTrueDimuon() : SubsysReco("AnaTrueDimuon")
{
  ;
}

int AnaTrueDimuon::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaTrueDimuon::InitRun(PHCompositeNode* topNode)
{
  mi_evt         = findNode::getClass<SQEvent       >(topNode, "SQEvent");
  mi_evt_true    = findNode::getClass<SQMCEvent     >(topNode, "SQMCEvent");
  mi_vec_dim     = findNode::getClass<SQDimuonVector>(topNode, "SQTruthDimuonVector");
  if (!mi_evt || !mi_evt_true || !mi_vec_dim) return Fun4AllReturnCodes::ABORTEVENT;

  mi_dim_vec_rec = findNode::getClass<SQDimuonVector>(topNode, "SQRecDimuonVector_PM");
  if (!mi_dim_vec_rec) {
    cout << "The SQRecDimuonVector_PM nodes cannot be found in DST and thus won't be analyzed." << endl;
  }

  mo_file = new TFile("sim_tree.root", "RECREATE");
  mo_tree = new TTree("tree", "Created by AnaTrueDimuon");
  mo_tree->Branch("evt"     , &mo_evt);
  mo_tree->Branch("dim_true", &mo_dim_true);
  mo_tree->Branch("dim_reco", &mo_dim_reco);

  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaTrueDimuon::process_event(PHCompositeNode* topNode)
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
  mo_evt.rec_stat   = 0; // mi_srec->getRecStatus();
  if (mi_dim_vec_rec) mo_evt.n_dim_reco = mi_dim_vec_rec->size();
  else                mo_evt.n_dim_reco = 0;

  ///
  /// Dimuon info
  ///
  mo_dim_true.clear();
  mo_dim_reco.clear();
  for (unsigned int i_true = 0; i_true < mi_vec_dim->size(); i_true++) {
    SQDimuon* dim_true = mi_vec_dim->at(i_true);
    DimuonData dd;
    dd.pdg_id  = dim_true->get_pdg_id();
    dd.pos     = dim_true->get_pos();
    dd.mom     = dim_true->get_mom();
    dd.mom_pos = dim_true->get_mom_pos();
    dd.mom_neg = dim_true->get_mom_neg();
    UtilDimuon::CalcVar(dim_true, dd.mass, dd.pT, dd.x1, dd.x2, dd.xF, dd.costh, dd.phi);
    mo_dim_true.push_back(dd);

    if (mi_dim_vec_rec) {
      double mass_true = dim_true->get_mom().M();

      int i_reco_best = -1;
      double mass_diff_best;
      for (unsigned int i_reco = 0; i_reco < mi_dim_vec_rec->size(); i_reco++) {
        SRecDimuon* dim_reco = dynamic_cast<SRecDimuon*>(mi_dim_vec_rec->at(i_reco));
        double mass_diff = fabs(dim_reco->mass - mass_true);
        if (i_reco_best < 0 || mass_diff < mass_diff_best) {
          i_reco_best    = i_reco;
          mass_diff_best = mass_diff;
        }
      }
      
      RecoDimuonData ddr;
      if (i_reco_best >= 0) {
        SRecDimuon* dim_reco = dynamic_cast<SRecDimuon*>(mi_dim_vec_rec->at(i_reco_best));
        ddr.pos            = dim_reco->vtx;
        ddr.mom_pos        = dim_reco->p_pos;
        ddr.mom_neg        = dim_reco->p_neg;
        ddr.mom_pos_target = dim_reco->p_pos_target;
        ddr.mom_neg_target = dim_reco->p_neg_target;
        ddr.mom_pos_dump   = dim_reco->p_pos_dump;
        ddr.mom_neg_dump   = dim_reco->p_neg_dump;
      }
      mo_dim_reco.push_back(ddr); // Added even if no matched track was found.
    }
  }

  mo_tree->Fill();
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaTrueDimuon::End(PHCompositeNode* topNode)
{
  mo_file->cd();
  mo_file->Write();
  mo_file->Close();
  return Fun4AllReturnCodes::EVENT_OK;
}
