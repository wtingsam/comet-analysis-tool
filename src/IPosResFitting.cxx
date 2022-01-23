#include "IPosResFitting.hxx"
#include "TStyle.h"
#include <ctime>
#include "IRunTimeManager.hxx"
static double fNpripars[6][6];

void IPosResFitting::ReadPrimaryIonsTable(){
    char* INFODIR = getenv ("CCWORKING_DIR");
    FILE* fp = fopen(Form("%s/info/fitpars.txt",INFODIR), "r");
    if(!fp){
        std::cerr << "## No primary ions table, please download it !" << std::endl;
    }
    char line[128];
    int i=0;
    int j=0;
    double npripar;
    while(fgets(line, sizeof(line), fp)) {
        sscanf(line, "%d %d %lf", &i, &j, &npripar);
        fNpripars[i][j] = npripar;
    }
    fclose(fp);
}


void IPosResFitting::GetPars(double* pars, double npri){
    for (int i=0; i<=5; i++) {
        TF1*f2 = new TF1("f2", "pol5", 0, 8);
        for (int j=0; j<=5; j++) {
            f2->SetParameter(j, fNpripars[i][j]);
        }
        pars[i] = f2->Eval(npri);
        delete f2;
    }
}

double IPosResFitting::PrimaryIonsErr_Fitting(double* x, double* par){
    double pars[6];
    double npri = par[0];
    TF1*f1 = new TF1("f1", "pol5", 0, 8);
    GetPars(pars, npri);
    f1->SetParameters(pars);
    double sig = f1->Eval(x[0]);
    delete f1;
    return sig;
}

double IPosResFitting::DiffusionErr_Fitting(double* x, double* par){
    return sqrt(2*par[0]*x[0]/par[1]);
}


double IPosResFitting::DriftTimeErr_Fitting(double* x, double* par){
    // Fix here it should be time dependent.
    double v_dt = 0.025;
    double t = ICDCGas::Get().GetDriftTime(x[0]);
    v_dt = ICDCGas::Get().GetDriftVelocity(t);
    double sig_TC = par[0]; // Trigger Counter; time walk ,etc..
    double sig_RECBE = sqrt(1./12);
    double sig_dx = sqrt(sig_RECBE*sig_RECBE + sig_TC*sig_TC)*v_dt;
    return sig_dx;
}

double IPosResFitting::TrackingErr_Fiting(double* x, double* par){
    // Should simulate it before hand
    return par[0];
}

double IPosResFitting::WirePositionRes_Fitting(double* x, double* par){

    double sig_pri   = PrimaryIonsErr_Fitting(x,&par[0]);
    double sig_diff  = DiffusionErr_Fitting(x,&par[1]);
    double sig_time  = DriftTimeErr_Fitting(x,&par[3]);
    double sig_track = TrackingErr_Fiting(x,&par[4]);

    return sqrt( sig_pri*sig_pri + sig_diff*sig_diff + sig_time*sig_time + sig_track*sig_track);
    //return sqrt( sig_pri*sig_pri + sig_diff*sig_diff + sig_time*sig_time);
}

void IPosResFitting::Fit(TGraphErrors *g, TString option, Double_t min, Double_t max){
    fGraph_ResVsR=g;

    if(fGraph_ResVsR!=NULL){
        fGraph_ResVsR->Fit(fFittingFunction,"",option,min,max);
    }else{
        std::cerr << "@@@@ ERROR Null fitting graph " << std::endl;
    }
}

void IPosResFitting::Print(TString figureName){
    TCanvas *c1 = new TCanvas("c1","c1",720,512);
    //c1->cd()->SetGrid();

    TH2D* frame2 = new TH2D("h2","The composition of total spatial resolution",100,0.1,8,100,0,0.5);
    frame2 -> GetXaxis()->SetTitle("DCA[mm]");
    frame2 -> GetYaxis()->SetTitle("Total spatial resolution [mm]");
    frame2 -> GetYaxis()->SetTitleOffset(1.3);
    frame2 -> Draw();
    if(fGraph_ResVsR)fGraph_ResVsR -> Draw("same p");
    TF1* f1 = new TF1 ("f_primary_ion",PrimaryIonsErr_Fitting,0.1,8,1);
    TF1* f2 = new TF1 ("f_diffusion"  ,DiffusionErr_Fitting,0.1,8,2);
    TF1* f3 = new TF1 ("f_drifttime"  ,DriftTimeErr_Fitting,0.1,8,1);
    TF1* f4 = new TF1 ("f_tracking"   ,TrackingErr_Fiting,0.1,8,1);
    f1->SetLineStyle(8);        f1->SetLineColor(1);  f1->SetLineWidth(1);
    f2->SetLineStyle(7);        f2->SetLineColor(1);  f2->SetLineWidth(1);
    f3->SetLineStyle(3);        f3->SetLineColor(1);  f3->SetLineWidth(1);
    f4->SetLineStyle(2);        f4->SetLineColor(1);  f4->SetLineWidth(1);
    fFittingFunction->SetLineColor(1);
    f1->SetParameter(0, fFittingFunction->GetParameter(0));
    f2->SetParameter(0, fFittingFunction->GetParameter(1));
    f2->SetParameter(1, fFittingFunction->GetParameter(2));
    f3->SetParameter(0, fFittingFunction->GetParameter(3));

    f4->SetParameter(0, fFittingFunction->GetParameter(4));
    fFittingFunction->SetParNames("Primary ions"
                                  ,"Diffusion constant"
                                  ,"Drift velocity"
                                  ,"Electronics");
                                  //              ,"Extrapolation");
    f1->Draw("same");
    f2->Draw("same");
    f3->Draw("same");
    f4->Draw("same");
    fFittingFunction->Draw("same");

    TLegend* l_tmp = new TLegend(0.5,0.65,0.9,0.9);
    l_tmp -> AddEntry(fGraph_ResVsR,"Data","pl");
    l_tmp -> AddEntry(f1,"Primary ion","lp");
    l_tmp -> AddEntry(f2,"Diffusion","lp");
    l_tmp -> AddEntry(f3,"Electronics","lp");
    l_tmp -> AddEntry(f4,"Tracking","lp");
    //l_tmp -> AddEntry(f4,Form("Extrapolation Fixed (%.1fum)",fFittingFunction->GetParameter(4)),"lp");
    l_tmp -> Draw();

    gStyle->SetOptStat(0);
    char* GRAPH_DIR = getenv ("CCGRAPH_DIR");

    c1->SaveAs(Form("%s/",GRAPH_DIR)+figureName);
}

void IPosResFitting::WriteFitFunction(TString fileName){
    char *paraDIR=getenv("CCPARAMETER_DIR");
    if(paraDIR==NULL){
        fprintf(stderr,"##!! No working directory \"CCPARAMETER_DIR\" in env!");
        sprintf(paraDIR, ".");
        fprintf(stderr,"##!! Using default value:\"%s\"",paraDIR);
    }

    TFile* _file = new TFile(Form("%s/",paraDIR)+fileName,"UPDATE");

    if(!(TF1*)_file->Get(fFittingFunction->GetName())){
        fFittingFunction->Write();
    }else{
        std::cout << "## WARNING!! Found fitting function, will not write into root file" << std::endl;
        std::cout << "## " << paraDIR <<"/"<< fileName << std::endl;
        return ;
    }
    _file->Close();
    printf("## The position resolution relation has been written into root file : %s \n",fileName.Data());
}
