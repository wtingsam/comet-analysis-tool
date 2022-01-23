#ifndef __TH2DBUILDER_HXX_
#define __TH2DBUILDER_HXX_
#include <string>
#include <iostream>
#include <vector>

#include "TCanvas.h"
#include "TH2D.h"
#include "TLegend.h"

class TH2DBuilder{
    public:
        /// Constructor
        TH2DBuilder(){}
        TH2DBuilder(char* head, int listSize)
            :fHeader(head),fName(""),fTitle(""),fXTitle(""),fYTitle(""),
             fSize(listSize), fCanvas(NULL)
            {
                fLeg = new TLegend(0.7,0.65,0.9,0.9);
                if(listSize/10!=0)
                    fLeg->SetNColumns(listSize/10);
            }
        /// Make ONE histogram
        TH2D* MakeTH2D(TString name, TString title, int binX, int xmin, int xmax, int binY, int ymin, int ymax, TString xTitle, TString yTitle);

        /// Make a lot of histograms depending on size
        void MakeTH2Ds(TString name, TString title, int binX, int xmin, int xmax, int binY, int ymin, int ymax, TString xTitle, TString yTitle);

        TH2D* GetHisto(int i){      return fHisto[i];   }

        template < typename T1, typename T2>
        void Fill(int i, T1 element_x, T2 element_y){
            fHisto[i]->Fill(element_x, element_y);
        }

        void Draw(TString option="",bool logX=false, bool logY=false, bool logZ=false);
        void Draw(int i,TString option="", bool logX=false, bool logY=false, bool logZ=false);

        TString GetHeader(){    return fHeader;         }

        TString GetName(int i){    if(fHisto[i]!=NULL)  return fHisto[i]->GetName();    return "";      }

        TString GetTitle(int i){    if(fHisto[i]!=NULL)         return fHisto[i]->GetTitle();   return "";      }

        void Delete(){      for(int i=0;i<fSize;i++) delete fHisto[i];  }

    private:
        TString fHeader;

        TString fName;
        TString fTitle;
        TString fXTitle;
        TString fYTitle;

        Int_t fBinX;
        Int_t fMinX;
        Int_t fMaxX;
        Int_t fBinY;
        Int_t fMinY;
        Int_t fMaxY;
        Int_t fSize;
        TH2D *fHisto[5000]; // TODO
        TCanvas *fCanvas;
        TLegend *fLeg;
};
#endif
