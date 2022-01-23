#ifndef __IPosResFitting_hxx__
#define __IPosResFitting_hxx__

#include "TFile.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TString.h"
#include "TCanvas.h"
#include "TH2D.h"
#include "TLegend.h"


#include <stdlib.h>
#include <math.h>
/***
    This class is dedicated for fitting of position resolution.
    Input is TGraphError
    Output can be printing of pdf and also the the fitting function
 ***/

class IPosResFitting{
    public:
	IPosResFitting(TString name):fGraph_ResVsR(NULL){
	    ReadPrimaryIonsTable();
	    fNameFittingFunction = name;
	    fFittingFunction = new TF1(name,WirePositionRes_Fitting,0.1,8,5);
	    //Set fit starting point dedicated for He-iC4H10(90/10)
	    double npi = 1.4; 
	    double diff_coeff = 5e-5;
	    double drift_vel = 0.025; // mm/ns
	    double dtErr = 1; // ns
	    double extErr = 0.05; // mm
	    fFittingFunction->SetParameter(0,npi);	       
	    fFittingFunction->SetParameter(1,diff_coeff);    
	    fFittingFunction->SetParameter(2,drift_vel);     
	    fFittingFunction->FixParameter(3,dtErr); 
	    fFittingFunction->FixParameter(4,extErr);
	}
	~IPosResFitting(){};

	void Fit(TGraphErrors *g, TString option, Double_t min, Double_t max);

	void Print(TString figureName="output.pdf");
	
	void SetFunction(TF1 *fit){  fFittingFunction=fit;  }

	TF1 *GetFunction(){  return fFittingFunction;  }
	
	/// Write the fitting function into a file : fileName
	void WriteFitFunction(TString fileName);
	
	/// Static function for fitting
	static Double_t WirePositionRes_Fitting(Double_t* x, Double_t* par);
	static void ReadPrimaryIonsTable();
	static void GetPars(double* pars, double npri);
	static Double_t PrimaryIonsErr_Fitting(Double_t* x, Double_t* par);
	static Double_t DiffusionErr_Fitting(Double_t* x, Double_t* par);
	static Double_t DriftTimeErr_Fitting(Double_t* x, Double_t* par);
	static Double_t TrackingErr_Fiting(Double_t* x, Double_t* par);
    
    private: 
	TF1 *fFittingFunction;
	TGraphErrors *fGraph_ResVsR;
	TString fNameFittingFunction;	
};

#endif 
