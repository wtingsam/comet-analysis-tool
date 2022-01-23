#ifndef _DRIFTTIMEANALYZER_HH_
#define _DRIFTTIMEANALYZER_HH_

#include "AnalyzerBase.hxx"

class DriftTimeAnalyzer : public AnalyzerBase{

    public:
        DriftTimeAnalyzer();
        virtual ~DriftTimeAnalyzer();
	
        /// Because of the FPGA clock, the TDC shoudl be fixed every 2^15
	Double_t FixTDC(Int_t TDC);
        Double_t FixTDC(Double_t TDC);
	Double_t TDC2DriftTime(Int_t TDC){
	    return TDC*1000./960;
	}
 
	/// Fit the drift time distribution to get the t0 offset
	Double_t FitT0(TH1D *h, TF1 *fitFunction, TString option="Q0");
 
	/// Fit the drift time distribution to get the t0 offset
	Double_t FitT0(TH1D *h, TF1 *fitFunction, TString option,Double_t min, Double_t max);
 
        /// Fit the drift time distribution to get the edge/boundary of the drift time
        Double_t FitEdge(TH1D *h, TF1 *fitFunction, TString option="Q0");
	
        /// Fitting functions from literatures
        static Double_t FitFunction_findt0(Double_t *t,Double_t *par);
        static Double_t FitFunction_findEdge(Double_t *t,Double_t *par);
        static Double_t FitFunction_driftTime(Double_t *t,Double_t *par);
};

#endif
