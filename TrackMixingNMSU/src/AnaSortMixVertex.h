#ifndef _ANA_SortMixVertex__H_
#define _ANA_SortMixVertex__H_
#include <fstream>
#include <ktracker/SRawEvent.h>
#include <ktracker/SRecEvent.h>
#include <ktracker/UtilSRawEvent.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include <ktracker/GFField.h>
#include <ktracker/GFTrack.h>

class TFile;
class TTree;
class SRecTrack;
class SQEvent;
class SQHitVector;

/*struct SRecDimuonData {
  int            trk_id_pos;
  int		 trk_id_neg;
  std::string    origin;
  TVector3       pos;
  TLorentzVector mom;
  TLorentzVector mom_pos;
  TLorentzVector mom_neg;
  double         mass;
  double         pT;
  double         x1;
  double         x2;
  double         xF;
  double         costh;
  double         phi;
  
  SRecDimuonData();
  virtual ~SRecDimuonData() {;}

  ClassDef(SRecDimuonData, 1);

};*/

/// Base class of all analysis modules.
class AnaSortMixVertex
{
	/// Parameter
	std::string m_name;
	std::string m_dir_out;
	//std::string geom_file_name;
	std::string m_ds; ///< Dataset
	int  m_run_id; ///< Run ID
	bool m_use_raw; ///< true if using SRawEvent 
	bool m_use_rec; ///< true if using SRecEvent
	unsigned int m_n_evt_max; ///< N of max events to be analyzed per run
	int m_verb; ///< Verbosity
	/// Input
	SRawEvent* m_raw;
	SRecEvent* m_rec;
	SQEvent* sq_evt;
	SQHitVector* sq_htvec;
	/// Output
	std::ofstream m_ofs;
	TFile* m_file_out;

	//bool dumpfit;
	//int m_target_id;

	public:
	virtual ~AnaSortMixVertex();
	virtual void Init(const int run_id);
	virtual void End();
	virtual void Analyze(const int run_id, const std::vector<std::string> list_in);
	AnaSortMixVertex(const std::string name="ana_mixvertex");


	virtual void SortTree(TTree* tree_to_sort);
	virtual void MixTracks(TTree* sorted_tree);
	virtual void DoVertex(TTree* inputtree, const int run_id, bool mix);
	bool Target_Like(SRecTrack* trk_pos, SRecTrack* trk_neg);
	bool Dump_Like(SRecTrack* trk_pos, SRecTrack* trk_neg);
	bool UpStream_Like(SRecTrack* trk_pos, SRecTrack* trk_neg);
	//virtual void PrepareMC(char* file_mctree);
	virtual void EmbedMCSignal( char *file_mixed, char* file_mcsignal);
	void SetTargetPos( const int target_pos){m_target_pos = target_pos;}

	/// Get the name of analysis module.
	std::string Name() const { return m_name; }
	/// Set the output directory.
	void SetOutputDir(const std::string dir_out) { m_dir_out = dir_out; }
	/// Set the output directory using the given integer (usually run ID).
	void SetOutputDir(const int dir_out) { m_dir_out = std::to_string(dir_out); }
	/// Get the output directory.
	std::string GetOutputDir() const { return m_dir_out; }
	void SetDataset(const std::string ds) { m_ds = ds; } ///< Set the dataset to be analyzed.
	std::string GetDataset() const { return m_ds; }      ///< Get the dataset to be analyzed.
	void SetRunID(const int run_id) { m_run_id = run_id; } ///< Set the run ID to be analyzed.
	int  GetRunID() const { return m_run_id; }             ///< Get the run ID to be analyzed.
	void UseRawEvent(const bool value) { m_use_raw = value; } ///< Turn on/off the use of SRawEvent.
	void UseRecEvent(const bool value) { m_use_rec = value; } ///< Turn on/off the use of SRecEvent.
	/// Set the max number of events to be analyzed per run.
	void SetNumEventsAnalyzed(const unsigned int n_evt) { m_n_evt_max = n_evt; }
	void Verbosity(const int verb) { m_verb = verb; } ///< Set the verbosity.
	int  Verbosity() const  { return m_verb; }        ///< Get the verbosity.
	/// Get the ofstream object for output.  Usable in `Init()`, `AnalyzeEvent()` and `End()` of each analysis module.
	std::ofstream& OFS() { return m_ofs; }
	/// Get the TFile object for output.  Usable in `Init()`, `AnalyzeEvent()` and `End()` of each analysis module.
	TFile* OFile() { return m_file_out; }


	protected:
	SRawEvent* GetRawEvent() { return m_raw; } ///< Get a pointer to `SRawEvent`.  Usable in `AnalyzeEvent()`.
	SRecEvent* GetRecEvent() { return m_rec; } ///< Get a pointer to `SRecEvent`.  Usable in `AnalyzeEvent()`.

	TTree* ana_tree;
	TTree* sorted_tree;
	TTree* mixed_tree;
	TTree* save_sorted;
	TTree* save_mix;


	int   runID, m_target_pos;
	int spill_ID, plus_spill_ID, minus_spill_ID;
	int event_ID, plus_event_ID, minus_event_ID;

	//int rfp00; ///< RF+00
	float rfp00c; ///< RF+00 corrected for the pedestal
	float pot_p00; ///proton number corrsponding to RF+00 (pedestal corrected)
	float liveP; //Live PoT

	int DimMult;
	int occuD0, occuD1, occuD2, occuD3p, occuD3m;
	int TrkMult, PosTrkMult, NegTrkMult;
	int TargetPos;
	int fpga1, fpga2, fpga3, fpga4, fpga5;

	std::vector<SRecTrack> pos_tracks, neg_tracks;
	std::vector<SRecTrack> pos_tracks_mix, neg_tracks_mix;
	SRecEvent* recEvent;
	SRecEvent* recEvent_target_like;
	SRecEvent* recEvent_dump_like;
	SRecEvent* recEvent_upstream_like;
	SQGenFit::GFField* gfield_v2;

	int plus_fpga1, plus_occuD1, plus_occuD2, plus_occuD3p, plus_occuD3m, plus_TargetPos; 
	float plus_rfp00c, plus_pot_p00, plus_liveP;

	int minus_fpga1, minus_occuD1, minus_occuD2, minus_occuD3p, minus_occuD3m, minus_TargetPos;
	float minus_rfp00c, minus_pot_p00, minus_liveP;


};
#endif // _ANA_VERTEXFIT__H_
