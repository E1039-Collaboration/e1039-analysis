#include <fstream>
#include <iomanip>
#include <TFile.h>
#include <TTree.h>
#include <phool/getClass.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include "GenEmbeddingDataNim4.h"
using namespace std;

GenEmbeddingDataNim4::GenEmbeddingDataNim4(const std::string list_name, const std::string name)
  : SubsysReco(name)
  , m_list_name(list_name)
  , m_name_file("embedding_data.root")
  , m_name_tree("tree")
{
  ;
}

int GenEmbeddingDataNim4::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int GenEmbeddingDataNim4::InitRun(PHCompositeNode* topNode)
{
  SQRun* sq_run = findNode::getClass<SQRun>(topNode, "SQRun");
  if (!sq_run) return Fun4AllReturnCodes::ABORTEVENT;
  ReadSpillList(sq_run->get_run_id());
  
  mi_evt     = findNode::getClass<SQEvent       >(topNode, "SQEvent");
  mi_vec_hit = findNode::getClass<SQHitVector   >(topNode, "SQHitVector");
  if (!mi_evt || !mi_vec_hit) {
    cout << PHWHERE << ":  Cannot find SQEvent and/or SQHitVector." << endl;
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  mo_file = new TFile(m_name_file.c_str(), "RECREATE");
  mo_tree = new TTree(m_name_tree.c_str(), "Created by GenEmbeddingDataNim4");
  mo_tree->Branch("SQEvent"    , &mi_evt);
  mo_tree->Branch("SQHitVector", &mi_vec_hit);
  mo_tree->Branch("qa_data"    , &mo_qa_data);
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int GenEmbeddingDataNim4::process_event(PHCompositeNode* topNode)
{
  int spill_id = mi_evt->get_spill_id();
  if (std::find(m_list_spill_id.begin(), m_list_spill_id.end(), spill_id) == m_list_spill_id.end()) {
    return Fun4AllReturnCodes::EVENT_OK;
  }

  mo_qa_data.turn_id = mi_evt->get_qie_turn_id();
  mo_qa_data.rf_id   = mi_evt->get_qie_rf_id();
  mo_qa_data.rfp01   = mi_evt->get_qie_rf_intensity( 1);
  mo_qa_data.rfp00   = mi_evt->get_qie_rf_intensity( 0);
  mo_qa_data.rfm01   = mi_evt->get_qie_rf_intensity(-1);
  mo_qa_data.D1 = mo_qa_data.D2 = mo_qa_data.D3p = mo_qa_data.D3m = 0;
  for (SQHitVector::Iter it = mi_vec_hit->begin(); it != mi_vec_hit->end(); it++) {
    SQHit* hit = *it;
    int det_id = hit->get_detector_id();
    if      ( 0 < det_id && det_id <=  6) mo_qa_data.D1++;
    else if (12 < det_id && det_id <= 18) mo_qa_data.D2++;
    else if (18 < det_id && det_id <= 24) mo_qa_data.D3p++;
    else if (24 < det_id && det_id <= 30) mo_qa_data.D3m++;
  }
  mo_tree->Fill();
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int GenEmbeddingDataNim4::End(PHCompositeNode* topNode)
{
  mo_file->cd();
  mo_file->Write();
  mo_file->Close();
  return Fun4AllReturnCodes::EVENT_OK;
}

void GenEmbeddingDataNim4::ReadSpillList(const int run_id)
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
  cout << "GenEmbeddingDataNim4::ReadSpillList():  " << n_sp << " spills." << endl;
  if (m_list_spill_id.size() == 0) {
    cout << "!!ERROR!!  No good spill from the list.  Abort." << endl;
    exit(1);
  }
}
