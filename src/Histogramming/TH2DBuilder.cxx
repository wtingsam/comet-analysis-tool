#include "TH2DBuilder.hxx"
#include "TStyle.h"

TH2D* TH2DBuilder::MakeTH2D(TString name, TString title, int binX, int xmin, int xmax, int binY, int ymin, int ymax, TString xTitle, TString yTitle)
{
    TH2D* h = new TH2D(name,title,binX,xmin,xmax,binY,ymin,ymax);
    h -> GetXaxis()->SetTitle(xTitle);
    h -> GetYaxis()->SetTitle(yTitle);
    h -> GetYaxis()->SetTitleOffset(1.5);
    return h;
}

void TH2DBuilder::MakeTH2Ds(TString name, TString title, int binX, int xmin, int xmax, int binY, int ymin, int ymax, TString xTitle, TString yTitle)
{
    fName      = name;
    fTitle     = title;
    fBinX      = binX;
    fMinX      = xmin;
    fMaxX      = xmax;
    fBinY      = binY;
    fMinY      = ymin;
    fMaxY      = ymax;
    fXTitle    = xTitle;
    fYTitle    = yTitle;

    for(int i=0;i<fSize;i++){
        TString nn=fName+Form("_%d",i);
        TString tt=fTitle+Form("_%d",i);
        fHisto[i] = MakeTH2D(nn,tt,fBinX,fMinX,fMaxX,fBinY,fMinY,fMaxY,fXTitle,fYTitle);
    }

}

void TH2DBuilder::Draw(int i, TString option, bool logX, bool logY, bool logZ){
    char *DIR = getenv ("CCGRAPH_DIR");
    fCanvas = new TCanvas(fHeader+"1",fHeader+"1",720,512);
    fCanvas -> SetGrid();
    if(logX) fCanvas -> SetLogx();
    if(logY) fCanvas -> SetLogy();
    if(logZ) fCanvas -> SetLogz();
    fHisto[i]->Draw("colz"+option);
    TString fileNamePDF = Form("%s/",DIR) + fHeader + Form(".%d.pdf",i) ;
    fCanvas->SaveAs(fileNamePDF);
    TString fileNamePNG = Form("%s/",DIR) + fHeader + Form(".%d.png",i) ;
    fCanvas->SaveAs(fileNamePNG);
    fCanvas->Close();
}

void TH2DBuilder::Draw(TString option, bool logX, bool logY, bool logZ){
    char *DIR = getenv ("CCGRAPH_DIR");
    fCanvas = new TCanvas(fHeader+"2",fHeader+"2",720,512);
    fCanvas -> SetGrid();

    int DividedSize = sqrt(fSize) + 1;
    fCanvas->Divide(DividedSize,DividedSize);
    for(int i=0;i<fSize;i++){
        if(logX) fCanvas -> cd(i+1) -> SetLogx();
        if(logY) fCanvas -> cd(i+1) -> SetLogy();
        if(logZ) fCanvas -> cd(i+1) -> SetLogz();
        fCanvas->cd(i+1);
        fHisto[i]->Draw("colz"+option);
    }
    TString fileNamePDF = Form("%s/",DIR) + fHeader + ".pdf" ;
    fCanvas->SaveAs(fileNamePDF);
    TString fileNamePNG = Form("%s/",DIR) + fHeader + ".png" ;
    fCanvas->SaveAs(fileNamePNG);
    fCanvas->Close();
}
