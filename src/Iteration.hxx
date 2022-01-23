#ifndef _ITERATION_HXX_
#define _ITERATION_HXX_
#include <TString.h>
#include <TF1.h>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <map>
#include <set>

using namespace std;
/***
    This is a singleton class.
    Only call it one time when you need it at the beginning of the main function
***/

class Iteration {
    public:
        ~Iteration();

        /// Initialize the iteration
        void Initialize(Int_t iterStep);

        /// Check if the line is empty
        bool IsEmpty(TString s_card);

        /// Read calibration routine
        bool ReadRoutine(TString routine);

        /// Get Name and State
        TString GetRoutineName() {  return fRoutine;       }
        TString GetXtCalState()  {  return fXtCalState;    }
        TString GetT0CalState()  {  return fT0CalState;    }
        TString GetErrCalState() {  return fErrCalState;   }
        TString GetWireCalState(){  return fWireCalState;  }
        TString GetEPCalState()  {  return fEPCalState;    }

        /// Print the status of calibration routine
        void Print();

        static Iteration &Get();
    private:
        TString fRoutine;
        TString fXtCalState;
        TString fT0CalState;
        TString fErrCalState;
        TString fWireCalState;
        TString fEPCalState;

        Int_t fXtAvgFromLay;
        Int_t fXtAvgToLay;
        Int_t fXtUseFromLay;
        Int_t fXtUseToLay;

        Int_t fErrAvgFromLay;
        Int_t fErrAvgToLay;
        Int_t fErrUseFromLay;
        Int_t fErrUseToLay;
        Int_t fInputIterStep;

        Iteration(){}     // Don't Implement
        Iteration(Iteration const&){}        // Don't Implement
        void operator=(Iteration const&); // Don't implement
        static Iteration* fIteration;
};

#endif
