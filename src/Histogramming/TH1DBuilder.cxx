#include "TH1DBuilder.hxx"
#include "TStyle.h"

TH1D* TH1DBuilder::MakeTH1D(TString name, TString title,
                            int binX, int xmin, int xmax,
                            TString xTitle, int color, int fillColor)
{
    TH1D* h = new TH1D(name,title,binX,xmin,xmax);
    h -> GetXaxis()->SetTitle(xTitle);
    h -> SetLineColor(color);
    if(fForceColor==-1){
        h -> SetFillColor(fillColor);
    }else{
        h -> SetFillColor(fForceColor);
    }
    if(fUseFillColorFlag)h -> SetFillStyle(3001);
    else h -> SetFillStyle(0);
    return h;
}

void TH1DBuilder::MakeTH1Ds(
    TString name, TString title,
    int binX, int xmin, int xmax,
    TString xTitle, int color, int fillColor){
    fName      = name;
    fTitle     = title;
    fBinX      = binX;
    fMinX      = xmin;
    fMaxX      = xmax;
    fXTitle    = xTitle;
    fColor     = color;
    fFillColor = fillColor;

    for(int i=0;i<fSize;i++){
        TString nn=fName+Form("_%d",i);
        TString tt=fTitle+Form("_%d",i);

        int fillColor=0;
        int lineColor=0;
        if(fUseFillColorFlag){    fillColor = fFillColor+i;   }
        if(fUseLineColorFlag){
            lineColor = fColor+i;
            if(lineColor==10 || lineColor==0)
                lineColor++;
        }
        if(fForceColor!=-1){
            lineColor=fForceColor;
        }

        fHisto[i] = MakeTH1D(nn,tt,fBinX,fMinX,fMaxX,fXTitle,lineColor,fillColor);
//      std::cout << fHeader << ":" << i << " Created " << fHisto[i] << std::endl;
    }

}

void TH1DBuilder::Draw(int i, TString option, bool logX, bool logY, bool logZ){
    char *DIR = getenv ("CCGRAPH_DIR");
    fCanvas = new TCanvas(fHeader+"1",fHeader+"1",720,512);
    fCanvas -> SetGrid();
    if(logX) fCanvas -> SetLogx();
    if(logY) fCanvas -> SetLogy();
    if(logZ) fCanvas -> SetLogz();

    fHisto[i]->Draw(option);
    TString fileNamePDF = Form("%s/",DIR) + fHeader + Form(".%d.pdf",i) ;
    fCanvas->SaveAs(fileNamePDF);
    TString fileNamePNG = Form("%s/",DIR) + fHeader + Form(".%d.png",i) ;
    fCanvas->SaveAs(fileNamePNG);
    fCanvas->Close();
}

void TH1DBuilder::Draw(TString option, bool logX, bool logY, bool logZ){
    char *DIR = getenv ("CCGRAPH_DIR");
    fCanvas = new TCanvas(fHeader+"2",fHeader+"2",720,512);
    fCanvas -> SetGrid();
    int DividedSize = sqrt(fSize) + 1;
    fCanvas->Divide(DividedSize,DividedSize);
    for(int i=0;i<fSize;i++){
        fCanvas->cd(i+1);
        if(logX) fCanvas -> cd(i+1) -> SetLogx();
        if(logY) fCanvas -> cd(i+1) -> SetLogy();
        if(logZ) fCanvas -> cd(i+1) -> SetLogz();
        fHisto[i]->Draw(option);
    }
    TString fileNamePDF = Form("%s/",DIR) + fHeader + ".pdf" ;
    fCanvas->SaveAs(fileNamePDF);
    TString fileNamePNG = Form("%s/",DIR) + fHeader + ".png" ;
    fCanvas->SaveAs(fileNamePNG);
    fCanvas->Close();
}

void TH1DBuilder::DrawCombine(TString option, bool logX, bool logY, bool logZ, bool drawLeg){
    char *DIR = getenv ("CCGRAPH_DIR");
    fCanvas = new TCanvas(fHeader+"3",fHeader+"3",720,512);
    fCanvas -> SetGrid();
    fCanvas -> SetGrid();
    gStyle -> SetOptStat(0);
    if(logX) fCanvas -> SetLogx();
    if(logY) fCanvas -> SetLogy();
    if(logZ) fCanvas -> SetLogz();

    for(int i=0;i<fSize;i++){
        fLeg -> AddEntry(fHisto[i],Form("%d",i),"lp");
        if(i==0)
            fHisto[i]->Draw(option);
        else{
            double scaleF=fHisto[0]->GetEntries()*1./fHisto[i]->GetEntries();
            fHisto[i]->Scale(scaleF);
            fHisto[i]->Draw("same");
        }
    }
    if(drawLeg)fLeg->Draw();
    TString fileNamePDF = Form("%s/",DIR) + fHeader + "combined.pdf" ;
    fCanvas->SaveAs(fileNamePDF);
    TString fileNamePNG = Form("%s/",DIR) + fHeader + "combined.png" ;
    fCanvas->SaveAs(fileNamePNG);
    fCanvas->Close();
}
