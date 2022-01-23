#ifndef _P4SPRING8ANALYSISSTRATEGY_HH_
#define _P4SPRING8ANALYSISSTRATEGY_HH_
#include "IHit.hxx"

#include "AnalysisStrategy.hxx"
#include "IRunTimeManager.hxx"

#include "TCanvas.h"
#include "TH1D.h"
#include "TF1.h"
#include "TStyle.h"

class P4SPring8AnalysisStrategy: public AnalysisStrategy{
    public:
        P4SPring8AnalysisStrategy(InputRootCRT *input):
            AnalysisStrategy(input)
            {
                AnalysisStrategy::Init();
                AnalysisStrategy::Branch();
            }
        virtual ~P4SPring8AnalysisStrategy(){}

    private:
        void GetPreTrackingParameters();

        void PrepareRootFile();

        /// Print out the results of fitting on drift time
        void Print();

};


#endif
