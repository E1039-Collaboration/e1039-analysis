#include <fstream>
#include <iomanip>
#include <TFile.h>
#include <TTree.h>
#include <phool/getClass.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include "AnaBeam.h"
using namespace std;

AnaBeam::AnaBeam(const std::string name)
  : SubsysReco(name)
  , m_list_name("")
  , m_name_file("output.root")
  , m_name_tree("tree")
{
  ;
}

int AnaBeam::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaBeam::InitRun(PHCompositeNode* topNode)
{
  SQRun* sq_run = findNode::getClass<SQRun>(topNode, "SQRun");
  if (!sq_run) return Fun4AllReturnCodes::ABORTEVENT;
  mo_beam.run_id = sq_run->get_run_id();
  if (m_list_name.size() > 0) ReadSpillList(mo_beam.run_id);
  
  mi_evt     = findNode::getClass<SQEvent       >(topNode, "SQEvent");
  mi_vec_hit = findNode::getClass<SQHitVector   >(topNode, "SQHitVector");
  if (!mi_evt || !mi_vec_hit) {
    cout << PHWHERE << ":  Cannot find SQEvent and/or SQHitVector." << endl;
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  mo_file = new TFile(m_name_file.c_str(), "RECREATE");
  mo_tree = new TTree(m_name_tree.c_str(), "Created by AnaBeam");
  mo_tree->Branch("beam_data", &mo_beam);
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaBeam::process_event(PHCompositeNode* topNode)
{
  if (! mi_evt->get_trigger(SQEvent::MATRIX1) &&
      ! mi_evt->get_trigger(SQEvent::NIM4   )   ) return Fun4AllReturnCodes::EVENT_OK;
  
  mo_beam.spill_id = mi_evt->get_spill_id();
  if (std::find(m_list_spill_id.begin(), m_list_spill_id.end(), mo_beam.spill_id) == m_list_spill_id.end()) return Fun4AllReturnCodes::EVENT_OK;

  mo_beam.event_id  =  mi_evt->get_event_id();
  mo_beam.fpga_bits = (mi_evt->get_trigger() >> SQEvent::MATRIX1) & 0x1f;
  mo_beam.nim_bits  = (mi_evt->get_trigger() >> SQEvent::NIM1   ) & 0x1f;
  mo_beam.turn_id   =  mi_evt->get_qie_turn_id();
  mo_beam.rf_id     =  mi_evt->get_qie_rf_id();
  mo_beam.rfp01     =  mi_evt->get_qie_rf_intensity( 1);
  mo_beam.rfp00     =  mi_evt->get_qie_rf_intensity( 0);
  mo_beam.rfm01     =  mi_evt->get_qie_rf_intensity(-1);
  mo_beam.d1 = mo_beam.d2 = mo_beam.d3p = mo_beam.d3m = 0;
  for (SQHitVector::Iter it = mi_vec_hit->begin(); it != mi_vec_hit->end(); it++) {
    SQHit* hit = *it;
    int det_id = hit->get_detector_id();
    if      ( 0 < det_id && det_id <=  6) mo_beam.d1++;
    else if (12 < det_id && det_id <= 18) mo_beam.d2++;
    else if (18 < det_id && det_id <= 24) mo_beam.d3p++;
    else if (24 < det_id && det_id <= 30) mo_beam.d3m++;
  }
  mo_tree->Fill();
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaBeam::End(PHCompositeNode* topNode)
{
  mo_file->cd();
  mo_file->Write();
  mo_file->Close();
  return Fun4AllReturnCodes::EVENT_OK;
}

void AnaBeam::ReadSpillList(const int run_id)
{
  m_list_spill_id.clear();
  ifstream ifs(m_list_name);
  int run;
  int spill;
  while (ifs >> run >> spill) {
    if (run == run_id) m_list_spill_id.push_back(spill);
  }
  ifs.close();
  int n_sp = m_list_spill_id.size();
  cout << "AnaBeam::ReadSpillList():  " << n_sp << " spills." << endl;
  if (m_list_spill_id.size() == 0) {
    cout << "!!ERROR!!  No good spill from the list.  Abort." << endl;
    exit(1);
  }
}
