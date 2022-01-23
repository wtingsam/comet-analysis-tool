#include "TGraphErrors.h"
#include "TF1.h"
#include <iomanip>
#include "TTree.h"
#include "TLegend.h"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <unistd.h> //get opt

#include "TFile.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2D.h"
#include "TMath.h"
#include <time.h>
#include "TStyle.h"
#include "TLine.h"
#include "TMinuit.h"
#include "TNamed.h"

#include "IXTFitter.hxx"
#include "IRunTimeManager.hxx"
#include "IOStreamFunction.hxx"

#define NPHIBIN 2 // Note that phi bin can only be even number
#define SLICE_TIME_BIN  500
#define SLICE_DIST_BIN  640
#define SLICE_BIN  500
#define RANGE_TIME  600
#define RANGE_DIST  12
#define RES_CUT 0.5

#define CELL_BOUND_CUT 7.9
#define CELL_BOUND_R_LOW 7.80
#define CELL_BOUND_R_UP 7.90
#define CELL_BOUND_L_LOW 7.80
#define CELL_BOUND_L_UP 7.90
#define CELL_DT_CUT 360
#define MAX_RES_2MM 20

#define CELL_TURN_PT0 -25
#define CELL_TURN_PT1 20
#define CELL_TURN_PT2 350
#define CELL_TURN_PT3 600
#define MOVE_XT_CUT 160
using namespace std ;
/***
    This program fit the x-t relation by projecting
    the x-t distribution on x.
    The x-t relation will be fitted according to each layer,
    if the events is smaller than 1000,
    if failed change the cut condition
    if failed , read all root-track files
    and then make 1 x-t distribution and then fit to get.
    for iteration < 3,
    I use only 1 layer for all layers after that,
    I open for all layers
***/
#define ITER_READ_LAYER_10 3

class MyAnalysisCode{
    private:
        std::vector<int>        *fDOF_in;
        std::vector<double>        *fChi2_in;
        std::vector<double>        *fChi2p_in;
        std::vector<int>        *fLayerID_in;
        std::vector<int>        *fCellID_in;
        std::vector<int>        *fClosestCell_in;
        std::vector<int>        *fTrackIndex_in;
        std::vector<double>     *fDriftTimeBest_in;
        std::vector<double>     *fFitR_in;
        std::vector<double>     *fHitRBest_in;
        Int_t fTrkIdMinResTestLay_in;

        Double_t fCellEdge_T[2];
        Int_t fCount_BoundCell[2];

        Double_t fCellEdge_T_Avg[2]; //left right
        Int_t fCount_BoundCell_Avg[2];
        Double_t fTurningPt_T_Avg[4]; // 3  turning points

        TFile *fFile;
        TString fInputPath;
        TTree *fTree;

        char *fGraphDir;

        /***
            Special treatment for DCA < 2 mm
        ***/
        TF1 *fRCalibFun[2];
        double fResCalib_x[MAX_RES_2MM][2]; // Spatial resolution
        double fResCalib_y[MAX_RES_2MM][2];
	TH1D *hTimeEdgeHisto; 
        TH2D *hDistTimeHisto; // All;
        TH2D *hTimeDriftDistHisto[3]; // 0:LEFT 1:RIGHT 2:ALL
        TH2D *hTimeDriftDistEdge[3];  // 0:Close origin 1: Edge L Edge R
        TH1D *hLeftRightCalibration[2];
        TH2D *fResVsDCA;
        TH1D *fSPProjOnTHisto[1000][2];
        TH1D *fSPProjOnXTHisto[1000][2];

        TH1D *fHistoRes;
        TH1D *hT0Residue;
        std::vector<TGraphErrors *>fMeanXtGraphT; // Left Right
        std::vector<TGraphErrors *>fMeanXtGraphD; // Left Right
        TGraphErrors *fTempXtGraph;
        TLine *fLineBoundCell[2];

        TCanvas *fCanvas;

	TF1 *fXtFunction[3][2]; // edge LeftRight, 
        TF1 *fXtFunctionCombined[2];//left right
        TGraph *gSymmetryTestXtPt;
        int fRunNo;
        int fIter;
        int fTestLayer;
        Int_t fSizePos[2];
        int fSign[4];
        int fEdge[4];
        int MAX_SENSE_LAYER;
        int fNumOfFittingPar;

        void Initialize(int runNo, int iter){
            fRunNo=runNo;
            fIter=iter;
            fCellEdge_T_Avg[0]=0;
            fCellEdge_T_Avg[1]=0;
	    
            MAX_SENSE_LAYER  = ExperimentConfig::Get().GetNumOfLayer();

            fGraphDir=getenv("CCGRAPH_DIR");
            fCanvas= new TCanvas("tmp","tmp",2048,1700);

            for(int ii=0;ii<2;ii++)
                for(int i=0;i<3;i++)
                    fXtFunction[i][ii]=NULL;

            hLeftRightCalibration[0] = new TH1D("hLeftRightCalibration_0","hLeftRightCalibration_0",100,-10,0);
            hLeftRightCalibration[1] = new TH1D("hLeftRightCalibration_1","hLeftRightCalibration_1",100,0,10);

            fResVsDCA=new TH2D("fResVsDCA","Residue vs DCA",100,-2,2,50,-10,10);
	    hTimeEdgeHisto = new TH1D("hTimeEdgeHisto",";driftTime[ns];",96,299.5,400.5);
            hDistTimeHisto = new TH2D("hDistTimeHisto",";distance of closest approch [mm];driftTime[ns]",512,-10,10,600,-24.5,600.5);
            for(int ii=0;ii<3;ii++){
                hTimeDriftDistHisto[ii]=new TH2D(Form("h0_%d",ii),"Fitting of x-t relation on x-t distribution",600,-24.5,600.5,512,-10,10);
                hTimeDriftDistHisto[ii]->GetXaxis()->SetTitle("Drift Time [ns]");
                hTimeDriftDistHisto[ii]->GetYaxis()->SetTitle("DCA [mm]");
            }
            hTimeDriftDistEdge[0]=new TH2D("hedge0","Edge x-t distribution",100,-24.5,100.5,128,-3,3);
            hTimeDriftDistEdge[1]=new TH2D("hedge1","Edge x-t distribution",175,150.5,450.5,512,6,8.5);
            hTimeDriftDistEdge[2]=new TH2D("hedge1","Edge x-t distribution",175,150.5,450.5,512,-8.5,-6);

            for(int j=0;j<SLICE_TIME_BIN;j++){
                fSPProjOnTHisto[j][0] = new TH1D(Form("hxt_%d_0",j),
						 Form("Left ( %8.3f ~ %8.3f ]ns;DCA[mm]",
						      j*RANGE_TIME*1.0/SLICE_TIME_BIN,
						      (j+1)*RANGE_TIME*1.0/SLICE_TIME_BIN),
						 200,-RANGE_DIST,RANGE_DIST);
                fSPProjOnTHisto[j][1] = new TH1D(Form("hxt_%d_1",j),
						 Form("Right ( %8.3f ~ %8.3f ]ns;DCA[mm]",
						      j*RANGE_TIME*1.0/SLICE_TIME_BIN,
						      (j+1)*RANGE_TIME*1.0/SLICE_TIME_BIN),
						 200,-RANGE_DIST,RANGE_DIST);
            }


            for(int j=0;j<SLICE_DIST_BIN;j++){
                fSPProjOnXTHisto[j][0] = new TH1D(Form("htx_%d_0",j),
						  Form("( %8.3f ~ %8.3f ]mm;driftTime[ns]",
						       j*RANGE_DIST*1.0/SLICE_DIST_BIN,
						       (j+1)*RANGE_DIST*1.0/SLICE_DIST_BIN),
						  200,-10,RANGE_TIME);
                fSPProjOnXTHisto[j][1] = new TH1D(Form("htx_%d_1",j),
						  Form("( %8.3f ~ %8.3f ]mm;driftTime[ns]",
						       -(j+1)*RANGE_DIST*1.0/SLICE_DIST_BIN,
						       -(j)*RANGE_DIST*1.0/SLICE_DIST_BIN),
						  200,-10,RANGE_TIME);
            }

            fHistoRes = new TH1D("HistoRes","HistoRes",100,-5,5);
        }
    public:

        MyAnalysisCode(int runNo, int iter, TString inputPath);

        virtual ~MyAnalysisCode(){
        }

        void FileLoop(Int_t lowerLay, Int_t upperLay){
	    // For now!!
	    if(lowerLay!=upperLay)return ;
	    fTestLayer=lowerLay;
            char *rootDIR = getenv("CCTRACKROOT_DIR");
            for(int iLay=lowerLay;iLay<upperLay+1;iLay++){// Take the average of the middle layers
                if(fInputPath!=""){
                    std::cout << " Look at path: " << fInputPath.Data() << std::endl;
                    fFile = new TFile(fInputPath);
                }else{
                    fFile = new TFile(Form("%s/xt_%d_i%d_l%d.root",rootDIR,fRunNo,fIter-1,iLay));
                }
                if(!fFile->IsOpen()){
                    std::cerr << "## ERROR Do not have root file" << std::endl;
                    continue ;
                }
                fTree = (TTree*)fFile->Get("t");
                fTree->SetBranchAddress("DOF", &fDOF_in);
                fTree->SetBranchAddress("chi2", &fChi2_in);
                fTree->SetBranchAddress("chi2p", &fChi2p_in);
                fTree->SetBranchAddress("layerID", &fLayerID_in);
                fTree->SetBranchAddress("cellID", &fCellID_in);
                fTree->SetBranchAddress("closestCell", &fClosestCell_in);
                fTree->SetBranchAddress("trackIndex", &fTrackIndex_in);
                fTree->SetBranchAddress("driftTime",&fDriftTimeBest_in);
                fTree->SetBranchAddress("fitR",&fFitR_in);
                fTree->SetBranchAddress("hitRBest",&fHitRBest_in);
                fTree->SetBranchAddress("trkIdMinResTestLayt",&fTrkIdMinResTestLay_in);

                // First make sure the x-t distribtion is in the middle
                // Assumption is that the middle part of the x-t relation is symmetry
                double avg_dv=0;
                double avg_dv_count=0;
                for(int iev=0;iev<fTree->GetEntries();iev++){
                    fTree->GetEntry(iev);
                    const int nsize = (int)fLayerID_in->size();
                    for(int i=0;i<nsize;i++){//hit loop
                        // Cut out all tracks not best
                        //if(fChi2p_in->at(i)<=0.05 || fChi2_in->at(i)/fDOF_in->at(i)>=10 || fDOF_in->at(i)!=3) continue;
                        if(fChi2p_in->at(i)<=0.05 || fChi2_in->at(i)/fDOF_in->at(i)>=10) continue;
                        if(fTrackIndex_in->at(i)!=fTrkIdMinResTestLay_in) continue;
                        int lid = fLayerID_in->at(i);
                        int cid = fCellID_in->at(i);
                        int ccid = fClosestCell_in->at(i);
                        // Get information and fill left right historgarm
                        double dt=fDriftTimeBest_in->at(i);
                        double hr=fHitRBest_in->at(i);
                        double fr=fFitR_in->at(i);
                        double sp=fabs(hr)-fabs(fr);
                        if(lid==iLay && ccid==cid){
                            if(dt>MOVE_XT_CUT-1 && dt<MOVE_XT_CUT+1 && fr<0  && fabs(sp)<0.5){
                                hLeftRightCalibration[0]->Fill(fr);
                            }
                            if(dt>MOVE_XT_CUT-1 && dt<MOVE_XT_CUT+1 && fr>=0 && fabs(sp)<0.5){
                                hLeftRightCalibration[1]->Fill(fr);
                            }
                        }
                    }
                }

                // Get mean
                const int MOVE_TOLERANCE = 1; //mm
                double move_fr=0;
                if(hLeftRightCalibration[0]->Integral()>1){
                    double left =hLeftRightCalibration[0]->GetMean();
                    double right=hLeftRightCalibration[1]->GetMean();
                    move_fr = (left+right)/2;
                    if(abs(move_fr)>MOVE_TOLERANCE)
                        move_fr=0;
                    printf("## Left mean %f Right mean %f so we move %f \n",left,right,move_fr);
                }

                // Event loop for filling in x-t
                for(int iev=0;iev<fTree->GetEntries();iev++){
                    fTree->GetEntry(iev);
                    const int nsize = (int)fLayerID_in->size();
                    for(int i=0;i<nsize;i++){//hit loop
                        // Cut out all tracks not best
                        //chi2p>0.05 && chi2/DOF <2
//                        if(fChi2p_in->at(i)<=0.05 || fChi2_in->at(i)/fDOF_in->at(i)>=10 || fDOF_in->at(i)!=3) continue;
                        if(fChi2p_in->at(i)<=0.05 || fChi2_in->at(i)/fDOF_in->at(i)>=10) continue;
                        if(fTrackIndex_in->at(i)!=fTrkIdMinResTestLay_in) continue;
                        int lid = fLayerID_in->at(i);
                        int cid = fCellID_in->at(i);
                        int ccid = fClosestCell_in->at(i);
                        if(lid==iLay && ccid==cid){
                            // Fill t-x distribtion
                            double dt=fDriftTimeBest_in->at(i);
                            double hr=fHitRBest_in->at(i);
                            double fr=fFitR_in->at(i) - move_fr;
                            double sp=fabs(hr)-fabs(fr);
                            // Fill Res vs DCA
                            fResVsDCA->Fill(sp,fr);

                            // XT
                            if(cid==ccid){
                                hDistTimeHisto->Fill(fr,dt);
				hTimeEdgeHisto->Fill(dt);
			    }
                            // Fill TX
                            if(fabs(sp)<RES_CUT){
                                hTimeDriftDistEdge[0]->Fill(dt,fr);
                                hTimeDriftDistEdge[1]->Fill(dt,fr);
                                hTimeDriftDistEdge[2]->Fill(dt,fr);
                                hTimeDriftDistHisto[2]->Fill(dt,fr);
                                if(fr<0){
                                    hTimeDriftDistHisto[0]->Fill(dt,fr);
                                }else{
                                    hTimeDriftDistHisto[1]->Fill(dt,fr);
                                }
                            }
                            // Fill slicespoint time projection
                            for(int iSlice=0;iSlice<SLICE_TIME_BIN;iSlice++){
                                if(dt>=iSlice*1.*RANGE_TIME/SLICE_TIME_BIN && dt<(iSlice+1)*1.*RANGE_TIME/SLICE_TIME_BIN){
                                    if(fr>0){
                                        if(fabs(sp)<RES_CUT){
                                            fSPProjOnTHisto[iSlice][1]->Fill(fr);
					}
                                    }else{
                                        if(fabs(sp)<RES_CUT)
                                            fSPProjOnTHisto[iSlice][0]->Fill(fr);
                                    }
                                }
                            }
                            // Fill slicespoint distance  projection
                            for(int iSlice=0;iSlice<SLICE_DIST_BIN;iSlice++){
                                if(fabs(fr)>=iSlice*1.*RANGE_DIST/SLICE_DIST_BIN && fabs(fr)<(iSlice+1)*1.*RANGE_DIST/SLICE_DIST_BIN){
                                    if(fr>0){
                                        if(fabs(sp)<RES_CUT)
					    fSPProjOnXTHisto[iSlice][1]->Fill(dt);
                                    }else{
                                        if(fabs(sp)<RES_CUT)
					    fSPProjOnXTHisto[iSlice][0]->Fill(dt);
                                    }
                                }
                            }
                        }
                    }
                }
                //GetCellEdge(iLay);
            }

            // for(int ii=0;ii<2;ii++)
            //     fCellEdge_T_Avg[ii]/=(upperLay-lowerLay+1);
            // printf("## Final average : Left: %f Right: %f \n",fCellEdge_T_Avg[0],fCellEdge_T_Avg[1]);
        }

        /// A function for getting the average cell edge
        // void GetCellEdge(int testLayer){
        //     fCount_BoundCell[0]=0;
        //     fCount_BoundCell[1]=0;
        //     fCellEdge_T[0]=0;
        //     fCellEdge_T[1]=0;
        //     for(int iev=0;iev<fTree->GetEntries();iev++){
        //         fTree->GetEntry(iev);
        //         const int nsize = (int)fLayerID_in->size();
        //         for(int i=0;i<nsize;i++){//hit loop
        //             // Cut out all tracks not best
        //             if(fChi2p_in->at(i)<=0.05 || fChi2_in->at(i)/fDOF_in->at(i)>=10 || fDOF_in->at(i)!=3) continue;
        //             if(fTrackIndex_in->at(i)!=fTrkIdMinResTestLay_in) continue;
        //             int lid = fLayerID_in->at(i);
        //             int cid = fCellID_in->at(i);
        //             int ccid = fClosestCell_in->at(i);
        //             double dt=fDriftTimeBest_in->at(i);
        //             double hr=fHitRBest_in->at(i);
        //             double fr=fFitR_in->at(i);
        //             double sp=fabs(hr)-fabs(fr);
        //             if(lid==testLayer && ccid==cid){
        //                 // Find boundary
        //                 if(fr<0){
        //                     if(fabs(fr)>CELL_BOUND_L_LOW && fabs(fr)<CELL_BOUND_L_UP){
        //                         fCellEdge_T[0]+=dt;
        //                         fCount_BoundCell[0]++;
        //                     }
        //                 }else{
        //                     if(fabs(fr)>CELL_BOUND_R_LOW && fabs(fr)<CELL_BOUND_R_UP){
        //                         fCellEdge_T[1]+=dt;
        //                         fCount_BoundCell[1]++;
        //                     }
        //                 }
        //             }
        //         }
        //     }
        //     printf("## The boundary of cell between L (%.3f,%.3f) R(%.3f,%.3f) has a average value of Left: %f/%d = %f Right: %f/%d = %f \n",
        //            CELL_BOUND_L_LOW,CELL_BOUND_L_UP,
        //            CELL_BOUND_R_LOW,CELL_BOUND_R_UP,
        //            fCellEdge_T[0],fCount_BoundCell[0],fCellEdge_T[0]/fCount_BoundCell[0],
        //            fCellEdge_T[1],fCount_BoundCell[1],fCellEdge_T[1]/fCount_BoundCell[1]
        //            );
        //     fCellEdge_T[0]/=fCount_BoundCell[0];
        //     fCellEdge_T_Avg[0]+=fCellEdge_T[0];
        //     fCellEdge_T[1]/=fCount_BoundCell[1];
        //     fCellEdge_T_Avg[1]+=fCellEdge_T[1];
        //     printf("## ----    Sum Left : %f Right :%f \n", fCellEdge_T_Avg[0],fCellEdge_T_Avg[1]);
        // }

        TGraphErrors* FillGraphSample(TString name, TString title,
                                      vector<double> x,vector<double> x_err,
                                      vector<double> t,vector<double> t_err, int color, int linecolor){
            const int size = x.size();
            TGraphErrors * gTmp = new TGraphErrors(size);
            gTmp -> SetName(name);
            gTmp -> SetTitle(title);

            for(int i=0;i<size;i++){
                gTmp->SetPoint(i,x[i],t[i]);
            }
            gTmp->SetMarkerStyle(8);
            gTmp->SetMarkerSize(0.5);
            gTmp->SetMarkerColor(color);
            gTmp->SetLineColor(color);
            gTmp->SetLineWidth(2);

            return gTmp;
        }

        bool GetMeanXtGraph(TString option){
            vector<double> fr_pos[2] ;
            vector<double> dt_pos[2] ;
            vector<double> fr_pos_err[2] ;
            vector<double> dt_pos_err[2] ;
            TH1D *hTmp;
            int sign[2] = {-1,1};
            int color;
            int linecolor;
            double rms_cut;
            int nBin;
            if(option.Contains("dist")){
                nBin = SLICE_DIST_BIN;
                rms_cut = 20;
                color = 1;
                linecolor = 1;
            }else{
                nBin = SLICE_TIME_BIN;
                rms_cut = 0.5;
                color = 3;
                linecolor = 3;
            }

            for(int ii=0;ii<2;ii++){ // Loop over left and right
                for(int iSlice=0;iSlice<nBin;iSlice++){ // Loop over all slices
                    //LEFT
                    double fr = iSlice*1.*RANGE_DIST/nBin * sign[ii];
                    double fr_err = 0;
                    double dt = iSlice*1.*RANGE_TIME/nBin;
                    double dt_err = 0;
                    double range;
                    // Choose sampling
                    if(option.Contains("dist")){
                        range = RANGE_DIST*2;
                        hTmp = fSPProjOnXTHisto[iSlice][ii];
                    }else{
                        range = RANGE_TIME;
                        hTmp = fSPProjOnTHisto[iSlice][ii];
                    }

                    if(hTmp->Integral()>3){
                        int binmax = hTmp->GetMaximumBin();
                        double x = hTmp->GetXaxis()->GetBinCenter(binmax);
                        double par = 100; if(option.Contains("time")) par = 3;
                        double xmin = x - par;
                        double xmax = x + par;
                        //printf("mean %f binmax %d xmax %f range(%f %f)\n",hTmp->GetMean(),binmax,x,xmin,xmax);
                        hTmp->Fit("gaus","Q","",xmin,xmax);
                        //hTmp->Fit("gaus","Q");
                        if(option.Contains("dist")){
                            dt    = hTmp -> GetMean();
                            //dt      = hTmp -> GetFunction("gaus") -> GetParameter(1);
                            dt_err  = hTmp -> GetMeanError();
                        }else{
                            //fr    = hTmp -> GetMean();
                            fr      = hTmp -> GetFunction("gaus") -> GetParameter(1);
                            if(fabs(fr)<0.2) fr = x;
			    if(dt>CELL_TURN_PT2-10 && dt<CELL_TURN_PT3){
				if(fr>0)
				    fr+=0.1;
				else
				    fr-=0.1;
			    }

                            fr_err  = hTmp -> GetMeanError();
                        }
                        //double    rms     = hTmp -> GetRMS();
                        double      rms     = hTmp -> GetFunction("gaus") -> GetParameter(2);
                        double      rms_err = hTmp -> GetRMSError();
                        int         entry   = hTmp -> GetEntries();
                        //printf("%f %f %f %f rms %f \n",fr,fr_err,dt,dt_err,rms);
                        if(rms<rms_cut){ // Get sigma and must smaller than 0.35
                            fr_pos[ii].push_back(fr);
                            fr_pos_err[ii].push_back(fr_err);
                            dt_pos[ii].push_back(dt);
                            dt_pos_err[ii].push_back(dt_err);
                        }
                    }
                    // fCanvas->Clear();
		    //fCanvas->cd();
		    //hTmp->Draw();
                    //if(option.Contains("time") && iSlice<100)fCanvas->SaveAs("tmp/" + option + Form(".%d_%03d.png",ii,iSlice));
                }
                // Check size
                if(fr_pos[ii].size()==0 || dt_pos[ii].size()==0){
                    std::cerr << "@@@@ Error there is no hits , please check "
                              << ii << " size of fr : "
                              << fr_pos[ii].size()
                              << " size of dt :"
                              << dt_pos[ii].size()
                              << std::endl;
                    return false;
                }
                // Write the mean point into files
                char *outputXtDir=getenv("CCPARAMETER_DIR");
                if(option.Contains("time"))
                    fMeanXtGraphT.push_back(FillGraphSample(Form("xtt_%d",ii),Form("xtt_%d",ii),
                                                            dt_pos[ii], dt_pos_err[ii],
                                                            fr_pos[ii], fr_pos_err[ii], color, linecolor));
                else if(option.Contains("dist"))
                    fMeanXtGraphD.push_back(FillGraphSample(Form("xtd_%d",ii),Form("xtd_%d",ii),
                                                            dt_pos[ii], dt_pos_err[ii],
                                                            fr_pos[ii], fr_pos_err[ii], color, linecolor));

            }
            return true;
        }

        void FitXtHistrogram(void){
            // tEdge should be EdgeT , which is boundary of the cell
	    fTurningPt_T_Avg[0]=CELL_TURN_PT0 ;
	    fTurningPt_T_Avg[1]=CELL_TURN_PT1 ;
	    fTurningPt_T_Avg[2]=CELL_TURN_PT2 ;
	    fTurningPt_T_Avg[3]=CELL_TURN_PT3 ;
	    TString form_lists[3]={
		"pol9",
		"pol7",
		"pol5"
	    };
	    for(int it=0;it<3;it++){
		for(int ii=0;ii<2;ii++){
		    fXtFunction[it][ii] = new TF1(Form("avg_xt_it%d_phi%d",it,ii),form_lists[it], fTurningPt_T_Avg[it],fTurningPt_T_Avg[it+1]);
		    fXtFunction[it][ii]->SetLineWidth(2);
		    fXtFunction[it][ii]->SetLineColor(2);
		    if(hTimeDriftDistHisto[ii]->Integral()>100){
			if(it>0)
			    fMeanXtGraphT[ii]->Fit(fXtFunction[it][ii],"RBEMN","",fTurningPt_T_Avg[it],fTurningPt_T_Avg[it+1]);
			else
			    fMeanXtGraphD[ii]->Fit(fXtFunction[it][ii],"RBEMN","",fTurningPt_T_Avg[it],fTurningPt_T_Avg[it+1]);
		    }
		}
	    }
            // Use fitter class to combine two xts
            printf("###################################### \n");
            printf("###################################### \n\n Combining two functions \n");
            for(int ii=0;ii<2;ii++){
                //IXTFitter *xtFitter = new IXTFitter(fXtFunction[0][ii],fXtFunction[1][ii]);
                IXTFitter *xtFitter = new IXTFitter();
                xtFitter->SetName(Form("avg_xt_%d",ii));
                fXtFunctionCombined[ii]=xtFitter->Combined3TF1(fXtFunction[0][ii],
							       fXtFunction[1][ii],
							       fXtFunction[2][ii],
							       fTurningPt_T_Avg,
                                                               -25,600);
                fMeanXtGraphT[ii]->Fit(fXtFunctionCombined[ii]);
                fXtFunctionCombined[ii]->SetLineColor(2);
                fXtFunctionCombined[ii]->SetLineWidth(2);
                fCellEdge_T_Avg[ii] = fXtFunctionCombined[ii]->GetParameter(fXtFunctionCombined[ii]->GetNpar()-1);
                delete xtFitter;
            }
	    
            fXtFunctionCombined[0]->Print();
            fXtFunctionCombined[1]->Print();
            // Symmetry x-t
            printf("#################################\n\n Symmetry xt \n");
            const int np = 1000;
            gSymmetryTestXtPt = new TGraph(np);
            gSymmetryTestXtPt -> SetLineWidth(2);
            gSymmetryTestXtPt -> SetLineStyle(8);
            gSymmetryTestXtPt -> SetLineColor(15);
            for(int i=0;i<np;i++){
                double x = 1000*i/(np-1);
                double yp = fXtFunctionCombined[0]->Eval(x);
                double yn = fXtFunctionCombined[1]->Eval(x);
                gSymmetryTestXtPt->SetPoint(i,(double)x,yp+yn);
            }
        }

        void PrintResult(){
            fCanvas->Clear();
            fCanvas->Divide(2,2);
            fCanvas->cd(1)->SetGrid();
            fCanvas->cd(1)->SetLogz();
            gStyle->SetOptStat(0);
            hTimeDriftDistHisto[2]->Draw("colz");
            gSymmetryTestXtPt->Draw("samepl");
	    
	    for(int ii=0;ii<2;ii++){
                fLineBoundCell[ii] = new TLine(fCellEdge_T_Avg[ii],-10,fCellEdge_T_Avg[ii],10);
                fLineBoundCell[ii]->SetLineColor(ii+5);
                fLineBoundCell[ii]->Draw("same");
                fXtFunctionCombined[ii]->Draw("same");
            }
            fCanvas->cd(2);
            fCanvas->cd(2)->Divide(2,1);
            //fCanvas->cd(2)->cd(1)->SetLogz();
            fCanvas->cd(2)->cd(1)->SetGrid();
            hTimeDriftDistEdge[0]->Draw("colz");
            for(int ii=0;ii<2;ii++){
                fMeanXtGraphT[ii]->Draw("same p");
                //fMeanXtGraphD[ii]->Draw("same p");
                fXtFunctionCombined[ii]->Draw("same");
            }
            //fCanvas->cd(2)->cd(2)->SetLogz();
            fCanvas->cd(2)->cd(2)->Divide(1,2);
            fCanvas->cd(2)->cd(2)->cd(1)->SetGrid();
            hTimeDriftDistEdge[1]->Draw("colz");
            for(int ii=0;ii<2;ii++){
                fMeanXtGraphT[ii]->Draw("same p");
                //fMeanXtGraphD[ii]->Draw("same p");
                fXtFunctionCombined[ii]->Draw("same");
            }
            fCanvas->cd(2)->cd(2)->cd(2)->SetGrid();
            hTimeDriftDistEdge[2]->Draw("colz");
            for(int ii=0;ii<2;ii++){
                fMeanXtGraphT[ii]->Draw("same p");
                //fMeanXtGraphD[ii]->Draw("same p");
                fXtFunctionCombined[ii]->Draw("same");
            }
            fCanvas->cd(3)->SetGrid();
            fCanvas->cd(3)->SetLogz();
            gStyle->SetOptStat(0);
            fResVsDCA->Draw("colz");
            TLine *line = new TLine(0,-10,0,10);
            line->SetLineColor(1);
            line->SetLineWidth(2.0);
            line->Draw("same");
            fCanvas->cd(4)->SetGrid();
            fCanvas->cd(4)->SetLogz();
            hTimeEdgeHisto->Draw();
	    hTimeEdgeHisto->Fit("gaus");
            fCanvas->SaveAs(Form("%s/getAvgXt_summarize_r%d_i%d_l%d.png",fGraphDir,fRunNo,fIter,fTestLayer));
	}

        void Finalize(){
            fCanvas->Clear();
            fCanvas->cd()->SetGrid();;
            hTimeDriftDistHisto[2]->Draw("colz");
            for(int ii=0;ii<2;ii++){
                fLineBoundCell[ii] = new TLine(fCellEdge_T_Avg[ii],-10,fCellEdge_T_Avg[ii],10);
                fLineBoundCell[ii]->SetLineColor(ii+5);
                fLineBoundCell[ii]->Draw("same");
                fMeanXtGraphT[ii]->Draw("same p");
                fMeanXtGraphD[ii]->Draw("same p");
                fXtFunctionCombined[ii]->Draw("same");
                gSymmetryTestXtPt->Draw("samepl");
            }
            fCanvas->SaveAs(Form("%s/hTimeDriftDistHisto_r%d_i%d_l%d.png",fGraphDir,fRunNo,fIter,fTestLayer));

            fCanvas->Clear();
            hDistTimeHisto->Draw("colz");
            fCanvas->SaveAs(Form("%s/hDistTimeHisto_r%d_i%d_l%d.png",fGraphDir,fRunNo,fIter,fTestLayer));
            WriteXtFunction();
        }

        void WriteXtFunction(){
            char *outputXtDir=getenv("CCPARAMETER_DIR");
            TFile* f2 = new TFile(Form("%s/xtf_run%d_i%d_l%d.root",outputXtDir,fRunNo,fIter,fTestLayer),"UPDATE");
            for(int j=0;j<SLICE_TIME_BIN;j++)  fSPProjOnTHisto[j][0]->Write();
	    for(int j=0;j<SLICE_TIME_BIN;j++)  fSPProjOnTHisto[j][1]->Write();
            for(int j=0;j<SLICE_TIME_BIN;j++)  fSPProjOnXTHisto[j][0]->Write();
	    for(int j=0;j<SLICE_TIME_BIN;j++)  fSPProjOnXTHisto[j][1]->Write();

            for(int i=0;i<2;i++){
                if(!(TF1*)f2->Get(Form("avg_xt_%d",i))){
                    fXtFunctionCombined[i]->Write();
                    fXtFunction[0][i]->Write();
                    fXtFunction[1][i]->Write();
                    fMeanXtGraphT[i]->Write();
                    fMeanXtGraphD[i]->Write();
                }
            }
            f2->Close();
            printf("## The xt relation has been written into root file : \n## %s \n",Form("%s/xtf_run%d_i%d.root",outputXtDir,fRunNo,fIter) );
        }

};

MyAnalysisCode::MyAnalysisCode(int runNo, int iter,TString inputPath)
    :fDOF_in(0),
     fChi2_in(0),
     fChi2p_in(0),
     fLayerID_in(0),
     fCellID_in(0),
     fClosestCell_in(0),
     fTrackIndex_in(0),
     fDriftTimeBest_in(0),
     fFitR_in(0),
     fHitRBest_in(0),
     fFile(NULL),fInputPath(inputPath),
     fTree(NULL),
     fGraphDir(""),
     fCanvas(NULL)
{
    Initialize(runNo, iter);
}

int usage(char* prog_name)
{
    fprintf(stderr,"=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@\n\n");
    fprintf(stderr,"[How is works]\n");
    fprintf(stderr,"=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@==@=@=@=@=@=@@=@\n");

    fprintf(stderr,"Usage %s [options (args)]\n",prog_name);
    fprintf(stderr,"[options]\n");
    fprintf(stderr,"\t -r\n");
    fprintf(stderr,"\t\t force a run number\n");
    fprintf(stderr,"\t -I\n");
    fprintf(stderr,"\t\t set a iteration step\n");
    fprintf(stderr,"\t -f\n");
    fprintf(stderr,"\t\t set start from \n");
    fprintf(stderr,"\t -t\n");
    fprintf(stderr,"\t\t set layer to\n");
    fprintf(stderr,"\t -i\n");
    fprintf(stderr,"\t\t set an input file\n");
    fprintf(stderr,"\t -p\n");
    fprintf(stderr,"\t\t set event level information print modulo [1000 as default]\n");
    fprintf(stderr,"\t -C\n");
    fprintf(stderr,"\t\t set to copy the previous x-t relation for this iteration\n");
    fprintf(stderr,"\t -s\n");
    fprintf(stderr,"\t\t set a suffix\n");
    fprintf(stderr,"\t -h\n");
    fprintf(stderr,"\t\t help message \n\n");
    return 0;
}

int main(int argc, char*argv[])
{
    std::string configure = "experiemnt-config.txt";
    std::string experiment = "crtsetup2";
    std::string inputPath = "";
    std::string suffix = "";
    std::string xtInputPath = "";
    std::string resInputPath = "";
    Long64_t printModulo = 1000; // Print event level information every fPrintModulo events
    int         runNo,iter;
    int fromLay,toLay;
    int copyXt=0;

    int optquery;
    while((optquery=getopt(argc,argv,"he:c:r:i:I:p:f:t:C:d:x:s:S:"))!=-1){
        switch(optquery){
        case 'e':
            experiment = optarg;
            std::transform(experiment.begin(),experiment.end(),experiment.begin(),::tolower);
            printf("Set experiment to : %s\n",experiment.c_str());
            break;
        case 'c':
            configure = optarg;
            printf("Set configure file to : %s\n",configure.c_str());
            break;
        case 'r':
            runNo = atoi(optarg);
            printf("Set run number to : %d\n",runNo);
            break;
        case 'i':
            inputPath = optarg;
            printf("Set input path to : %s\n",inputPath.c_str());
            break;
        case 'I':
            iter = atol(optarg);
            printf("Set iteration step to : %ld\n",iter);
            break;
        case 'p':
            printModulo = atol(optarg);
            printf("Set event print modulo to : %ld\n",printModulo);
            break;
        case 'f':
            fromLay = atol(optarg);
            printf("Set start layer to : %ld\n",fromLay);
            break;
        case 't':
            toLay  = atol(optarg);
            printf("Set end layer to : %ld\n",toLay);
            break;
        case 'C':
            copyXt  = atol(optarg);
            printf("Set copy previous iteration's information to : %d\n",copyXt);
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
            suffix  = optarg;
            printf("Set suffix : %s\n",suffix.c_str());
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
    int dummy=-99;
    IRunTimeManager::Get().Initialize(runNo, -1, iter, configure, experiment, xtInputPath, resInputPath);
    IRunTimeManager::Get().AddSuffix(suffix);
    MyAnalysisCode userCode(runNo,iter,inputPath);
    userCode.FileLoop(fromLay,toLay);
    if(!userCode.GetMeanXtGraph("time") || !userCode.GetMeanXtGraph("dist") ){
        return -1;
    }
    userCode.FitXtHistrogram();
    userCode.PrintResult();
    userCode.Finalize();
}
