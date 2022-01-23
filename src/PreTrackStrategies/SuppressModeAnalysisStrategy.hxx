#ifndef _SUPPRESSMODEANALYSISSTRATEGY_HH_
#define _SUPPRESSMODEANALYSISSTRATEGY_HH_

#include "AnalysisStrategy.hxx"

class SuppressModeAnalysisStrategy: public AnalysisStrategy{
    public:
	SuppressModeAnalysisStrategy(InputRootCRT *input):
	    AnalysisStrategy(input)
	    {}
    private:
	void GetPreTrackingParameters(){
	    printf("## I am analyzing considering suppress mode\n");
	    std::cout << fInput->GetEntries() << std::endl;
	}
	void PrepareRootFile(){
	    printf("## Under construction \n");
	}
	void Print(){
	    printf("## Under construction \n");
	}
	void ClearEvent(){
	    printf("## Under construction \n");
	}

};


#endif
