#include <fstream>
#include <ctime>
#include <iomanip>
#include <TCanvas.h>
#include <TH1D.h>
#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
#include <phool/recoConsts.h>
#include <geom_svc/GeomSvc.h>
#include <ktracker/SRawEvent.h>
#include <ktracker/SRecEvent.h>
#include <ktracker/SQVertexing.h>
#include <interface_main/SQTrackVector.h>
#include <GenFit/FieldManager.h>
#include <phfield/PHFieldConfig.h>
#include <phfield/PHFieldConfig_v3.h>
#include <phfield/PHFieldUtility.h>
#include "AnaSortMixVertex.h"
#include <UtilAna/UtilDimuon.h>
#include "UtilBeam.h"
#include "TreeData.h"

using namespace std;

AnaSortMixVertex::AnaSortMixVertex(const std::string name)
	: m_name(name)
	, m_dir_out(".")
	, m_ds("")
	, m_run_id(0)
	, m_use_raw(true)
	, m_use_rec(true)
	, m_n_evt_max(0)
	, m_verb(0)
	, m_raw(0)
	, m_rec(0)
	, ana_tree(0)
	, mixed_tree(0)
	, m_target_pos(-1)
{
  ;
}
AnaSortMixVertex::~AnaSortMixVertex()
{
  if (m_raw) delete m_raw;
  if (m_rec) delete m_rec;
}

/// Function to initialize all variables for the 1st analysis step.
/**
 * Init() of each analysis module should call this function and then initialize its own variables.
 */
void AnaSortMixVertex::Init(const int run_id)
{
  //initializing the reco constants used during original reconstruction	
  recoConsts* rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", run_id);
  rc->set_DoubleFlag("FMAGSTR", -1.044);
  rc->set_DoubleFlag("KMAGSTR", -1.025);
  //rc->set_DoubleFlag("X_BEAM", -0.2); 
  rc->set_BoolFlag("COARSE_MODE", false);
  rc->set_BoolFlag("REQUIRE_MUID", false);
  rc->set_CharFlag("HIT_MASK_MODE", "X");
  
  rc->set_CharFlag("AlignmentMille", "$E1039_RESOURCE/alignment/run0/align_mille_v10_a.txt");
  rc->set_CharFlag("AlignmentHodo", "");
  rc->set_CharFlag("AlignmentProp", "");
  rc->set_CharFlag("Calibration", "");
  rc->set_IntFlag ("MaxHitsDC0" , 350);
  rc->set_IntFlag ("MaxHitsDC1" , 350);
  rc->set_IntFlag ("MaxHitsDC2" , 170);
  rc->set_IntFlag ("MaxHitsDC3p", 140);
  rc->set_IntFlag ("MaxHitsDC3m", 140);
  rc->set_DoubleFlag("RejectWinDC0" , 0.3);
  rc->set_DoubleFlag("RejectWinDC1" , 0.5);
  rc->set_DoubleFlag("RejectWinDC2" , 0.35);
  rc->set_DoubleFlag("RejectWinDC3p", 0.24);
  rc->set_DoubleFlag("RejectWinDC3m", 0.24);
  
  //setting up the output directory
  if (m_dir_out != ".") gSystem->mkdir(m_dir_out.c_str(), true);
  m_ofs.open((m_dir_out + "/output.txt").c_str());
  m_file_out = new TFile((m_dir_out + "/output.root").c_str(), "RECREATE");

  m_vtxfit = new SQVertexing();
  m_vtxfit->set_geom_file_name((string)gSystem->Getenv("E1039_RESOURCE") + "/geometry/geom_run005433.root");
  m_vtxfit->set_legacy_rec_container(true);
  m_vtxfit->Init();
  m_vtxfit->InitRun();
}

void AnaSortMixVertex::Analyze(const int run_id, const std::vector<std::string> list_in)
{
	//this is the main funtion to collect all the data from all good spill for given run and call the sort, mix and vertec function.
 
       //tree to store all the necessary information collected from the input reco files	
	m_file_out->cd();
	ana_tree = new TTree("ana_tree","tree to store analysis variables");
	ana_tree->Branch("pos_tracks", &pos_tracks);
	ana_tree->Branch("neg_tracks", &neg_tracks);
	ana_tree->Branch("fpga1", &fpga1, "fpga1/I");
	ana_tree->Branch("fpga2", &fpga2, "fpga2/I");
	ana_tree->Branch("fpga3", &fpga3, "fpga3/I");
	ana_tree->Branch("fpga4", &fpga4, "fpga4/I");
	//ana_tree->Branch("occuD0", &occuD0, "occuD0/I");
	ana_tree->Branch("occuD1", &occuD1, "occuD1/I"); //using occuD1 to store occuD0 for e1039.
	ana_tree->Branch("occuD2", &occuD2, "occuD2/I");
	ana_tree->Branch("occuD3p", &occuD3p, "occuD3p/I");
	ana_tree->Branch("occuD3m", &occuD3m, "occuD3m/I");
	ana_tree->Branch("runID", &runID, "runID/I");
	ana_tree->Branch("spill_ID", &spill_ID, "spill_ID/I");
	ana_tree->Branch("event_ID", &event_ID, "event_ID/I");
	ana_tree->Branch("TargetPos", &TargetPos, "TargetPos/I");
	ana_tree->Branch("rfp00c", &rfp00c, "rfp00c/f");
	ana_tree->Branch("pot_p00", &pot_p00, "pot_p00/f");
	ana_tree->Branch("liveP", &liveP, "liveP/f");

	TFile* file_rec;
	TTree* tree_rec;
	
	cout<<"\n\n=================  Collecting track and event info starts ===================" <<endl;
	
	std::clock_t start;
	double duration;
	start = std::clock();

	int events_tot = 0;

	for (auto it = list_in.begin(); it != list_in.end(); it++) {
		cout << "Input = " << *it << endl;
		const string fn_raw = *it;
		const string fn_rec = *it;

		unsigned int n_evt_rec = 0;

		file_rec = new TFile(fn_rec.c_str());
		if (! file_rec) {
			cout << "Cannot get the DST tree.  Continue." << endl;
		continue;
		//exit(1);
		}

		tree_rec = (TTree*)file_rec->Get("T");
		if (! tree_rec) {
		cout << "Cannot find the rec data tree.  Continue." << endl;
		continue;
		//exit(1);
		}
		if (! m_rec) m_rec = new SRecEvent();

		SQEvent *sq_evt = 0;
		SQHitVector *sq_hitvec = 0;
                SQTrackVector* sq_trk_vec = 0;
		SRawEvent *m_org = new SRawEvent();
		tree_rec->SetBranchAddress("DST.SQEvent", &sq_evt);
		tree_rec->SetBranchAddress("DST.SQRecTrackVector", &sq_trk_vec);
		//tree_rec->SetBranchAddress("DST.SRecEvent", &m_rec);
		tree_rec->SetBranchAddress("DST.SQHitVector", &sq_hitvec);
		n_evt_rec = tree_rec->GetEntries();
 
  		cout << "N of events:  all = " << n_evt_rec << ", to be analyzed = " << n_evt_rec << endl;



  		///------- iterating over the rec tree  

  
		 for (std::size_t i_evt = 0; i_evt < n_evt_rec; i_evt++) {

			 if      ((i_evt+1) % 10000 == 0) cout << i_evt+1 << endl;
			 else if ((i_evt+1) %  1000 == 0) cout << " . " << flush;
			 //tree_raw->GetEntry(i_evt);
			 tree_rec->GetEntry(i_evt);
			 fpga1=0;
			 fpga2=0;
			 fpga3=0;
			 fpga4=0;

			 //occuD0=0; for now i will store D0 occu in occuD1 so I do not have to modify the rest of the code.
			 occuD1=0;
			 occuD2=0;
			 occuD3p=0;
			 occuD3m=0;
			 TargetPos=-9999;
			 spill_ID = -9999;
			 event_ID = 0;
			 rfp00c = 0.; ///< RF+00 corrected for the pedestal
			 pot_p00 = 0.; ///proton number corrsponding to RF+00 (pedestal corrected)
			 liveP = 0.; //Live PoT

			 pos_tracks.clear();
			 neg_tracks.clear();

			 UtilSRawEvent::SetEvent     (m_org, sq_evt, true);
			 UtilSRawEvent::SetHit       (m_org, sq_hitvec);

			 runID   = m_org->getRunID(); 
			 spill_ID = m_org->getSpillID(); 
			 event_ID = m_org->getEventID(); 



			 liveP =  UtilBeam::PoTLive(spill_ID); 
			 TargetPos = m_org->getTargetPos(); 

			 if(m_target_pos>0 && TargetPos != m_target_pos) continue;
			 if (!m_org->isTriggeredBy(SRawEvent::MATRIX1)) continue;
			 events_tot ++;
			 fpga1 = (m_org->isTriggeredBy(SRawEvent::MATRIX1));
			 fpga2 = (m_org->isTriggeredBy(SRawEvent::MATRIX2));
			 fpga3 = (m_org->isTriggeredBy(SRawEvent::MATRIX3));
			 fpga4 = (m_org->isTriggeredBy(SRawEvent::MATRIX4));

			 int rfp00 = m_org->getIntensity(0);
			 //double qie_ped = e906sc->GetQIEPedestal(spill_ID);
			 double qie_ped = 0; //set to zero for now as we do not have a datacatalog for e1039
			 rfp00c = rfp00 - qie_ped;
			 double coef = UtilBeam::PoTPerQIE(spill_ID);
			 pot_p00 = coef * rfp00c;

			 occuD1 = m_org->getNHitsInD0();
			 occuD2 = m_org->getNHitsInD2();
			 occuD3p = m_org->getNHitsInD3p();
			 occuD3m = m_org->getNHitsInD3m();

                         for (auto it = sq_trk_vec->begin(); it != sq_trk_vec->end(); it++) {
                           //int TrkMult = m_rec->getNTracks();
			 //Loop to get vector<SRecTrack> for +ve and -ve tracks from rec event
			 //for (int i =0; i< TrkMult; i++)
			 //{ 
                           //SRecTrack recTrack = m_rec->getTrack(i);
				 SRecTrack* recTrack = dynamic_cast<SRecTrack*>(*it);
				 if (recTrack->getCharge()>0)
				 {
					 pos_tracks.push_back(*recTrack);
				 }
				 if (recTrack->getCharge()<0)
				 {
					 neg_tracks.push_back(*recTrack);
				 }

			 }
			 
			 ana_tree->Fill();


		 }
		 std::cout<<std::endl;
		 if (m_use_rec) file_rec->Close();
		 std::cout<<"Current total events in the run = " <<events_tot<<std::endl;
	}
	m_file_out->cd();
	ana_tree->Write();
	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

	std::cout<<"\n================Collecting track and envent info Ended, cpu time taken: ==="<< duration <<endl;

	///
	///
	//DoVertex(ana_tree, run_id, 1);
	SortTree(ana_tree); //sort the tree
	MixTracks(sorted_tree);//Mix the sorted tree
	DoVertex(sorted_tree, run_id, 0);//Vertex the sorted tree
	DoVertex(mixed_tree, run_id, 1);//Vertex the mixed tree

	///
	///

	cout<< endl; 
}


void AnaSortMixVertex::End()
{
	m_ofs.close();
	m_file_out->cd();
	m_file_out->Close();
}



/// 
/**
 * Sort the ana_tree
 */

void AnaSortMixVertex::SortTree(TTree* tree_to_sort)
{

	cout<<"\n\n=================  Sorting starts ===================" <<endl;

	std::clock_t start;
	double duration;
	start = std::clock();

	m_file_out->cd();
	sorted_tree = (TTree*)tree_to_sort->CloneTree(0);  


	Int_t nentries = (Int_t)tree_to_sort->GetEntries();

	tree_to_sort->Draw("occuD1","","goff");

	Int_t *index = new Int_t[nentries];
	//sort array containing occuD1 in decreasing order
	//The array index contains the entry numbers in decreasing order of occuD1
	TMath::Sort(nentries,tree_to_sort->GetV1(),index);


	cout<<" No. of enteries to be sorted: "<<nentries<<endl;
	for (Int_t i=0;i<nentries;i++) {
		tree_to_sort->GetEntry(index[i]);
		tree_to_sort->LoadBaskets(2000000000);
		sorted_tree->Fill();
	}
	tree_to_sort->DropBaskets();
	//tree_to_sort = NULL; //pointed dangling ptr to NULL
	sorted_tree->SetName("sorted_tree"); 
	tree_to_sort->Delete();
	m_file_out->cd();
	sorted_tree->Write();

	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;        
	std::cout<<"\n================ Sorting Ended, cpu time taken: ==="<< duration <<endl;

	delete [] index;

}


/// Mixing tracks from sorted_tree

void AnaSortMixVertex::MixTracks(TTree* sorted_tree1)
{    

	cout<<"\n\n======================  Mixing Begins========================"<<endl;
	std::clock_t start;
	double duration;
	start = std::clock();
	
	//tree to store mixed tracks and their information
	m_file_out->cd();
	mixed_tree = new TTree("mixed_tree", "tree from mixing");
	mixed_tree->Branch("pos_tracks", &pos_tracks_mix);
	mixed_tree->Branch("neg_tracks", &neg_tracks_mix);

	mixed_tree->Branch("plus_fpga1", &plus_fpga1);
	mixed_tree->Branch("minus_fpga1", &minus_fpga1);

	mixed_tree->Branch("plus_occuD1", &plus_occuD1, "plus_occuD1/I");
	mixed_tree->Branch("plus_occuD2", &plus_occuD2, "plus_occuD2/I");
	mixed_tree->Branch("plus_occuD3m", &plus_occuD3m, "plus_occuD3m/I");
	mixed_tree->Branch("plus_occuD3p", &plus_occuD3p, "plus_occuD3p/I");
	mixed_tree->Branch("plus_rfp00c", &plus_rfp00c, "plus_rfp00c/f");
	mixed_tree->Branch("plus_pot_p00", &plus_pot_p00, "plus_pot_p00/f");
	mixed_tree->Branch("plus_liveP", &plus_liveP, "plus_liveP/f");
	mixed_tree->Branch("plus_TargetPos", &plus_TargetPos, "plus_TargetPos/I");
	mixed_tree->Branch("plus_spill_ID", &plus_spill_ID, "plus_spill_ID/I");
	mixed_tree->Branch("plus_event_ID", &plus_event_ID, "plus_event_ID/I"); 


	mixed_tree->Branch("minus_occuD1", &minus_occuD1, "minus_occuD1/I");
	mixed_tree->Branch("minus_occuD2", &minus_occuD2, "minus_occuD2/I");
	mixed_tree->Branch("minus_occuD3m", &minus_occuD3m, "minus_occuD3m/I");
	mixed_tree->Branch("minus_occuD3p", &minus_occuD3p, "minus_occuD3p/I");
	mixed_tree->Branch("minus_rfp00c", &minus_rfp00c, "minus_rfp00c/f");
	mixed_tree->Branch("minus_pot_p00", &minus_pot_p00, "minus_pot_p00/f");
	mixed_tree->Branch("minus_liveP", &minus_liveP, "minus_liveP/f");
	mixed_tree->Branch("minus_TargetPos", &minus_TargetPos, "minus_TargetPos/I");
	mixed_tree->Branch("minus_spill_ID", &minus_spill_ID, "minus_spill_ID/I");
	mixed_tree->Branch("minus_event_ID", &minus_event_ID, "minus_event_ID/I");


	std::vector<SRecTrack>* pos_tracks_ = 0;
	std::vector<SRecTrack>* neg_tracks_ = 0;

	int occuD1_,occuD2_, occuD3p_, occuD3m_,TargetPos_, fpga1_, event_ID_, spill_ID_;
	occuD1_ = occuD2_ = occuD3p_ = occuD3m_ = TargetPos_ = event_ID_ = spill_ID_ = 0;
	float rfp00c_, pot_p00_, liveP_;
	rfp00c_ = pot_p00_ = liveP_ = 0.0;
	
	//getting the input tree branches
	sorted_tree1->SetBranchAddress("pos_tracks", &pos_tracks_);
	sorted_tree1->SetBranchAddress("neg_tracks", &neg_tracks_);
	sorted_tree1->SetBranchAddress("fpga1", &fpga1_);
	sorted_tree1->SetBranchAddress("occuD1", &occuD1_);
	sorted_tree1->SetBranchAddress("occuD2", &occuD2_);
	sorted_tree1->SetBranchAddress("occuD3p", &occuD3p_);
	sorted_tree1->SetBranchAddress("occuD3m", &occuD3m_);
	sorted_tree1->SetBranchAddress("TargetPos", &TargetPos_);
	sorted_tree1->SetBranchAddress("rfp00c", &rfp00c_);
	sorted_tree1->SetBranchAddress("pot_p00", &pot_p00_);
	sorted_tree1->SetBranchAddress("liveP", &liveP_);
	sorted_tree1->SetBranchAddress("spill_ID", &spill_ID_);
	sorted_tree1->SetBranchAddress("event_ID", &event_ID_);

	cout<<"No. of entries from sorted tree: "<<sorted_tree1->GetEntries()<<endl;

	for(int i_evt=0;i_evt<sorted_tree1->GetEntries();i_evt++) 
	{

		///
		///======= Get +ve tracks from ith event  
		///
		sorted_tree1->GetEntry(i_evt);
		plus_fpga1 = fpga1_;
		plus_occuD1 = occuD1_;
		plus_occuD2 = occuD2_;
		plus_occuD3p = occuD3p_;
		plus_occuD3m = occuD3m_;
		plus_TargetPos = TargetPos_; 
		plus_rfp00c = rfp00c_;
		plus_pot_p00 = pot_p00_;  
		plus_liveP = liveP_; 
		plus_event_ID = event_ID_;
		plus_spill_ID = spill_ID_;

		for (std::size_t j=0; j<pos_tracks_->size(); ++j)
		{
			pos_tracks_mix.push_back(pos_tracks_->at(j));	      
		}

		///	
		///====== Get -ve tracks from (i+1)th event
		///
		sorted_tree1->GetEntry(i_evt+1);
		minus_fpga1 = fpga1_;
		minus_occuD1 = occuD1_;
		minus_occuD2 = occuD2_;
		minus_occuD3p = occuD3p_;
		minus_occuD3m = occuD3m_;
		minus_TargetPos = TargetPos_; 
		minus_rfp00c = rfp00c_;
		minus_pot_p00 = pot_p00_;  
		minus_liveP = liveP_;
		minus_event_ID = event_ID_;
		minus_spill_ID = spill_ID_;

		for (std::size_t k=0; k<neg_tracks_->size(); ++k) 
		{	
			neg_tracks_mix.push_back(neg_tracks_->at(k));         
		}

		mixed_tree->Fill();
		pos_tracks_mix.clear();
		neg_tracks_mix.clear();

		plus_occuD1 = 0;
		plus_occuD2 = 0;
		plus_occuD3p = 0;
		plus_occuD3m = 0;
		plus_TargetPos = 0; 
		plus_rfp00c = 0;
		plus_pot_p00 = 0;  
		plus_liveP = 0;
		plus_event_ID = 0;
		plus_spill_ID = 0;

		minus_occuD1 = 0;
		minus_occuD2 = 0;
		minus_occuD3p = 0;
		minus_occuD3m = 0;
		minus_TargetPos = 0; 
		minus_rfp00c = 0;
		minus_pot_p00 = 0;  
		minus_liveP = 0;
		minus_event_ID = 0;
		minus_spill_ID = 0;  
	}

	m_file_out->cd();
	mixed_tree->Write();  
	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;        
	std::cout<<"\n================ Mixing Ended, CPU time taken in sec: ==="<< duration<<endl;
	
}

////
/// Do the vertexing for the pos and neg tracks in input_tree.
/// 
void AnaSortMixVertex::DoVertex(TTree* input_tree, const int run_id, bool mix_flag)
{    

	cout<<"\n\n======================== Vertexing Starts ================ "<<endl;
	cout<<"Mixflag: "<<mix_flag<<endl;
	std::clock_t start;
	double duration;
	start = std::clock();

	std::cout<<"No. of events in the tree "<<input_tree->GetEntries()<<std::endl;

	recEvent = new SRecEvent();

	m_file_out->cd();
	if(!mix_flag)
	{
		//tree to store results after vertexing the sorted_tree
		save_sorted = new TTree("save_sorted", "the sorted tree after vertexing");
		save_sorted->Branch("recEvent", &recEvent, 256000, 99);
		save_sorted->Branch("fpga1", &fpga1, "fpga1/I");
		save_sorted->Branch("occuD1", &occuD1, "occuD1/I");
		save_sorted->Branch("occuD2", &occuD2, "occuD2/I");
		save_sorted->Branch("occuD3m", &occuD3m, "occuD3m/I");
		save_sorted->Branch("occuD3p", &occuD3p, "occuD3p/I");
		save_sorted->Branch("rfp00c", &rfp00c, "rfp00c/f");
		save_sorted->Branch("pot_p00", &pot_p00, "pot_p00/f");
		save_sorted->Branch("liveP", &liveP, "liveP/f");
		save_sorted->Branch("TargetPos", &TargetPos, "TargetPos/I");
		save_sorted->Branch("spill_ID", &spill_ID, "spill_ID/I");
		save_sorted->Branch("event_ID", &event_ID, "event_ID/I");
	}
	else
	{
		//tree to store results after vertexing the mixed_tree
		save_mix = new TTree("save_mix", "the mixed tree after vertexing");
		save_mix->Branch("recEvent", &recEvent, 256000, 99);
		save_mix->Branch("plus_fpga1", &plus_fpga1, "plus_fpga1/I");
		save_mix->Branch("plus_occuD1", &plus_occuD1, "plus_occuD1/I");
		save_mix->Branch("plus_occuD2", &plus_occuD2, "plus_occuD2/I");
		save_mix->Branch("plus_occuD3m", &plus_occuD3m, "plus_occuD3m/I");
		save_mix->Branch("plus_occuD3p", &plus_occuD3p, "plus_occuD3p/I");
		save_mix->Branch("plus_rfp00c", &plus_rfp00c, "plus_rfp00c/f");
		save_mix->Branch("plus_pot_p00", &plus_pot_p00, "plus_pot_p00/f");
		save_mix->Branch("plus_liveP", &plus_liveP, "plus_liveP/f");
		save_mix->Branch("plus_TargetPos", &plus_TargetPos, "plus_TargetPos/I");
		save_mix->Branch("plus_spill_ID", &plus_spill_ID, "plus_spill_ID/I");
		save_mix->Branch("plus_event_ID", &plus_event_ID, "plus_event_ID/I");

		save_mix->Branch("minus_fpga1", &minus_fpga1, "minus_fpga1/I");
		save_mix->Branch("minus_occuD1", &minus_occuD1, "minus_occuD1/I");
		save_mix->Branch("minus_occuD2", &minus_occuD2, "minus_occuD2/I");
		save_mix->Branch("minus_occuD3m", &minus_occuD3m, "minus_occuD3m/I");
		save_mix->Branch("minus_occuD3p", &minus_occuD3p, "minus_occuD3p/I");
		save_mix->Branch("minus_rfp00c", &minus_rfp00c, "minus_rfp00c/f");
		save_mix->Branch("minus_pot_p00", &minus_pot_p00, "minus_pot_p00/f");
		save_mix->Branch("minus_liveP", &minus_liveP, "minus_liveP/f");
		save_mix->Branch("minus_TargetPos", &minus_TargetPos, "minus_TargetPos/I");
		save_mix->Branch("minus_spill_ID", &minus_spill_ID, "minus_spill_ID/I");
		save_mix->Branch("minus_event_ID", &minus_event_ID, "minus_event_ID/I");

	}

	SRecDimuon recDimuon;

	std::vector<SRecTrack> * pos_tracks_input = 0;
	std::vector<SRecTrack> * neg_tracks_input = 0;

	int plus_fpga1_, plus_occuD1_,plus_occuD2_,plus_occuD3p_, plus_occuD3m_, plus_TargetPos_; 
	plus_occuD1_ =plus_occuD2_ =  plus_occuD3p_ = plus_occuD3m_ = plus_TargetPos_ = 0; 
	float plus_rfp00c_, plus_pot_p00_, plus_liveP_ ;
	plus_rfp00c_ = plus_pot_p00_ = plus_liveP_ = 0.0;

	int minus_fpga1_, minus_occuD1_, minus_occuD2_, minus_occuD3p_,minus_occuD3m_,minus_TargetPos_;
	minus_occuD1_ = minus_occuD2_ = minus_occuD3p_ = minus_occuD3m_ = minus_TargetPos_ = 0;
	float minus_rfp00c_, minus_pot_p00_, minus_liveP_;
	minus_rfp00c_ = minus_pot_p00_ = minus_liveP_ = 0.;

	int fpga1_, occuD1_, occuD2_, occuD3p_, occuD3m_, TargetPos_;
	int  event_ID_, spill_ID_, plus_event_ID_, plus_spill_ID_,minus_event_ID_, minus_spill_ID_;
	occuD1_ =  occuD2_ = occuD3p_ = occuD3m_ =TargetPos_ = 0;
	event_ID_=spill_ID_=plus_event_ID_ = plus_spill_ID_ = minus_event_ID_ = minus_spill_ID_= 0;

	float rfp00c_, pot_p00_ , liveP_;
	rfp00c_ = pot_p00_ =  liveP_ = 0;

	input_tree->SetBranchAddress("pos_tracks", &pos_tracks_input);
	input_tree->SetBranchAddress("neg_tracks", &neg_tracks_input);

	if(mix_flag)
	{
		input_tree->SetBranchAddress("plus_fpga1", &plus_fpga1_);
		input_tree->SetBranchAddress("plus_occuD1", &plus_occuD1_);
		input_tree->SetBranchAddress("plus_occuD2", &plus_occuD2_);
		input_tree->SetBranchAddress("plus_occuD3m", &plus_occuD3m_);
		input_tree->SetBranchAddress("plus_occuD3p", &plus_occuD3p_);
		input_tree->SetBranchAddress("plus_rfp00c", &plus_rfp00c_);
		input_tree->SetBranchAddress("plus_pot_p00", &plus_pot_p00_);
		input_tree->SetBranchAddress("plus_liveP", &plus_liveP_);
		input_tree->SetBranchAddress("plus_TargetPos", &plus_TargetPos_);
		input_tree->SetBranchAddress("plus_spill_ID", &plus_spill_ID_);
		input_tree->SetBranchAddress("plus_event_ID", &plus_event_ID_);

		input_tree->SetBranchAddress("minus_fpga1", &minus_fpga1_);
		input_tree->SetBranchAddress("minus_occuD1", &minus_occuD1_);
		input_tree->SetBranchAddress("minus_occuD2", &minus_occuD2_);
		input_tree->SetBranchAddress("minus_occuD3m", &minus_occuD3m_);
		input_tree->SetBranchAddress("minus_occuD3p", &minus_occuD3p_);
		input_tree->SetBranchAddress("minus_rfp00c", &minus_rfp00c_);
		input_tree->SetBranchAddress("minus_pot_p00", &minus_pot_p00_);
		input_tree->SetBranchAddress("minus_liveP", &minus_liveP_);
		input_tree->SetBranchAddress("minus_TargetPos", &minus_TargetPos_);
		input_tree->SetBranchAddress("minus_spill_ID", &minus_spill_ID_);
		input_tree->SetBranchAddress("minus_event_ID", &minus_event_ID_);
	}
	else
	{
		input_tree->SetBranchAddress("fpga1", &fpga1_);
		input_tree->SetBranchAddress("occuD1", &occuD1_);
		input_tree->SetBranchAddress("occuD2", &occuD2_);
		input_tree->SetBranchAddress("occuD3p", &occuD3p_);
		input_tree->SetBranchAddress("occuD3m", &occuD3m_);
		input_tree->SetBranchAddress("TargetPos", &TargetPos_);
		input_tree->SetBranchAddress("rfp00c", &rfp00c_);
		input_tree->SetBranchAddress("pot_p00", &pot_p00_);
		input_tree->SetBranchAddress("liveP", &liveP_);
		input_tree->SetBranchAddress("spill_ID", &spill_ID_);
		input_tree->SetBranchAddress("event_ID", &event_ID_);
	}

	for(int i_evt=0;i_evt<input_tree->GetEntries();i_evt++) 
	{
		input_tree->GetEntry(i_evt);

		occuD1 = 0;

		plus_occuD1 = 0;
		plus_occuD2 = 0;
		plus_occuD3p = 0;
		plus_occuD3m = 0;
		plus_TargetPos = -999; 
		plus_rfp00c = 0;
		plus_pot_p00 = 0;  
		plus_liveP = 0;
		plus_spill_ID = 0;
		plus_event_ID = 0;

		minus_occuD1 = 0;
		minus_occuD2 = 0;
		minus_occuD3p = 0;
		minus_occuD3m = 0;
		minus_TargetPos = -999; 
		minus_rfp00c = 0;
		minus_pot_p00 = 0;  
		minus_liveP = 0;
		minus_spill_ID = 0;
		minus_event_ID = 0;

		spill_ID = 0;
		event_ID = 0;
		
		if(!mix_flag) {
			spill_ID = spill_ID_;
			event_ID = event_ID_;	
		}
		
		plus_fpga1 = plus_fpga1_;
		plus_occuD1 = plus_occuD1_;
		plus_occuD2 = plus_occuD2_;
		plus_occuD3p = plus_occuD3p_;
		plus_occuD3m = plus_occuD3m_;
		plus_TargetPos = plus_TargetPos_; 
		plus_rfp00c = plus_rfp00c_;
		plus_pot_p00 = plus_pot_p00_;  
		plus_liveP = plus_liveP_;
		plus_spill_ID = plus_spill_ID_;
		plus_event_ID = plus_event_ID_;

		minus_fpga1 = plus_fpga1_;
		minus_occuD1 = minus_occuD1_;
		minus_occuD2 = minus_occuD2_;
		minus_occuD3p = minus_occuD3p_;
		minus_occuD3m = minus_occuD3m_;
		minus_TargetPos = minus_TargetPos_; 
		minus_rfp00c = minus_rfp00c_;
		minus_pot_p00 = minus_pot_p00_;  
		minus_liveP = minus_liveP_;
		minus_spill_ID = minus_spill_ID_;
		minus_event_ID = minus_event_ID_;

		occuD1 = 0;
		occuD2 = 0;
		occuD3p = 0;
		occuD3m = 0;
		TargetPos = -9999; 
		rfp00c = 0;
		pot_p00 = 0;  
		liveP = 0;

		fpga1 = fpga1_;
		occuD1 = occuD1_;
		occuD2 = occuD2_;
		occuD3p = occuD3p_;
		occuD3m = occuD3m_;
		TargetPos = TargetPos_; 
		rfp00c = rfp00c_;
		pot_p00 = pot_p00_;  
		liveP = liveP_;
		
		if(i_evt%1000==0) cout<<"No. of events analyzed: "<<i_evt<<endl;
		
		std::vector<SRecTrack>  pos_tracks_hold, neg_tracks_hold ;

		for (std::size_t j=0; j<pos_tracks_input->size(); ++j)
		{
			SRecTrack* trk_p = &(pos_tracks_input->at(j)); 
			pos_tracks_hold.push_back(*trk_p);	  
			recEvent->insertTrack(*trk_p);
		}


		for (std::size_t k=0; k<neg_tracks_input->size(); ++k) 
		{
			SRecTrack* trk_n = &(neg_tracks_input->at(k));
			neg_tracks_hold.push_back(*trk_n);
			recEvent->insertTrack(*trk_n);
		}

		for(std::size_t j=0; j<pos_tracks_hold.size(); ++j)
		{
			for (std::size_t k=0; k<neg_tracks_hold.size(); ++k)
			{


				//Implementing FPGA1 trigger roadset requirement
				if((pos_tracks_hold.at(j).getTriggerRoad() > 0 && neg_tracks_hold.at(k).getTriggerRoad() > 0) || (pos_tracks_hold.at(j).getTriggerRoad() < 0 && neg_tracks_hold.at(k).getTriggerRoad() < 0)) continue;

				

				SRecTrack* track1 = &pos_tracks_hold.at(j);
				SRecTrack* track2 = &neg_tracks_hold.at(k);
				if(!track1->isKalmanFitted() || !track2->isKalmanFitted()) continue;

				if(m_vtxfit->processOneDimuon(track1, track2, recDimuon)){
					if (track1->getStateVector(0).GetNcols()==0) std::cout<<"track1 Ncols ==0 "<<std::endl;
					if (track2->getStateVector(0).GetNcols()==0) std::cout<<"track2 Ncols ==0 "<<std::endl;
					
					recDimuon.set_track_id_pos(static_cast<int>(j));
					recDimuon.set_track_id_neg(static_cast<int>(k) + static_cast<int>(pos_tracks_hold.size()));
					recDimuon.calcVariables();
					
					recEvent->insertDimuon(recDimuon);		
				}
			}	     
		}

		if(mix_flag) save_mix->Fill();
		else save_sorted->Fill();
		recEvent->clear();
		pos_tracks_hold.clear();
		neg_tracks_hold.clear();
	}

	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;        
	std::cout<<"\n================ Vertexing Ended, CPU time taken in sec: ==="<< duration<<endl;
	m_file_out->cd();
	if (mix_flag) save_mix->Write();
	else save_sorted->Write();
}



//currently no embedding is done in e1039. So, commenting out all embedding stuffs.

///MC preperation for embedding
/*
void AnaSortMixVertex::PrepareMC(char* fn_mctree)
{    


	///Create a tree filled with pos-neg tracks from MC for embedding

	std::vector<SRecTrack> * pos_tracks = 0;
	std::vector<SRecTrack> * neg_tracks = 0;
	std::vector<double> dim_M_mc;

	TTree* mc_tree = new TTree("mc_tree", "Tree with MC for mixing");
	mc_tree->Branch("pos_tracks",&pos_tracks);
	mc_tree->Branch("neg_tracks",&neg_tracks);
	mc_tree->Branch("dim_M_mc", &dim_M_mc);


	///Reading MC signal tree to be embedded
	SRecEvent* recEvent = new SRecEvent();
	SRawMCEvent* raw = new SRawMCEvent();

	cout<<"Reading MC tree: "<<fn_mctree<<endl;

	TFile* mc_file = new TFile (fn_mctree, "read");
	TTree* save = (TTree*)mc_file->Get("save");

	save->SetBranchAddress("recEvent",&recEvent);
	save->SetBranchAddress("rawEvent",&raw);


	for(int i_evt=0;i_evt<save->GetEntries();i_evt++) 
	{
		save->GetEntry(i_evt);

		pos_tracks->clear();
		neg_tracks->clear();
		dim_M_mc.clear();

		if(raw->isTriggeredBy(SRawEvent::MATRIX1)) continue;   

		if(recEvent->getNDimuons()<1) continue;       
		for (int i =0; i<recEvent->getNTracks(); i++)
		{    
			SRecTrack recTrack = recEvent->getTrack(i);
			if (recTrack.getCharge()>0) pos_tracks->push_back(recTrack);
			if (recTrack.getCharge()<0) neg_tracks->push_back(recTrack);	 	    
		}

		if((pos_tracks->at(0).getTriggerRoad() > 0 && neg_tracks->at(0).getTriggerRoad() > 0) || (pos_tracks->at(0).getTriggerRoad() < 0 && neg_tracks->at(0).getTriggerRoad() < 0)) continue;


		SRecDimuon recDimuon_mc = recEvent->getDimuon(0);
		dim_M_mc.push_back(recDimuon_mc.mass);

		mc_tree ->Fill();

	}


}


///
*/
void AnaSortMixVertex::EmbedMCSignal( char *file_mixed,  char* file_mcsignal)
{

	///Creating TTree with mixed with mc signal embedded
	cout<<"creating tree to store embedded event"<<endl;
	TTree* mixed_embed_tree = new TTree("tree","MC signals embeded on mixed tree");

	std::vector<SRecTrack> * pos_tracks = 0;
	std::vector<SRecTrack> * neg_tracks = 0;
	std::vector<double> dim_M_mixed, dim_M_mc;
	mixed_embed_tree->Branch("pos_tracks",&pos_tracks);
	mixed_embed_tree->Branch("neg_tracks",&neg_tracks);
	//mixed_embed_tree->Branch("dim_M_mixed", &dim_M_mixed);
	mixed_embed_tree->Branch("dim_M_mc", &dim_M_mc);

	///Reading MC signal tree to be embedded

	cout<<"Reading MC tree: "<<file_mcsignal<<endl;


	std::vector<SRecTrack> * pos_tracks_mc = 0;
	std::vector<SRecTrack> * neg_tracks_mc = 0;
	std::vector<double> * dim_M_mc_ = 0;

	TFile* mc_file = new TFile (file_mcsignal, "read");
	TTree* mc_tree = (TTree*)mc_file->Get("mc_tree");


	mc_tree->SetBranchAddress("pos_tracks", &pos_tracks_mc);
	mc_tree->SetBranchAddress("neg_tracks", &neg_tracks_mc);
	mc_tree->SetBranchAddress("dim_M_mc", &dim_M_mc_);


	//Reading "T" tree form fn_mixedtree
	std::vector<SRecTrack> * pos_tracks_ = 0;
	std::vector<SRecTrack> * neg_tracks_ = 0;
	cout<<"Reading Mixed tree: "<<file_mixed<<endl;

	TFile* _mixedFile = new TFile (file_mixed,"read");
	TTree* T = (TTree*)_mixedFile->Get("tree");

	T->SetBranchAddress("pos_tracks", &pos_tracks_);
	T->SetBranchAddress("neg_tracks", &neg_tracks_);



	int i_mc_evt=0;
	cout<<"Total entries in the mixed tree: "<<T->GetEntries()<<endl;

	for(int i_evt=0;i_evt<T->GetEntries();i_evt++) 
	{
		T->GetEntry(i_evt);
		pos_tracks->clear();
		neg_tracks->clear();
		dim_M_mc.clear();


		///Get +ve tracks  

		for (std::size_t j=0; j<pos_tracks_->size(); ++j)
		{
			pos_tracks->push_back(pos_tracks_->at(j));	      
		}


		///Get -ve tracks 
		///
		for (std::size_t k=0; k<neg_tracks_->size(); ++k) 
		{	
			neg_tracks->push_back(neg_tracks_->at(k));
		}


		///embed the mc events===

		if(i_evt%25==0)//embed every 30 mixed events
		{
			mc_tree->GetEntry(i_mc_evt);
			pos_tracks->push_back(pos_tracks_mc->at(0)); 
			neg_tracks->push_back(neg_tracks_mc->at(0));
			dim_M_mc.push_back(dim_M_mc_->at(0)); 
			//cout<<"mc_dimuon\t"<<dim_M_mc_->at(0)<<endl;
			i_mc_evt++;   

		}


		mixed_embed_tree ->Fill();


	}


}

