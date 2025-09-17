
#ifndef _SQVERTEXING_V2_H
#define _SQVERTEXING_V2_H

#include <fun4all/SubsysReco.h>

#include <ktracker/GFField.h>
#include <ktracker/GFTrack.h>


class TString;
class TVector3;
class SQTrack;
class SQTrackVector;
class SQDimuonVector;
class SRecEvent;
class SRecTrack;
class SRecDimuon;

class SQVertexing_v2: public SubsysReco
{
	public:
		SQVertexing_v2(const std::string& name = "SQVertexing_v2", int sign1 = 1, int sign2 = -1);
		~SQVertexing_v2();

		int Init(PHCompositeNode* topNode);
		int InitRun(PHCompositeNode* topNode);
		int process_event(PHCompositeNode* topNode);
		int End(PHCompositeNode* topNode);
		void InitVar();
		void set_legacy_rec_container(const bool enable = true)  { legacyContainer_in  = enable; legacyContainer_out = enable; }
		void set_legacy_in_container(const bool enable = true)   { legacyContainer_in  = enable; }
		void set_legacy_out_container(const bool enable = true)  { legacyContainer_out = enable; }
		void set_single_retracking(const bool enable = true)     { enableSingleRetracking = true; }

		void set_geom_file_name(const std::string& geomFileName) { geom_file_name = geomFileName; }

		int InitField(PHCompositeNode* topNode);
		int InitField();
		int InitGeom(PHCompositeNode* topNode);
		int InitGeom();
		int MakeNodes(PHCompositeNode* topNode);
		int GetNodes(PHCompositeNode*  topNode);

		double swimTrackToVertex(SQGenFit::GFTrack& track, double z, TVector3* pos = nullptr, TVector3* mom = nullptr);
		double refitTrkToVtx(SQGenFit::GFTrack& track, double z, TVector3* pos = nullptr, TVector3* mom = nullptr);
		double refitTrkToVtx(SRecTrack*         track, double z, TVector3* pos = nullptr, TVector3* mom = nullptr);
		double findDimuonZVertex(SRecDimuon& dimuon, SQGenFit::GFTrack& track1, SQGenFit::GFTrack& track2);
		double calcZsclp(double p);
		bool   processOneDimuon(SRecTrack* track1, SRecTrack* track2, SRecDimuon& dimuon);
  bool   processOneMuon(SRecTrack* track);
  
		bool legacyContainer_in, legacyContainer_out;
		bool enableSingleRetracking;

		int charge1, charge2;

		std::string geom_file_name;
		SQGenFit::GFField* gfield;

		SRecEvent*      recEvent;
		SQTrackVector*  recTrackVec;

		SQDimuonVector* recDimuonVec;
};

#endif
