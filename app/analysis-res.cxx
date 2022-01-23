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

#include "TStyle.h"
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
#define SP_BIN 16
#define CELL_BOUND 8   //mm
#define MAX_DT 600 //ns
class MyAnalysisCode{
    private:
        std::vector<double> *fTrackPar_in;
        std::vector<int>        *fDOF_in;
        std::vector<double>     *fChi2_in;
        std::vector<double>     *fChi2p_in;
        std::vector<int>        *fLayerID_in;
        std::vector<int>        *fCellID_in;
        std::vector<int>        *fClosestCell_in;
        std::vector<int>        *fTrackIndex_in;
        std::vector<double>     *fDriftTimeBest_in;
        std::vector<double>     *fFitR_in;
        std::vector<double>     *fPhiAng_in;
        std::vector<double>     *fHitRBest_in;
        Int_t fTrkIdMinResTestLay_in;

        TFile *fFile;
        TString fInputPath;
        TTree *fTree;
        int fRunNo;
        int fIter;
        int fTestLayer;
        int MAX_SENSE_LAYER;

        // For output histogram and graph
        char *fGraphDir;
        TCanvas *fCanvas;
        TH1D *hHitRprev[20];
        TH1D *hHitRcurr[20];
        TH1D *hSPprev[20];
        TH1D *hSPcurr[20];
        TH2D *hSPvsDCAprev[20];
        TH2D *hSPvsDCAcurr[20];
        TH1D *hHitRcurrprevDiff[20]; // Current - previous
        TH2D *hHitRcurrprevDiff2D[20]; // Current - previous
	TGraphErrors* gSPvsDCAcurr[20];
	TGraphErrors* gSPvsDCAprev[20];
	TGraphErrors* gSPvsDCARcurr[20];
	TGraphErrors* gSPvsDCARprev[20];
	TGraphErrors* gSPvsDCALcurr[20];
	TGraphErrors* gSPvsDCALprev[20];
        std::vector<TH1D*> hDCAcurrSliceDCA[20];
        std::vector<TH1D*> hDCAprevSliceDCA[20];
        std::vector<TH1D*> hDCAcurrSliceDCAR[20];
        std::vector<TH1D*> hDCAprevSliceDCAR[20];
        std::vector<TH1D*> hDCAcurrSliceDCAL[20];
        std::vector<TH1D*> hDCAprevSliceDCAL[20];
        void Initialize(int runNo, int iter){
            fRunNo=runNo;
            fIter=iter;
            MAX_SENSE_LAYER  = ExperimentConfig::Get().GetNumOfLayer();
            fGraphDir=getenv("CCGRAPH_DIR");
            fCanvas= new TCanvas("tmp","tmp",2048,1700);
            for(int i=0;i<MAX_SENSE_LAYER;i++){
                hHitRprev[i]=new TH1D(Form("hHitRprev%d",i),
                                      Form("Run %d iter %d HitR (prev) for layer %d;HitR-DCA[mm];",
                                           fRunNo,fIter,i),128,-10,10);
                hHitRcurr[i]=new TH1D(Form("hHitRcurr%d",i),
                                      Form("Run %d iter %d HitR (curr) for layer %d;HitR-DCA[mm];",
                                           fRunNo,fIter,i),128,-10,10);
                hSPprev[i]=new TH1D(Form("hSPprev%d",i),
                                    Form("Run %d iter %d Spatial resolution (prev) for layer %d;HitR-DCA[mm];",
                                         fRunNo,fIter,i),128,-3,3);
                hSPcurr[i]=new TH1D(Form("hSPcurr%d",i),
                                    Form("Run %d iter %d Spatial resolution (curr) for layer %d;HitR-DCA[mm];",
                                         fRunNo,fIter,i),128,-3,3);
                hSPvsDCAprev[i]=new TH2D(Form("hSPvsDCAprev%d",i),
                                       Form("Run %d iter %d Spatial resolution vs R(prev) for layer %d;DCA;HitR-DCA[mm]",
                                            fRunNo,fIter,i),128,-10,10,128,-3,3);
                hSPvsDCAcurr[i]=new TH2D(Form("hSPvsDCAcurr%d",i),
                                       Form("Run %d iter %d Spatial resolution vs R(curr) for layer %d;DCA;HitR-DCA[mm];",
                                            fRunNo,fIter,i),128,-10,10,128,-3,3);
		hHitRcurrprevDiff[i] =new TH1D(Form("hHitRcurrprevDiff%d",i),
					       Form("Run %d iter %d Current-Previous for layer %d;HitR_c-HitR_p[mm];",
						    fRunNo,fIter,i),128,-3,3);
		hHitRcurrprevDiff2D[i] =new TH2D(Form("hHitRcurrprevDiff2D%d",i),
						 Form("Run %d iter %d Current-Previous for layer %d;HitR_c;HitR_c-HitR_p[mm]",
						      fRunNo,fIter,i),256,-10,10,256,-1,1);

		// Bin histgrams
		gSPvsDCAcurr[i]=new TGraphErrors(SP_BIN);
		gSPvsDCAcurr[i]->SetName(Form("gSPvsDCAcurr_l%d",i));
		gSPvsDCAcurr[i]->SetTitle(Form("gSPvsDCAcurr_l%d",i));
		gSPvsDCAcurr[i]->SetMarkerColor(1);
		gSPvsDCAcurr[i]->SetLineColor(1);
		gSPvsDCAcurr[i]->SetMarkerStyle(6);
		gSPvsDCAcurr[i]->SetFillColor(1);
		gSPvsDCAcurr[i]->SetFillStyle(3010);
		gSPvsDCAprev[i]=new TGraphErrors(SP_BIN);
		gSPvsDCAprev[i]->SetName(Form("gSPvsDCAprev_l%d",i));
		gSPvsDCAprev[i]->SetTitle(Form("gSPvsDCAprev_l%d",i));
		gSPvsDCAprev[i]->SetMarkerColor(1);
		gSPvsDCAprev[i]->SetLineColor(1);
		gSPvsDCAprev[i]->SetMarkerStyle(4);
		gSPvsDCAprev[i]->SetLineStyle(4);
		gSPvsDCAprev[i]->SetFillColor(1);
		gSPvsDCAprev[i]->SetFillStyle(3010);
		//Right
		gSPvsDCARcurr[i]=new TGraphErrors(SP_BIN);
		gSPvsDCARcurr[i]->SetName(Form("gSPvsDCARcurr_l%d",i));
		gSPvsDCARcurr[i]->SetTitle(Form("gSPvsDCARcurr_l%d",i));
		gSPvsDCARcurr[i]->SetMarkerColor(2);
		gSPvsDCARcurr[i]->SetLineColor(2);
		gSPvsDCARcurr[i]->SetMarkerStyle(6);
		gSPvsDCARcurr[i]->SetFillColor(1);
		gSPvsDCARcurr[i]->SetFillStyle(3010);
		gSPvsDCARprev[i]=new TGraphErrors(SP_BIN);
		gSPvsDCARprev[i]->SetName(Form("gSPvsDCARprev_l%d",i));
		gSPvsDCARprev[i]->SetTitle(Form("gSPvsDCARprev_l%d",i));
		gSPvsDCARprev[i]->SetMarkerColor(2);
		gSPvsDCARprev[i]->SetLineColor(2);
		gSPvsDCARprev[i]->SetMarkerStyle(4);
		gSPvsDCARprev[i]->SetLineStyle(4);
		gSPvsDCARprev[i]->SetFillColor(2);
		gSPvsDCARprev[i]->SetFillStyle(3010);
		// Left
		gSPvsDCALcurr[i]=new TGraphErrors(SP_BIN);
		gSPvsDCALcurr[i]->SetName(Form("gSPvsDCALcurr_l%d",i));
		gSPvsDCALcurr[i]->SetTitle(Form("gSPvsDCALcurr_l%d",i));
		gSPvsDCALcurr[i]->SetMarkerColor(3);
		gSPvsDCALcurr[i]->SetLineColor(3);
		gSPvsDCALcurr[i]->SetMarkerStyle(6);
		gSPvsDCALcurr[i]->SetFillColor(3);
		gSPvsDCALcurr[i]->SetFillStyle(3010);
		gSPvsDCALprev[i]=new TGraphErrors(SP_BIN);
		gSPvsDCALprev[i]->SetName(Form("gSPvsDCALprev_l%d",i));
		gSPvsDCALprev[i]->SetTitle(Form("gSPvsDCALprev_l%d",i));
		gSPvsDCALprev[i]->SetMarkerColor(3);
		gSPvsDCALprev[i]->SetLineColor(3);
		gSPvsDCALprev[i]->SetMarkerStyle(6);
		gSPvsDCALprev[i]->SetLineStyle(4);
		gSPvsDCALprev[i]->SetFillColor(3);
		gSPvsDCALprev[i]->SetFillStyle(3010);

		for(int ib=0;ib<SP_BIN;ib++){
		    double r_l=ib*CELL_BOUND*1./SP_BIN;
		    double r_u=(ib+1)*CELL_BOUND*1./SP_BIN;
		    hDCAprevSliceDCA[i].push_back(new TH1D(Form("hDCAprevSliceDCA_l%d_i%d",i,ib),
							    Form("hDCAprevSliceDCA_%d (%.2f~%.2f)mm ;DCA[mm];",ib,r_l,r_u),
							    128,-CELL_BOUND,CELL_BOUND));
		    hDCAcurrSliceDCA[i].push_back(new TH1D(Form("hDCAcurrSliceDCA_l%d,i%d",i,ib),
							    Form("hDCAcurrSliceDCA_%d (%.2f~%.2f)mm ;DCA[mm];",ib,r_l,r_u),
							   128,-CELL_BOUND,CELL_BOUND));
		    hDCAprevSliceDCAR[i].push_back(new TH1D(Form("hDCAprevSliceDCAR_l%d_i%d",i,ib),
							    Form("hDCAprevSliceDCAR_%d (%.2f~%.2f)mm ;DCA[mm];",ib,r_l,r_u),
							    128,-CELL_BOUND,CELL_BOUND));
		    hDCAcurrSliceDCAR[i].push_back(new TH1D(Form("hDCAcurrSliceDCAR_l%d,i%d",i,ib),
							    Form("hDCAcurrSliceDCAR_%d (%.2f~%.2f)mm ;DCA[mm];",ib,r_l,r_u),
							   128,-CELL_BOUND,CELL_BOUND));
		    hDCAprevSliceDCAL[i].push_back(new TH1D(Form("hDCAprevSliceDCAL_l%d_i%d",i,ib),
							    Form("hDCAprevSliceDCAL_%d (%.2f~%.2f)mm ;DCA[mm];",ib,r_l,r_u),
							    128,-CELL_BOUND,CELL_BOUND));
		    hDCAcurrSliceDCAL[i].push_back(new TH1D(Form("hDCAcurrSliceDCAL_l%d,i%d",i,ib),
							    Form("hDCAcurrSliceDCAL_%d (%.2f~%.2f)mm ;DCA[mm];",ib,r_l,r_u),
							    128,-CELL_BOUND,CELL_BOUND));
		}

            }
        }
    public:

        MyAnalysisCode(int runNo, int iter, TString inputPath);

        virtual ~MyAnalysisCode(){
        }

        void FileLoop(Int_t lowerLay, Int_t upperLay){
            char *rootDIR = getenv("CCTRACKROOT_DIR");
            for(int iLay=lowerLay;iLay<upperLay+1;iLay++){// Take the average of the middle layers
                if(fInputPath!="")
                    fFile = new TFile(fInputPath);
                else
                    fFile = new TFile(Form("%s/xt_1010_i%d_l%d.root",rootDIR,fIter,iLay));
                if(fFile->IsOpen()){
                    std::cout << "## Root File exists " << fFile->GetName() << std::endl;
                }else{
                    std::cerr << "## Do not have root file" << std::endl;
                    continue ;
                }
                fTree = (TTree*)fFile->Get("t");
                fTree->SetBranchAddress("trackPar", &fTrackPar_in);
                fTree->SetBranchAddress("DOF", &fDOF_in);
                fTree->SetBranchAddress("chi2", &fChi2_in);
                fTree->SetBranchAddress("chi2p", &fChi2p_in);
                fTree->SetBranchAddress("layerID", &fLayerID_in);
                fTree->SetBranchAddress("cellID", &fCellID_in);
                fTree->SetBranchAddress("closestCell", &fClosestCell_in);
                fTree->SetBranchAddress("trackIndex", &fTrackIndex_in);
                fTree->SetBranchAddress("driftTime",&fDriftTimeBest_in);
                fTree->SetBranchAddress("fitR",&fFitR_in);
                fTree->SetBranchAddress("phiAng",&fPhiAng_in);
                fTree->SetBranchAddress("hitRBest",&fHitRBest_in);
                fTree->SetBranchAddress("trkIdMinResTestLayt",&fTrkIdMinResTestLay_in);
		//for(int iBin=0;iBin<SP_BIN;iBin++)
                // First make sure the x-t distribtion is in the middle
                // Assumption is that the middle part of the x-t relation is symmetry
                double avg_dv=0;
                double avg_dv_count=0;
                for(int iev=0;iev<fTree->GetEntries();iev++){
                    //for(int iev=0;iev<1;iev++){
                    fTree->GetEntry(iev);
                    const int nsize = (int)fLayerID_in->size();
                    for(int i=0;i<nsize;i++){//hit loop
                        int lid = fLayerID_in->at(i);
                        // Only loop at current test layer
                        if(lid!=iLay) continue;
                        // Cut out all tracks not best
                        if(fChi2p_in->at(i)<=0.05 || fChi2_in->at(i)/fDOF_in->at(i)>=10 || fDOF_in->at(i)!=3) continue;
                        if(fTrackIndex_in->at(i)!=fTrkIdMinResTestLay_in) continue;
                        int cid = fCellID_in->at(i);
                        int ccid = fClosestCell_in->at(i);
                        // Get information and fill left right historgarm
                        if(cid!=ccid)continue;
                        double dt      = fDriftTimeBest_in->at(i);
                        double phi     = fPhiAng_in->at(i);
                        double dr      = fFitR_in->at(i);
                        double hr_prev = fHitRBest_in->at(i);
                        double sp_prev = fabs(hr_prev)-fabs(dr);
                        double hr_curr = ICDCGas::Get().GetDriftDistance(dt,lid,cid,phi,0);
                        //printf("hr_curr %f phi %f test %f \n",hr_curr,phi,ICDCGas::Get().GetDriftDistance(dt,lid,cid,,0));
                        double sp_curr = fabs(hr_curr)-fabs(dr);
                        hHitRprev[lid]->Fill(hr_prev);
                        hHitRcurr[lid]->Fill(hr_curr);
                        hSPprev[lid]->Fill(sp_prev);
                        hSPcurr[lid]->Fill(sp_curr);
                        hSPvsDCAprev[lid]->Fill(dr,sp_prev);
                        hSPvsDCAcurr[lid]->Fill(dr,sp_curr);
			hHitRcurrprevDiff[lid]->Fill(hr_curr-hr_prev);
			hHitRcurrprevDiff2D[lid]->Fill(hr_curr,hr_curr-hr_prev);
			for(int ib=0;ib<SP_BIN;ib++){
			    double r_l=ib*CELL_BOUND*1./SP_BIN;
			    double r_u=(ib+1)*CELL_BOUND*1./SP_BIN;
			    if(fabs(dr)>=r_l && fabs(dr)<r_u){
				hDCAprevSliceDCA[lid][ib]->Fill(sp_prev);
				hDCAcurrSliceDCA[lid][ib]->Fill(sp_curr);
				if(dr>=0){
				    hDCAprevSliceDCAR[lid][ib]->Fill(sp_prev);
				    hDCAcurrSliceDCAR[lid][ib]->Fill(sp_curr);
				}else{
				    hDCAprevSliceDCAL[lid][ib]->Fill(sp_prev);
				    hDCAcurrSliceDCAL[lid][ib]->Fill(sp_curr);
				}
			    }	
			}
                    }
                }
		// Fit gauss
		for(int ib=0;ib<SP_BIN;ib++){
		    double r_l=ib*CELL_BOUND*1./SP_BIN;
		    double r_u=(ib+1)*CELL_BOUND*1./SP_BIN;
		    double gx = (r_l+r_u)/2;
		    double gy = 0;
		    double gye = 0;
		    if(hDCAprevSliceDCA[iLay][ib]->GetEntries()){
			hDCAprevSliceDCA[iLay][ib]->Fit("gaus","","",-0.55,0.55);
			gy=hDCAprevSliceDCA[iLay][ib]->GetFunction("gaus")->GetParameter(2);
			gye=hDCAprevSliceDCA[iLay][ib]->GetFunction("gaus")->GetParError(2);
		    }
		    gSPvsDCAprev[iLay]->SetPoint(ib,gx,gy);
		    gSPvsDCAprev[iLay]->SetPointError(ib,CELL_BOUND*1./SP_BIN/2,gye);
		    if(hDCAcurrSliceDCA[iLay][ib]->GetEntries()){
			hDCAcurrSliceDCA[iLay][ib]->Fit("gaus","","",-0.55,0.55);
			gy=hDCAcurrSliceDCA[iLay][ib]->GetFunction("gaus")->GetParameter(2);
			gye=hDCAcurrSliceDCA[iLay][ib]->GetFunction("gaus")->GetParError(2);
		    }
		    gSPvsDCAcurr[iLay]->SetPoint(ib,gx,gy);
		    gSPvsDCAcurr[iLay]->SetPointError(ib,CELL_BOUND*1./SP_BIN/2,gye);
		    // RIght
		    if(hDCAprevSliceDCAR[iLay][ib]->GetEntries()){
			hDCAprevSliceDCAR[iLay][ib]->Fit("gaus","","",-0.55,0.55);
			gy=hDCAprevSliceDCAR[iLay][ib]->GetFunction("gaus")->GetParameter(2);
			gye=hDCAprevSliceDCAR[iLay][ib]->GetFunction("gaus")->GetParError(2);
		    }
		    gSPvsDCARprev[iLay]->SetPoint(ib,gx,gy);
		    gSPvsDCARprev[iLay]->SetPointError(ib,CELL_BOUND*1./SP_BIN/2,gye);
		    if(hDCAcurrSliceDCAR[iLay][ib]->GetEntries()){
			hDCAcurrSliceDCAR[iLay][ib]->Fit("gaus","","",-0.55,0.55);
			gy=hDCAcurrSliceDCAR[iLay][ib]->GetFunction("gaus")->GetParameter(2);
			gye=hDCAcurrSliceDCAR[iLay][ib]->GetFunction("gaus")->GetParError(2);
		    }
		    gSPvsDCARcurr[iLay]->SetPoint(ib,gx,gy);
		    gSPvsDCARcurr[iLay]->SetPointError(ib,CELL_BOUND*1./SP_BIN/2,gye);
		    // Left handside current
		    if(hDCAcurrSliceDCAL[iLay][ib]->GetEntries()){
			hDCAcurrSliceDCAL[iLay][ib]->Fit("gaus","","",-0.55,0.55);
			gy=hDCAcurrSliceDCAL[iLay][ib]->GetFunction("gaus")->GetParameter(2);
			gye=hDCAcurrSliceDCAL[iLay][ib]->GetFunction("gaus")->GetParError(2);
		    }
		    gSPvsDCALcurr[iLay]->SetPoint(ib,gx,gy);
		    gSPvsDCALcurr[iLay]->SetPointError(ib,CELL_BOUND*1./SP_BIN/2,gye);
		    // Left handside previous
		    if(hDCAprevSliceDCAL[iLay][ib]->GetEntries()){
			hDCAprevSliceDCAL[iLay][ib]->Fit("gaus","","",-0.55,0.55);
			gy=hDCAprevSliceDCAL[iLay][ib]->GetFunction("gaus")->GetParameter(2);
			gye=hDCAprevSliceDCAL[iLay][ib]->GetFunction("gaus")->GetParError(2);
		    }
		    gSPvsDCALprev[iLay]->SetPoint(ib,gx,gy);
		    gSPvsDCALprev[iLay]->SetPointError(ib,CELL_BOUND*1./SP_BIN/2,gye);

		}
            }
        }
        void Finalize(){
            // Write histogram
	    char *outputXtDir=getenv("CCPARAMETER_DIR");
            TFile * fout = new TFile(Form("%s/tx_iteration.r%d.i%d.root",outputXtDir,fRunNo,fIter),"recreate");
            for(int iLay=0;iLay<MAX_SENSE_LAYER;iLay++){
                if(hHitRprev[iLay]->GetEntries())  hHitRprev[iLay]->Write();
                if(hHitRcurr[iLay]->GetEntries())  hHitRcurr[iLay]->Write();
                if(hSPprev[iLay]->GetEntries())  hSPprev[iLay]->Write();
                if(hSPcurr[iLay]->GetEntries())  hSPcurr[iLay]->Write();
                if(hSPvsDCAprev[iLay]->GetEntries())  hSPvsDCAprev[iLay]->Write();
                if(hSPvsDCAcurr[iLay]->GetEntries())  hSPvsDCAcurr[iLay]->Write();
                if(hHitRcurrprevDiff[iLay]->GetEntries())  hHitRcurrprevDiff[iLay]->Write();
                if(hHitRcurrprevDiff2D[iLay]->GetEntries())  hHitRcurrprevDiff2D[iLay]->Write();
		for(int ib=0;ib<SP_BIN;ib++){
		    if(hDCAprevSliceDCA[iLay][ib]->GetEntries()) hDCAprevSliceDCA[iLay][ib]->Write();
		    if(hDCAcurrSliceDCA[iLay][ib]->GetEntries()) hDCAcurrSliceDCA[iLay][ib]->Write();
		    if(hDCAprevSliceDCAR[iLay][ib]->GetEntries()) hDCAprevSliceDCAR[iLay][ib]->Write();
		    if(hDCAcurrSliceDCAR[iLay][ib]->GetEntries()) hDCAcurrSliceDCAR[iLay][ib]->Write();
		    if(hDCAprevSliceDCAL[iLay][ib]->GetEntries()) hDCAprevSliceDCAL[iLay][ib]->Write();
		    if(hDCAcurrSliceDCAL[iLay][ib]->GetEntries()) hDCAcurrSliceDCAL[iLay][ib]->Write();
		}
		double x=0;
		double y=0;
		gSPvsDCAcurr[iLay]->GetPoint(0,x,y);
		if(fabs(x)>1e-5 && fabs(y)>1e-5){
		    printf("gSPvsDCAcurr layer %d Wrote ",iLay);
		    gSPvsDCAcurr[iLay]->Write();
		}
		gSPvsDCAprev[iLay]->GetPoint(0,x,y);
		if(fabs(x)>1e-5 && fabs(y)>1e-5){
		    printf("gSPvsDCAprev layer %d Wrote ",iLay);
		    gSPvsDCAprev[iLay]->Write();
		}
		// Right
		gSPvsDCARcurr[iLay]->GetPoint(0,x,y);
		if(fabs(x)>1e-5 && fabs(y)>1e-5){
		    printf("gSPvsDCAcurr layer %d Wrote ",iLay);
		    gSPvsDCARcurr[iLay]->Write();
		}
		gSPvsDCARprev[iLay]->GetPoint(0,x,y);
		if(fabs(x)>1e-5 && fabs(y)>1e-5){
		    printf("gSPvsDCAprev layer %d Wrote ",iLay);
		    gSPvsDCARprev[iLay]->Write();
		}
		// Left handside current
		gSPvsDCALcurr[iLay]->GetPoint(0,x,y);
		if(fabs(x)>1e-5 && fabs(y)>1e-5){
		    printf("gSPvsDCALcurr layer %d Wrote ",iLay);
		    gSPvsDCALcurr[iLay]->Write();
		}
		// Left handside previous
		gSPvsDCALprev[iLay]->GetPoint(0,x,y);
		if(fabs(x)>1e-5 && fabs(y)>1e-5){
		    printf("gSPvsDCALprev layer %d Wrote ",iLay);
		    gSPvsDCALprev[iLay]->Write();
		}
            }
	    // Drawing
            for(int iLay=0;iLay<MAX_SENSE_LAYER;iLay++){
                if(hHitRprev[iLay]->GetEntries()){
		    fCanvas->Clear(); fCanvas->cd();
		    hHitRprev[iLay]->Draw();
		    fCanvas->SaveAs(Form("%s/hHitRprev_r%d_i%d_l%d.png",fGraphDir,fRunNo,fIter,iLay));
		}
                if(hHitRcurr[iLay]->GetEntries()){
		    fCanvas->Clear(); fCanvas->cd();
		    hHitRcurr[iLay]->Draw();
		    fCanvas->SaveAs(Form("%s/hHitRcurr_r%d_i%d_l%d.png",fGraphDir,fRunNo,fIter,iLay));
		}
                if(hSPprev[iLay]->GetEntries()){
		    fCanvas->Clear(); fCanvas->cd();
		    hSPprev[iLay]->Draw();
		    fCanvas->SaveAs(Form("%s/hSPprev_r%d_i%d_l%d.png",fGraphDir,fRunNo,fIter,iLay));
		}
		if(hSPcurr[iLay]->GetEntries()){
		    fCanvas->Clear(); fCanvas->cd();
		    hSPcurr[iLay]->Draw();
		    fCanvas->SaveAs(Form("%s/hSPcurr_r%d_i%d_l%d.png",fGraphDir,fRunNo,fIter,iLay));
		}
                if(hSPvsDCAprev[iLay]->GetEntries()){
		    fCanvas->Clear(); fCanvas->cd();
		    hSPvsDCAprev[iLay]->Draw("colz");
		    fCanvas->SaveAs(Form("%s/hSPvsDCAprev_r%d_i%d_l%d.png",fGraphDir,fRunNo,fIter,iLay));
		}
                if(hSPvsDCAcurr[iLay]->GetEntries()){
		    fCanvas->Clear(); fCanvas->cd();
		    hSPvsDCAcurr[iLay]->Draw("colz");
		    fCanvas->SaveAs(Form("%s/hSPvsDCAcurr_r%d_i%d_l%d.png",fGraphDir,fRunNo,fIter,iLay));
		}
                if(hHitRcurrprevDiff[iLay]->GetEntries()){
		    fCanvas->Clear(); fCanvas->cd();
		    hHitRcurrprevDiff[iLay]->Draw();
		    fCanvas->SaveAs(Form("%s/hHitRcurrprevDiff_r%d_i%d_l%d.png",fGraphDir,fRunNo,fIter,iLay));
		}
                if(hHitRcurrprevDiff2D[iLay]->GetEntries()){
		    fCanvas->Clear(); fCanvas->cd();
		    hHitRcurrprevDiff2D[iLay]->Draw("colz");
		    fCanvas->SaveAs(Form("%s/hHitRcurrprevDiff2D_r%d_i%d_l%d.png",fGraphDir,fRunNo,fIter,iLay));
		}
		if(hDCAprevSliceDCA[iLay][0]->GetEntries()){
		    int nw = (int)sqrt(SP_BIN);
		    fCanvas->Clear(); fCanvas->Divide(nw,nw);
		    for(int ib=0;ib<SP_BIN;ib++){
			fCanvas->cd(ib+1);
			gStyle->SetOptFit(111);
			hDCAprevSliceDCA[iLay][ib]->Draw();
		    }
		    fCanvas->SaveAs(Form("%s/hDCAprevSliceDCA_r%d_i%d_l%d.png",fGraphDir,fRunNo,fIter,iLay));
		}
		if(hDCAprevSliceDCAL[iLay][0]->GetEntries()){
		    int nw = (int)sqrt(SP_BIN);
		    fCanvas->Clear(); fCanvas->Divide(nw,nw);
		    for(int ib=0;ib<SP_BIN;ib++){
			fCanvas->cd(ib+1);
			gStyle->SetOptFit(111);
			hDCAprevSliceDCAL[iLay][ib]->Draw();
		    }
		    fCanvas->SaveAs(Form("%s/hDCAprevSliceDCAL_r%d_i%d_l%d.png",fGraphDir,fRunNo,fIter,iLay));
		}
		if(hDCAprevSliceDCAR[iLay][0]->GetEntries()){
		    int nw = (int)sqrt(SP_BIN);
		    fCanvas->Clear(); fCanvas->Divide(nw,nw);
		    for(int ib=0;ib<SP_BIN;ib++){
			fCanvas->cd(ib+1);
			gStyle->SetOptFit(111);
			hDCAprevSliceDCAR[iLay][ib]->Draw();
		    }
		    fCanvas->SaveAs(Form("%s/hDCAprevSliceDCAR_r%d_i%d_l%d.png",fGraphDir,fRunNo,fIter,iLay));
		}
		double x,y;
		gSPvsDCAcurr[iLay]->GetPoint(0,x,y);
		if(fabs(x)>1e-5 && fabs(y)>1e-5){
		    fCanvas->Clear(); fCanvas->cd();
		    gSPvsDCAcurr[iLay]->Draw("apl");
		    gSPvsDCAprev[iLay]->Draw("plsame");
		    fCanvas->SaveAs(Form("%s/gSPvsDCAcurr_r%d_i%d_l%d.png",fGraphDir,fRunNo,fIter,iLay));
		}
		// Right
		gSPvsDCARcurr[iLay]->GetPoint(0,x,y);
		if(fabs(x)>1e-5 && fabs(y)>1e-5){
		    fCanvas->Clear(); fCanvas->cd();
		    gSPvsDCARcurr[iLay]->Draw("apl");
		    gSPvsDCARprev[iLay]->Draw("plsame");
		    fCanvas->SaveAs(Form("%s/gSPvsDCARcurr_r%d_i%d_l%d.png",fGraphDir,fRunNo,fIter,iLay));
		}
		// Left handside current
		gSPvsDCALcurr[iLay]->GetPoint(0,x,y);
		if(fabs(x)>1e-5 && fabs(y)>1e-5){
		    fCanvas->Clear(); fCanvas->cd();
		    gSPvsDCALcurr[iLay]->Draw("apl");
		    gSPvsDCALprev[iLay]->Draw("plsame");
		    fCanvas->SaveAs(Form("%s/gSPvsDCALcurr_r%d_i%d_l%d.png",fGraphDir,fRunNo,fIter,iLay));
		}
	    }
        }
};

MyAnalysisCode::MyAnalysisCode(int runNo, int iter,TString inputPath)
    :fTrackPar_in(0),
     fDOF_in(0),
     fChi2_in(0),
     fChi2p_in(0),
     fLayerID_in(0),
     fCellID_in(0),
     fClosestCell_in(0),
     fTrackIndex_in(0),
     fDriftTimeBest_in(0),
     fFitR_in(0),
     fPhiAng_in(0),
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
    userCode.Finalize();
}
