#ifndef _GEN_EMBEDDING_DATA_NIM4__H_
#define _GEN_EMBEDDING_DATA_NIM4__H_
#include <fun4all/SubsysReco.h>
#include "TreeData.h"
class TFile;
class TTree;
class SQEvent;
class SQHitVector;

/// An example class to analyze the simulated uDST file.
class GenEmbeddingDataNim4: public SubsysReco {
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
  EmbQAData mo_qa_data;

 public:
  GenEmbeddingDataNim4(const std::string list_name, const std::string name="GenEmbeddingDataNim4");
  virtual ~GenEmbeddingDataNim4() {;}
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

  void SetFileName  (const std::string name) { m_name_file = name; }
  void SetTreeName  (const std::string name) { m_name_tree = name; }
  std::string GetFileName  () const { return m_name_file; }
  std::string GetTreeName  () const { return m_name_tree; }

protected:
  void ReadSpillList(const int run_id);
};

#endif // _GEN_EMBEDDING_DATA_NIM4__H_
