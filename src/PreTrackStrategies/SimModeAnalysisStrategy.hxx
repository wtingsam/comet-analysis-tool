#ifndef _SIMMODEANALYSISSTRATEGY_HH_
#define _SIMMODEANALYSISSTRATEGY_HH_
#include "IHit.hxx"

#include "AnalysisStrategy.hxx"
#include "IRunTimeManager.hxx"

#include "WaveFormAnalyzer.hxx"
#include "DriftTimeAnalyzer.hxx"
#include "TCanvas.h"
#include "TH1D.h"
#include "TF1.h"
#include "TStyle.h"

class SimModeAnalysisStrategy: public AnalysisStrategy{
    private: 
        //output variable
	
    public:
	SimModeAnalysisStrategy(InputRootCRT *input):
	    AnalysisStrategy(input)
	    {
		AnalysisStrategy::Init();
	    }
	virtual ~SimModeAnalysisStrategy(){}

    private:
	
	void GetPreTrackingParameters();

	void PrepareRootFile();
	
	/// Print out the results of fitting on drift time
	void Print();
};


#endif
