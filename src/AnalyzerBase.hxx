#ifndef _ANALYZERBASE_HH_
#define _ANALYZERBASE_HH_

#include "TF1.h"
#include "TH1D.h"
#include <iostream>
#include <math.h>

#include "WireManager.hxx"
#include "ExperimentConfig.hxx"

class AnalyzerBase{
    public:
        AnalyzerBase();
        virtual ~AnalyzerBase();

        virtual void Initialize(){}

    private:

};

#endif
