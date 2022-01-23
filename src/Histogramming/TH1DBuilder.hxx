#ifndef __TH1DBUILDER_HXX_
#define __TH1DBUILDER_HXX_
#include <string>
#include <iostream>
#include <vector>

#include "TCanvas.h"
#include "TH1D.h"
#include "TLegend.h"

class TH1DBuilder{
    private:
        bool fUseFillColorFlag;
        bool fUseLineColorFlag;
        Int_t fForceColor;
        TString fHeader;

        TString fName;
        TString fTitle;
        TString fXTitle;

        Int_t fBinX;
        Int_t fMinX;
        Int_t fMaxX;
        Int_t fColor;
        Int_t fFillColor;
        Int_t fSize;
        TH1D *fHisto[5000]; // TODO
        TCanvas *fCanvas;
        TLegend *fLeg;
    public:
        /// Constructor
        TH1DBuilder(){}
        TH1DBuilder(char* head, int listSize, bool isLine=false, bool isFill=false)
            :fUseFillColorFlag(isFill), fUseLineColorFlag(isLine),
             fForceColor(-1),
             fHeader(head),fName(""),fTitle(""),fXTitle(""),
             fSize(listSize), fCanvas(NULL)
            {
                fLeg = new TLegend(0.7,0.65,0.9,0.9);
                if(listSize/10!=0)
                    fLeg->SetNColumns(listSize/10);
            }
        /// Make ONE histogram
        TH1D* MakeTH1D(TString name, TString title,  int binX, int xmin, int xmax, TString xTitle, int color, int fillColor=0);

        /// Make
        void MakeTH1Ds(TString name, TString title, int binX, int xmin, int xmax, TString xTitle, int color, int fillColor=0);

        TH1D* GetHisto(int i){      return fHisto[i];   }

        void ForceColor(int i){   fForceColor = i;  }

        template < typename T>
        void Fill(int i, T element){
            fHisto[i]->Fill(element);
        }

        void Draw(TString option="",bool logX=false, bool logY=false, bool logZ=false);
        void Draw(int i,TString option="", bool logX=false, bool logY=false, bool logZ=false);
        void DrawCombine(TString option="",bool logX=false, bool logY=false, bool logZ=false, bool drawLeg=true);

        TString GetHeader(){    return fHeader;         }

        TString GetName(int i){    if(fHisto[i]!=NULL)  return fHisto[i]->GetName();    return "";      }

        TString GetTitle(int i){    if(fHisto[i]!=NULL)         return fHisto[i]->GetTitle();   return "";      }

        void Delete(){      for(int i=0;i<fSize;i++) delete fHisto[i];  }

};
#endif
