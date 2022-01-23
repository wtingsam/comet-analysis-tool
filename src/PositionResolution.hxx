#ifndef __POSITIONRESOLUTION_HXX__
#define __POSITIONRESOLUTION_HXX__

#include "TF1.h"
#include "TString.h"
#include "TGraphErrors.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TH2D.h"
#include <stdlib.h>
#include <math.h>

#include "ICDCGas.hxx"

class PositionResolution {
    public:
        /// Initialize a function for fitting
        void Initialize(bool useOwnFunction=false);
        /// Initialize a function with specific name
        void Initialize(TString path, TString functionName, bool useOwnFunction=false);

        virtual ~PositionResolution();

        /// Read default function
        void ReadDefault();

        /// Read and set the function mannually
        void ReadFunctionFrom(TString path, TString name);

        /// Close the file for position error
        void Close(void){    fFileErr->Close();   }

        /// To do lists...
        /// Initialize parameters for all layers fixe me
        /// Ideally, we have to use an array for each layer
        void Initialize(){}
        /// Copy parameters from the pre-saved text file
        void CopyParFromText(){}
        /// Array for describing position resolution
        /// So in manager class, we can call this variable to evaluate and create ONLY 1 TF1 function
        double fPosResPara[20][5000][10] ;// [Layer][cell][num of parameters]
    protected:

        double fDefaultIntrSReso;
        TF1 *fFittingFunction;
        TFile *fFileErr;
        TString fNameFittingFunction;
        bool fIsUseErrFun;
};


#endif
