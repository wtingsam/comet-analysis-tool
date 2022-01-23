//==================================================================
//    This program is for calibration of the cosmic ray tracks
//
//-----------------------------------------------------------------
//** Data format
//
//    Unit: mm/ns
//
//-----------------------------------------------------------------
//  Author: Sam Wong
//    Date: 28/9/2016
//==================================================================
#include "TFile.h"
#include "TTree.h"

#include <vector>
#include <iostream>
#include <ctime>

#include "TH2D.h"
#include "TMath.h"
#include "TStyle.h"
#include "TAxis.h"
#include "TF1.h"
#include <unistd.h> //get opt

#include "WireManager.hxx"
//#include "InputRootCRT.hxx"
#include "RunLogManager.hxx"
#include "ICDCGas.hxx"
#include "Iteration.hxx"
#include "EventDisplay.hxx"

#include "ITrackingUtility.hxx"
#include "EventLoop.hxx"
#include "IOStreamFunction.hxx"
#include "CalibrationFile.hxx"
#include "IRunTimeManager.hxx"

//#include "IScanParameterSpace.hxx"

#include "IHit.hxx"
#include "ITrackContainer.hxx"
//#include "IHitSelection.hxx"
#include "IUtility.hxx"
#include "ITrackFinderLinear.hxx"
#include "ITrackFitterLinear.hxx"
#include "ITrackFitterMultPeakLinear.hxx"

class MyAnalysisCode : public EventLoop{
    private:
        bool fDEBUG;
        bool fAnalyzeSimData;
        int fTestLayer;
        int fFileID;
        int fFromIEV;
        int fToIEV;
        TString fPrefix;
        IHit *fRawHitCon;
        ITrackContainer *fTrackCon;
        EventDisplay *fEventDisplay;
        // Inputs
        int fTriggerNumber_in;
        int fNumOfHits_in;
        int fNumOfEvents_in;
        std::vector<bool>*fGoodHitFlag_in;
        std::vector<bool>*fChoosePeakFlag_in;
        std::vector<int>*fNumOfLayHits_in;
        std::vector<int>*fNumOfPeaks_in;
        std::vector<int>*fPeakWidth_in;
        std::vector<int>*fLayerID_in;
        std::vector<int>*fCellID_in;
        std::vector<double>*fDriftTime1Peak_in;
        std::vector< std::vector<double> > *fDriftTimeAll_in;
        std::vector< std::vector<double> > *fWaveform_in;
        std::vector< std::vector<double> > *fWaveformClk_in;
        std::vector<double>*fTestLayerDriftTAll_in; //all drift times of test layers
        std::vector<double>*fADCFirstPeak_in;
        std::vector<double>*fPedestal_in;
        std::vector<double>*fQ_in;
        std::vector<double>*fT0_in;
        std::vector<double>*fT0Offset_in;
        std::vector<double>*fADCCut_in;
        // mc
        std::vector<double>*fMCtrackPar_in;
        std::vector<double>*fMCDriftDistance_in;
        std::vector<double>*fMCDriftDistanceSmear_in;
        TFile *fFile_in;
        TTree *fTree_in;
        // Outputs
        TFile *fFile_out;
        TTree *fTree_out;

        int fTriggerNumber_out;
        int fIEV_out;
        /***
            EQ     Comment
            0     Trigger skip,
            1     Good Events
            10    Good Events (Chi2/DOF<10)
            2     Number of hits smaller than fitting DOF (4 linear,5 helix)
            3     Number of hits larger  than 50 hits //fixeme
            4     Cannot pass track finding condition
            5     Cannot recover tracks in track finding
        ***/
        int fNumOfHits_out;

        //Fitting parameters
        std::vector<double>fTrackPar_out;
        std::vector<double>fTrackParErr_out;
        int fCandidateIndex;
        int fSmallestResCellOfTestLayer_out;

        std::vector<int> fHitSelectedFlag_out;
        std::vector<int> fBestCellOnTestLayerFlag_out;
        std::vector<bool> fGoodHitFlag_out;
        std::vector<bool> fChoosePeakFlag_out;
        std::vector<int> fNumOfLayHits_out;
        std::vector<int> fNumOfPeaks_out;
        std::vector<int> fPeakWidth_out;
        std::vector<int> fLayerID_out;
        std::vector<int> fCellID_out;
        std::vector<int> fClosestCell_out; // The closest cell for this layer
        std::vector<double> fDriftTime1Peak_out;
        std::vector< std::vector<double> > fDriftTimeAll_out;
        std::vector<double> fHitXPosOnTrack_out;
        std::vector<double> fHitYPosOnTrack_out;
        std::vector<double> fHitZPosOnTrack_out;
        std::vector<double> fHitXPosOnWire_out;
        std::vector<double> fHitYPosOnWire_out;
        std::vector<double> fHitZPosOnWire_out;
        std::vector<double> fADCFirstPeak_out;
        std::vector<std::vector<double> >fCandidateTracks_out;
        std::vector<double> fQ_out;
        std::vector<double> fT0_out;
        std::vector<double> fPhiAng_out; // Angle between track and wire
        std::vector<double> fBetaAng_out; // Angle for determing the cell shape
        std::vector<int> fLeftRight_out;
        std::vector<double> fHitR_out;
        std::vector<double> fHitR_best_out;
        std::vector<double> fFitR_out;
        double fTheChi2_out;
        double fTheChi2p_out;
        int    fTheDOF_out;
        int    fTheWire_out; // the selected wire for testing layer
        double fQEvent_out;
        std::vector<double> fChi2_out;
        std::vector<double> fChi2p_out;
        std::vector<int> fDOF_out;
        std::vector<double> fTrackIndex_out;
        int fTrkIdMinResTestLay_out; // the track with smallest resolution for test layer
        int fTrkIdMinChi2TestLay_out; // the track with smallest chi2
        std::vector<double> fMCtrackPar_out;
        std::vector<double> fMCDriftDistance_out;
        std::vector<double> fMCDriftDistanceSmear_out;

        // Calibration
        //double fT0Calib[20][700]; //Max 5000 channel
    public:
        MyAnalysisCode(int fileID,int testLayer, TString inputPath, TString inputTree="t",TString prefix=""):
            fDEBUG(false),fAnalyzeSimData(false),
            fTestLayer(testLayer),fFileID(fileID),fPrefix(prefix),
            fRawHitCon(new IHit("Raw hits")),
            fEventDisplay(new EventDisplay())
        {
            printf("%% Start analysis: InputPath=%s inputTree=%s \n",inputPath.Data(),inputTree.Data());

            Begin();
            fFile_in = TFile::Open(inputPath);
            fTree_in = (TTree*) fFile_in->Get(inputTree);
        }
        void AnalyzeSimData(bool val=false){
            fAnalyzeSimData = val;
        }
        void Begin(){
            fTrkIdMinResTestLay_out = -1;
            fTrkIdMinChi2TestLay_out = -1;
            fFromIEV = -1;
            fToIEV = -1;
            fFile_out=NULL;
            fTree_out=NULL;
            fFile_in=NULL;
            fTree_in=NULL;
            fTriggerNumber_in=-1;
            fNumOfHits_in=-1;
            fNumOfEvents_in=-1;

            fGoodHitFlag_in=0;
            fChoosePeakFlag_in=0;
            fNumOfLayHits_in=0;
            fNumOfPeaks_in=0;
            fPeakWidth_in=0;
            fLayerID_in=0;
            fCellID_in=0;
            fDriftTime1Peak_in=0;
            fDriftTimeAll_in=0;
            fWaveform_in=0;
            fWaveformClk_in=0;
            fTestLayerDriftTAll_in=0;
            fADCFirstPeak_in=0;
            fPedestal_in=0;
            fQ_in=0;
            fT0_in=0;
            fT0Offset_in=0;
            fADCCut_in=0;
            // mc
            fMCtrackPar_in = 0;
            fMCDriftDistance_in = 0;
            fMCDriftDistanceSmear_in = 0;
        }

        void Run1EventOnly(int from, int to){
            fFromIEV = from;
            fToIEV = to;
        }

        void SetInputBranchAddress(){
            fTree_in->SetBranchAddress("goodHitFlag", &fGoodHitFlag_in);
            fTree_in->SetBranchAddress("choosePeakFlag", &fChoosePeakFlag_in);
            fTree_in->SetBranchAddress("triggerNumber", &fTriggerNumber_in);
            fTree_in->SetBranchAddress("nHit", &fNumOfHits_in);
            fTree_in->SetBranchAddress("nHitLayer", &fNumOfLayHits_in);
            fTree_in->SetBranchAddress("nPeaks", &fNumOfPeaks_in);
            fTree_in->SetBranchAddress("peakWidth", &fPeakWidth_in);
            fTree_in->SetBranchAddress("layerID", &fLayerID_in);
            fTree_in->SetBranchAddress("cellID", &fCellID_in);
            fTree_in->SetBranchAddress("driftTime", &fDriftTime1Peak_in);
            fTree_in->SetBranchAddress("driftTAll", &fDriftTimeAll_in);
            fTree_in->SetBranchAddress("waveform", &fWaveform_in);
            fTree_in->SetBranchAddress("waveformClk", &fWaveformClk_in);
            fTree_in->SetBranchAddress("adcPeak", &fADCFirstPeak_in);
            fTree_in->SetBranchAddress("q", &fQ_in);
            fTree_in->SetBranchAddress("t0", &fT0_in);
            fTree_in->SetBranchAddress("adccut", &fADCCut_in);
            fTree_in->SetBranchAddress("t0offset", &fT0Offset_in);
            fTree_in->SetBranchAddress("ped", &fPedestal_in);
            if(fAnalyzeSimData){
                fTree_in->SetBranchAddress("trackPar_mc",&fMCtrackPar_in);
                fTree_in->SetBranchAddress("driftD_mc",&fMCDriftDistance_in);
                fTree_in->SetBranchAddress("driftD_smear_mc",&fMCDriftDistanceSmear_in);
            }
            fNumOfEvents_in = fTree_in->GetEntries();
        }

        void SetOutput(){
            char *outputXtDir=getenv("CCTRACKROOT_DIR");
            TString filename = Form("%s/",outputXtDir);
            filename+=fPrefix+".root";
            fFile_out = new TFile(filename,"recreate");
            fTree_out = new TTree("t","reconstructed");
            // event level
            fTree_out->Branch("nHits", &fNumOfHits_out);
            fTree_out->Branch("triggerNumber", &fTriggerNumber_out);
            fTree_out->Branch("IEV", &fIEV_out);
            fTree_out->Branch("trkIdMinResTestLayt", &fTrkIdMinResTestLay_out);
            fTree_out->Branch("trkIdMinChi2TestLay", &fTrkIdMinChi2TestLay_out);

            fTree_out->Branch("testLayer", &fTestLayer);
            // hit level
            fTree_out->Branch("chi2", &fChi2_out);
            fTree_out->Branch("chi2p", &fChi2p_out);
            fTree_out->Branch("DOF", &fDOF_out);
            fTree_out->Branch("trackPar", &fTrackPar_out);

            fTree_out->Branch("layerID", &fLayerID_out);
            fTree_out->Branch("cellID", &fCellID_out);
            fTree_out->Branch("closestCell", &fClosestCell_out);
            fTree_out->Branch("driftTime", &fDriftTime1Peak_out);
            fTree_out->Branch("driftTimeAll", &fDriftTimeAll_out);
            fTree_out->Branch("Qevent", &fQEvent_out);
            fTree_out->Branch("hitR", &fHitR_out);
            fTree_out->Branch("hitRBest", &fHitR_best_out);
            fTree_out->Branch("fitR", &fFitR_out);
            fTree_out->Branch("phiAng", &fPhiAng_out);
            fTree_out->Branch("trackIndex", &fTrackIndex_out);
            if(fAnalyzeSimData){
                // track level
                fTree_out->Branch("trackPar_mc",&fMCtrackPar_in);
                // hit level
                fTree_out->Branch("driftD_mc",&fMCDriftDistance_out);
                fTree_out->Branch("driftD_smear_mc",&fMCDriftDistanceSmear_out);
            }
        }

        void FillOutputTree(){

            std::vector<double> chi2_checked_repeat;
            // Get all tracks saved so far
            const int nTracks = fTrackCon->GetNTracks();
            printf("=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=\n");
            double best_residue_at_testLayer = 1e5;
            double best_chi2_at_testLayer = 1e5;
            for(int iTrk=0;iTrk<nTracks;iTrk++){
                printf("\n");
                ITrackSummary a_trackSummary = fTrackCon->GetTrackSummaries()[iTrk];
                double trackPar[4];
                // Get infomation from track container
                a_trackSummary.GetTrackPar(trackPar);

                double chi2  = a_trackSummary.GetChi2();
                double DOF   = a_trackSummary.GetDOF();
                double chi2p = TMath::Prob(chi2,DOF);
                // Save only unique chi2
                if(IUtility::Repeated(&chi2_checked_repeat,chi2)) continue;
                else chi2_checked_repeat.push_back(chi2);
                // Get information from hit container
                fNumOfHits_out      = fRawHitCon->GetNumOfHits();
                std::vector<int> tmp_LayerID_out        = fRawHitCon->GetHitLayer();
                std::vector<int> tmp_CellID_out         = fRawHitCon->GetHitCell();
                std::vector<double>tmp_DriftTime1Peak_out = fRawHitCon->GetDriftTime();
                std::vector<std::vector<double> > tmp_DriftTimeAll_out   = fRawHitCon->GetDriftTimeAll();
                // Loop hits
                double total_chi2=0; // includes test layer..
                int total_dof=0;
                for(int iHit=0;iHit<fNumOfHits_out;iHit++){
                    // Information
                    int layer = tmp_LayerID_out[iHit];
                    int cell  = tmp_CellID_out[iHit];
                    double fitR = ITrackingUtility::GetFitDistance(layer,cell,trackPar);
                    double phi = ITrackingUtility::GetIncidentAngle(layer, cell, trackPar);
                    std::vector<double> v_dt_tmp = tmp_DriftTimeAll_out[iHit];

                    // Choose best hitR among 32 sample hits
                    double hitR = ICDCGas::Get().GetDriftDistance(v_dt_tmp[0],layer,cell,phi,0);
                    double hitR_best_sample=hitR;
                    double time_best_sample=v_dt_tmp[0];
                    double smallest_res_sample_hit=1e6;
                    // Get the hit R of the first hit
                    // Loop over 32 sample hits
                    for(int s=0;s<(int)v_dt_tmp.size();s++){
                        double tmp_hitR = ICDCGas::Get().GetDriftDistance(v_dt_tmp[s],layer,cell,phi,0);
                        double res  = fabs(tmp_hitR-fitR);
                        // Choose the best sample hit by picking up the minimum residue one
                        if(res<smallest_res_sample_hit){
                            smallest_res_sample_hit = res;
                            hitR_best_sample        = tmp_hitR;
                            time_best_sample        = v_dt_tmp[s];
                        }
                    }
                    // Find the closest cell
                    int closest_cell = cell;
                    double smallest_res = 1e5;
                    // Loop over all cell in this layer
                    for(int iCell=0;iCell<WireManager::Get().GetNumSenseWires(layer);iCell++){
                        double jfitR  = ITrackingUtility::GetFitDistance(layer,iCell,trackPar);
                        // Pick up a cell with smallest residue
                        double tmp_res = fabs(hitR_best_sample-jfitR);
                        if(tmp_res<smallest_res){
                            smallest_res=tmp_res;
                            closest_cell=iCell;
                        }
                    }
                    // Define the best res
                    double pres_best_sample = fabs(hitR_best_sample-fitR);
                    // Choosing the best track by using also test layer
                    if(layer==fTestLayer &&
                       closest_cell==cell &&
                       pres_best_sample<best_residue_at_testLayer
                        ){
                        best_residue_at_testLayer=pres_best_sample;
                        fTrkIdMinResTestLay_out=iTrk;
                    }
                    if(closest_cell==cell){
                        double delta = (hitR_best_sample-fitR)/0.2;
                        delta*=delta;
                        total_chi2+=delta;
                        total_dof++;
                    }
                    printf("TN %d IEV %d | Chi2 %8.3f DOF %3.0f chi2p %5.4f | iTrk %d [%d][%2d] BestCell %d %8.3f(H) %8.3f(F) dR %8.3f -- %s\n",
                           fTriggerNumber_out,
                           fIEV_out,
                           chi2,DOF,chi2p,
                           iTrk,
                           layer,cell,closest_cell,
                           hitR_best_sample,ITrackingUtility::GetFitDistance(layer,cell,trackPar),
                           hitR_best_sample-fitR,
                           (layer==fTestLayer)? "testlayer":""
                           );
                    // push back
                    fLayerID_out        .push_back(layer);
                    fCellID_out         .push_back(cell);
                    fDriftTime1Peak_out .push_back(time_best_sample);
                    fDriftTimeAll_out   .push_back(v_dt_tmp);

                    fPhiAng_out.push_back(phi);
                    fFitR_out.push_back(fitR);
                    fHitR_out.push_back(hitR);
                    fHitR_best_out.push_back(hitR_best_sample);
                    fClosestCell_out.push_back(closest_cell);
                    fTrackIndex_out.push_back(iTrk);
                    // Push back output vectors
                    fChi2_out.push_back(chi2);
                    fChi2p_out.push_back(chi2p);
                    fDOF_out.push_back(DOF);
                }
                printf("%d Total Chi2 (include testlayer) : %f %d \n",iTrk, total_chi2,total_dof-4);
                if(total_chi2<best_chi2_at_testLayer){
                    fTrkIdMinChi2TestLay_out=iTrk;
                    best_chi2_at_testLayer=total_chi2;
                }

                for(int w=0;w<4;w++)
                    fTrackPar_out.push_back(trackPar[w]);
            }
            printf("\nResult: fTrkIdMinResTestLay_out %d \n",fTrkIdMinResTestLay_out);
            printf("\nResult: fTrkIdMinChi2TestLay_out %d \n",fTrkIdMinChi2TestLay_out);
            fTree_out->Fill();
        }

        void Process(){
            SetInputBranchAddress();
            SetOutput();
            if(fFromIEV>=0 && fToIEV>=0){
                // Debug when you only look at one event
                //if(fToIEV-fFromIEV==1)fDEBUG=true;
                //fDEBUG=true;
                printf("### Reading events %d ~ %d \n",fFromIEV,fToIEV);
                StartEventLoop(fFromIEV,fToIEV);
            }else if(fFromIEV>=0 && fToIEV<0){
                fDEBUG=true;
                printf("### Reading events %d ~ %d \n",fFromIEV,fFromIEV+1);
                StartEventLoop(fFromIEV,fFromIEV+1);
            }else{
                printf("### Reading events %d ~ %d \n",0,fNumOfEvents_in);
                StartEventLoop(0,fNumOfEvents_in);
            }
            End();
        }

        void Clear(){
            fQEvent_out=0;
            fTheChi2_out=-1;
            fTheChi2p_out=-1;
            fTheDOF_out=-1;
            fTheWire_out=-1;
            // Object
            if(fRawHitCon) fRawHitCon->Clear();
            // double/integer
            fCandidateIndex=-1;
            fSmallestResCellOfTestLayer_out=-1;

            for(int i=0;i<4;i++){
                fTrackPar_out.clear();
                fTrackParErr_out.clear();
            }
            // vectors
            fHitSelectedFlag_out.clear();
            fBestCellOnTestLayerFlag_out.clear();
            fGoodHitFlag_out.clear();
            fChoosePeakFlag_out.clear();
            fNumOfLayHits_out.clear();
            fNumOfPeaks_out.clear();
            fPeakWidth_out.clear();
            fLayerID_out.clear();
            fCellID_out.clear();
            fClosestCell_out.clear();
            fDriftTime1Peak_out.clear();
            fDriftTimeAll_out.clear();
            fHitXPosOnTrack_out.clear();
            fHitYPosOnTrack_out.clear();
            fHitZPosOnTrack_out.clear();
            fHitXPosOnWire_out.clear();
            fHitYPosOnWire_out.clear();
            fHitZPosOnWire_out.clear();
            fADCFirstPeak_out.clear();
            fCandidateTracks_out.clear();
            fQ_out.clear();
            fT0_out.clear();
            fPhiAng_out.clear();
            fBetaAng_out.clear();
            fLeftRight_out.clear();
            fHitR_out.clear();
            fHitR_best_out.clear();
            fFitR_out.clear();
            fChi2_out.clear();
            fChi2p_out.clear();
            fDOF_out.clear();
            fTrackIndex_out.clear();
            fMCDriftDistance_out.clear();
            fMCDriftDistanceSmear_out.clear();
        }
        void StartEventLoop(int fromIEV, int toIEV){
            // Start counting time
            clock_t start, diff;
            start = clock();
            // Event loop
            for(int iev=fromIEV;iev<toIEV;iev++){
                EventLoop::ShowProgress(iev,fNumOfEvents_in);
                fTree_in->GetEntry(iev);
                fTriggerNumber_out=fTriggerNumber_in;
                fIEV_out=iev;
                if(fNumOfHits_in==0){
                    printf("## %d no hit at all (Considered as wrong triggered event)\n",fTriggerNumber_in);
                    return ;
                }
                if(fDEBUG){
                    printf("=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@\n");
                    printf("=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@\n");
                    printf("Beginning of event iev %d tn %d \n",iev,fTriggerNumber_in);
                }
                double avg_q            = 0;
                int    nHits            = 0;
                int    nHitsNoTestLay   = 0;
                int    nHitsWith1LayHit = 0;
                bool   testLayerHitFlag = false;
                // Make hit list with selection
                std::vector<double> dt_tmp;
                std::vector<std::vector<double> >dt_all_tmp;
                std::vector<std::vector<double> >v_wf_tmp;
                std::vector<std::vector<double> >v_wf_clk_tmp;
                std::vector<int> l_tmp;
                std::vector<int> c_tmp;
                std::vector<double> q_tmp;
                std::vector<double> p_tmp;
                std::vector<double> qcut_tmp;
                std::vector<int> nlh_tmp;
                std::vector<int> np_tmp;
                std::vector<int> pw_tmp;
                std::vector<bool> cp_tmp;
                std::vector<double> max_q(20);
                std::vector<double> max_peak(20);
                std::vector<double> hitR_tmp;
                std::vector<double> mc_r_tmp;
                std::vector<double> mc_rs_tmp;
                std::vector<int>left_right_tmp;
                std::vector<int>hit_flag_select_tmp;
                //pre set hits for IHit.cxx
                double chen_offset=3.125;
                for(int iHit=0;iHit<fNumOfHits_in;iHit++){// Hit loop
                    int     layer =         fLayerID_in->at(iHit);
                    int     cell  =         fCellID_in->at(iHit);
                    if(layer==0)continue;
                    double q = 0;
                    double q_cut_tmp = 30;
                    // fix negative drift time
                    //double dt = fDriftTime1Peak_in->at(iHit) - fT0Calib[layer][cell]; //FIXME
                    double dt = fDriftTime1Peak_in->at(iHit); //FIXME
                    dt-=chen_offset;
                    //double dt = fabs(fMCDriftDistance_in->at(iHit))/0.023; //FIXME
                    // Throw away small drift time hits
                    if(layer!=fTestLayer){ // Should be added to finding and fitting
                        if(dt<-10){
                            q=-1000;
                        }else if(dt>=-10 && dt <0){
                            dt = 0;
                            q=fQ_in->at(iHit);
                        }else if(dt>400){
                            q=-1000;
                        }else{
                            q=fQ_in->at(iHit);
                        }
                    }else{
                        q=fQ_in->at(iHit);
                        testLayerHitFlag=true;
                    }
                    // Find out maximum q in the layer
                    if(q>max_q[layer])max_q[layer]=q;
                    double peak = fADCFirstPeak_in->at(iHit);
                    if(peak>max_peak[layer])max_peak[layer]=peak;
                    // Fill in hit list
                    int select = 0;
                    double hitR=fabs(ICDCGas::Get().GetDriftDistance(dt,layer,cell,0,0));
                    if(q>q_cut_tmp && hitR<8.3){ //a random number for hiyR
                        // Push back all hits for output file
                        l_tmp.push_back(layer);
                        c_tmp.push_back(cell);
                        std::vector<double> dtTmpAll;
                        for(int w=0;w<fDriftTimeAll_in->at(iHit).size();w++){
                            dtTmpAll.push_back(fDriftTimeAll_in->at(iHit)[w]-chen_offset);
                        }
                        dt_all_tmp.push_back(dtTmpAll);
                        v_wf_tmp.push_back(fWaveform_in->at(iHit));
                        v_wf_clk_tmp.push_back(fWaveformClk_in->at(iHit));
                        dt_tmp.push_back(dt);
                        hitR_tmp.push_back(hitR); // Taken right hand side for now
                        q_tmp.push_back(q);
                        qcut_tmp.push_back(q_cut_tmp);
                        p_tmp.push_back(peak);
                        nlh_tmp.push_back(fNumOfLayHits_in->at(iHit));
                        np_tmp.push_back(fNumOfPeaks_in->at(iHit));
                        pw_tmp.push_back(fPeakWidth_in->at(iHit));
                        cp_tmp.push_back(fChoosePeakFlag_in->at(iHit));
                        fT0_out.push_back(fT0_in->at(iHit));
                        hit_flag_select_tmp.push_back(0);//Everything not selected first
                        if(fAnalyzeSimData){
                            mc_r_tmp.push_back(fMCDriftDistance_in->at(iHit));
                            mc_rs_tmp.push_back(fMCDriftDistanceSmear_in->at(iHit));
                        }
                        left_right_tmp.push_back(1);
                        avg_q+=q;
                        nHits++;
                        fQEvent_out+=q;
                        if(layer!=fTestLayer)  nHitsNoTestLay++;
                        if(fNumOfLayHits_in->at(iHit)==1)  nHitsWith1LayHit++;
                        select = 1;
                    }
                    if(fDEBUG){
                        printf("## Raw: %5d Lay: %5d Cell: %5d dt: %8.3f hitR(p): %8.3f q: %8.1f q_cut: %8.1f ",iHit,layer,cell,dt,hitR,q,q_cut_tmp);
                        if(select==1){
                            printf("   -----   selected \n");
                        }else{
                            printf("\n");
                        }
                    }
                }
                // nHits cut to remove unwanted events for spring8 only..
                if(nHitsNoTestLay<6){
                    Clear();
                    continue;
                }
                /// Setup hit container
                fRawHitCon->SetQEvent(fQEvent_out);
                fRawHitCon->SetEventID(iev,fTriggerNumber_out);
                fRawHitCon->SetNumOfHits(nHits);
                fRawHitCon->SetHitR(hitR_tmp);
                fRawHitCon->SetDriftTime(dt_tmp);
                fRawHitCon->SetDriftTimeAll(dt_all_tmp);
                fRawHitCon->SetADCPeak(p_tmp);
                fRawHitCon->SetQ(q_tmp);
                fRawHitCon->SetQCut(qcut_tmp);
                fRawHitCon->SetNumLayerHits(nlh_tmp);
                fRawHitCon->SetNumOfPeaks(np_tmp);
                fRawHitCon->SetWidthOfPeaks(pw_tmp);
                fRawHitCon->SetChoosePeakFlag(cp_tmp);
                fRawHitCon->SetHitFlag(hit_flag_select_tmp);
                fRawHitCon->SetHitWire(l_tmp,c_tmp);
                fRawHitCon->SetLeftRight(left_right_tmp);
                fRawHitCon->SetTestLayer(fTestLayer);
                fRawHitCon->SetMaxADCSUM(max_q);
                fRawHitCon->SetMaxADCPeak(max_peak);
                fRawHitCon->SetWaveform(v_wf_tmp);
                fRawHitCon->SetWaveformClk(v_wf_clk_tmp);

                //fRawHitCon->Show();
                // Do Track finding
                // Testing previous code
                ITrackFinderLinear *trackFinder = new ITrackFinderLinear(fRawHitCon,fDEBUG);
                trackFinder->Process();
                fTrackCon = trackFinder->GetTracks();
                if(fDEBUG){
                    printf("-------------------------------------\n");
                    printf(" After ITrackFinderLinear %d tracks\n",fTrackCon->GetNTracks());
                    fTrackCon->Show();
                }
                if(!fTrackCon || !fTrackCon->hasGoodTracks()){
                    Clear();
                    if(!fTrackCon->hasGoodTracks())
                        std::cout << "Finding results are really bad" << std::endl;
                    std::cout << "no tracks find " << std::endl;
                    continue;
                }
                // Do track fitting
                ITrackFitterLinear *trackFitter = new ITrackFitterLinear(fRawHitCon,fTrackCon,fDEBUG);
                // Write tracks to output file
                trackFitter->Process(0);
                fTrackCon = trackFitter->GetTracks();
                if(fDEBUG){
                    printf("-------------------------------------\n");
                    printf("After ITrackFitterLinear %d tracks\n",fTrackCon->GetNTracks());
                    fTrackCon->Show();
                }
                if(fDEBUG){
                    fEventDisplay->SetCanvasSize(512,720);
                    fEventDisplay->SetIHit(fRawHitCon, fTrackCon);
                    fEventDisplay->DrawEvent(Form("test"),"spza");
                }

                // Refine final using all peaks
                // ITrackFitterMultPeakLinear *trackFitterMultPeak = new ITrackFitterMultPeakLinear(fRawHitCon,fTrackCon,fDEBUG);
                FillOutputTree();
                Clear();
                delete trackFinder;
                delete trackFitter;
                //delete trackFitterMultPeak;
            }
            diff = clock() - start;
            int msec = diff * 1000 / CLOCKS_PER_SEC;
            printf("## Time taken %d seconds %d milliseconds\n", msec/1000, msec%1000);
        }
        void End(){
            fTree_out->Write();
            fFile_out->Close();
            fFile_in->Close();
        }
    private:
};

int usage(char* prog_name)
{
    fprintf(stderr,"=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@\n\n");
    fprintf(stderr,"[How is works]\n");
    fprintf(stderr,"=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@==@=@=@=@=@=@@=@\n");

    fprintf(stderr,"Usage %s [options (args)]\n",prog_name);
    fprintf(stderr,"[options]\n");
    fprintf(stderr,"\t -P\n");
    fprintf(stderr,"\t\t Add prefix to the output files \n");
    fprintf(stderr,"\t -r\n");
    fprintf(stderr,"\t\t force a run number\n");
    fprintf(stderr,"\t -I\n");
    fprintf(stderr,"\t\t set a iteration step\n");
    fprintf(stderr,"\t -f\n");
    fprintf(stderr,"\t\t from event id\n");
    fprintf(stderr,"\t -t\n");
    fprintf(stderr,"\t\t to event id\n");
    fprintf(stderr,"\t -F\n");
    fprintf(stderr,"\t\t set file ID \n");
    fprintf(stderr,"\t -T\n");
    fprintf(stderr,"\t\t set test layer \n");
    fprintf(stderr,"\t -N\n");
    fprintf(stderr,"\t\t force maximum number of events (all)]\n");
    fprintf(stderr,"\t -k\n");
    fprintf(stderr,"\t\t force to use constant velocity\n");
    fprintf(stderr,"\t -i\n");
    fprintf(stderr,"\t\t set an input file\n");
    fprintf(stderr,"\t -p\n");
    fprintf(stderr,"\t\t set event level information print modulo [1000 as default]\n");
    fprintf(stderr,"\t -d\n");
    fprintf(stderr,"\t\t set draw event id \n");
    fprintf(stderr,"\t -x\n");
    fprintf(stderr,"\t\t set input path of xt relation\n");
    fprintf(stderr,"\t -s\n");
    fprintf(stderr,"\t\t set input path of resolution relation\n");
    fprintf(stderr,"\t -h\n");
    fprintf(stderr,"\t\t help message \n\n");
    return 0;
}

int main(int argc, char **argv){
    //  int runNo,startIEV,endIEV,iFile,numOfFiles,iter ;
    int runNo,iter=0;
    int numOfEvents = -1;
    int testLayer=-1;
    int useSim=0;
    int fromIEV=-1;
    int toIEV=-1;
    Long64_t printModulo = 1000;
    int fileID=0;
    std::string fConfigure = "experiemnt-config.txt";
    std::string fExperiment = "crtsetup2";
    std::string inputPath = "";
    std::string xtInputPath = "";
    std::string resInputPath = "";
    std::string prefix = "";
    int optquery;
    while((optquery=getopt(argc,argv,"P:e:c:n:r:i:f:t:I:N:p:T:F:x:s:S:h"))!=-1){
        switch(optquery){
        case 'P':
            prefix = optarg;
            printf("Set prefix to : %s\n",prefix.c_str());
            break;
        case 'e':
            fExperiment = optarg;
            std::transform(fExperiment.begin(),fExperiment.end(),fExperiment.begin(),::tolower);
            printf("Set experiment to : %s\n",fExperiment.c_str());
            break;
        case 'c':
            fConfigure = optarg;
            printf("Set configure file to : %s\n",fConfigure.c_str());
            break;
        case 'r':
            runNo = atoi(optarg);
            printf("Set run number to : %d\n",runNo);
            break;
        case 'i':
            inputPath = optarg;
            printf("Set input path to : %s\n",inputPath.c_str());
            break;
        case 'f':
            fromIEV = atoi(optarg);
            printf("Set from event id to : %d\n",fromIEV);
            break;
        case 't':
            toIEV = atoi(optarg);
            printf("Set to event id to : %d\n",toIEV);
            break;
        case 'I':
            iter = atol(optarg);
            printf("Set iteration step to : %d\n",iter);
            break;
        case 'N':
            numOfEvents = atol(optarg);
            printf("Set number of events to : %d\n",numOfEvents);
            break;
        case 'p':
            printModulo = atol(optarg);
            printf("Set event print modulo to : %lld\n",printModulo);
            break;
        case 'F':
            fileID = atol(optarg);
            printf("Set test layer to : %d\n",fileID);
            break;
        case 'T':
            testLayer = atol(optarg);
            printf("Set test layer to : %d\n",testLayer);
            break;
        case 'x':
            xtInputPath = optarg;
            printf("Set xt input path to : %s\n", xtInputPath.c_str());
            break;
        case 's':
            resInputPath = optarg;
            printf("Set res input path to : %s\n", resInputPath.c_str());
            break;
        case 'S':
            useSim = atoi(optarg);
            printf("Set useSim to : %d\n", useSim);
            break;
        case 'h':
        default:
            usage(argv[0]);
            return 1;
        }
    }

    if(optind<2){
        usage(argv[0]);
        return 1;
    }
    // Create event display
    IRunTimeManager::Get().Initialize(runNo, testLayer, iter, fConfigure, fExperiment, xtInputPath, resInputPath);
    // Start reading the analysis code
    MyAnalysisCode    *userCode  = new MyAnalysisCode(fileID,testLayer,inputPath,"t",prefix);
    if(!(useSim==0))
        userCode->AnalyzeSimData(true);

    userCode->Run1EventOnly(fromIEV,toIEV);
    userCode->Process();
}
