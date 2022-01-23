#ifndef _INPUTROOTCRT_HXX_
#define _INPUTROOTCRT_HXX_

#include <iostream>
#include <TFile.h>
#include <TTree.h>

#define MAX_SENSE_WIRE 1000 // The max should be 4986
#define MAX_SAMPLING 32

class InputRootCRT{
/**
   This class is for input files after binary to root file.
   If you would like to change any structure of input format,
   Please change it here.
   The Input file is based on ROOT format.
**/
    public:

        /// Constructor
        InputRootCRT(TString path, TString option, TString tree, bool useSimData=false);

        /// Destructor
        virtual ~InputRootCRT();

        /// Get entry same as the one in ROOT
        virtual void GetEntry(Int_t iev){  fTree->GetEntry(iev);  }

        /// Get Number of entries
        Int_t GetEntries(void){  return fTree->GetEntries();  }

        /// Get Branch value
        Int_t GetTDC(Int_t ch, Int_t clk){  return fTDC[ch][clk];  }
        Int_t GetADC(Int_t ch, Int_t clk){  return fADC[ch][clk];  }
        Int_t GetClkHit(Int_t ch, Int_t clk){  return fClkHit[ch][clk];  }
        Int_t GetTDCNhit(Int_t ch){  return fTDCNhit[ch];  }
        Int_t GetTriggerNumber(void){  return fTriggerNumber;  }

        /// For simulation
        Double_t GetDriftTimeSim(Int_t ch, Int_t clk){  return fDriftTime_sim[ch][clk];  }

        virtual void SetOption(TString option){  fOption=option; fReadyState++; }
        virtual void SetPath(TString path){  fRootFilePath=path; fReadyState++; }
        virtual void SetTree(TString tree){  fTreeName=tree; fReadyState++; }

    private:
        /// If the root format from MIDAS/Binary2Root is changed.
        /// Please Check the branch Address as well.
        /// Initialization
        void Initialize(void);

        TString fRootFilePath;
        TString fTreeName;
        TString fOption;
        //Flags
        bool fManSet;
        bool fUseSim;
        int fReadyState;

        TFile* fFile;
        TTree* fTree;

        //From input root file
        int fTDC[MAX_SENSE_WIRE][MAX_SAMPLING];
        int fADC[MAX_SENSE_WIRE][MAX_SAMPLING];
        int fClkHit[MAX_SENSE_WIRE][MAX_SAMPLING];
        int fTDCNhit[MAX_SENSE_WIRE];
        double fDriftTime_sim[MAX_SENSE_WIRE][MAX_SAMPLING];
        int fTriggerNumber;

};



#endif
