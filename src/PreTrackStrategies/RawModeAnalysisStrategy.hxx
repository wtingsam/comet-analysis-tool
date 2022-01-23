#ifndef _RAWMODEANALYSISSTRATEGY_HH_
#define _RAWMODEANALYSISSTRATEGY_HH_
#include "IHit.hxx"

#include "AnalysisStrategy.hxx"
#include "IRunTimeManager.hxx"

#include "TCanvas.h"
#include "TH1D.h"
#include "TF1.h"
#include "TStyle.h"

class RawModeAnalysisStrategy: public AnalysisStrategy{
    public:
	RawModeAnalysisStrategy(InputRootCRT *input, bool triggerDelay):
	    AnalysisStrategy(input,triggerDelay)
	    {
		AnalysisStrategy::Init();
		AnalysisStrategy::Branch();
	    }
	virtual ~RawModeAnalysisStrategy(){}

    private:      
	
	virtual void GetPreTrackingParameters();

	virtual void PrepareRootFile();
	
	/// Print out the results of fitting on drift time
	virtual void Print();
	
};


#endif
