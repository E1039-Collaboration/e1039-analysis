#ifndef _ANA_BEAM__H_
#define _ANA_BEAM__H_
#include <fun4all/SubsysReco.h>
#include "TreeData.h"
class TFile;
class TTree;
class SQEvent;
class SQHitVector;

class AnaBeam: public SubsysReco {
  std::string m_list_name;
  std::vector<int> m_list_spill_id;
  
  /// Input
  SQEvent       * mi_evt;
  SQHitVector   * mi_vec_hit;

  /// Output
  std::string m_name_file;
  std::string m_name_tree;
  TFile* mo_file;
  TTree* mo_tree;
  BeamData mo_beam;

 public:
  AnaBeam(const std::string name="AnaBeam");
  virtual ~AnaBeam() {;}
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

  void SetSpillListName(const std::string name) { m_list_name = name; }
  void SetFileName     (const std::string name) { m_name_file = name; }
  void SetTreeName     (const std::string name) { m_name_tree = name; }
  std::string GetSpillListName() const { return m_list_name; }
  std::string GetFileName     () const { return m_name_file; }
  std::string GetTreeName     () const { return m_name_tree; }

protected:
  void ReadSpillList(const int run_id);
};

#endif // _ANA_BEAM__H_
