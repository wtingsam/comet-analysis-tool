#include "ICDCGas.hxx"
#include "TString.h"
#include <stdlib.h>
#include <math.h>
#include "RunLogManager.hxx"

ICDCGas *ICDCGas::fICDCGas = NULL;

ICDCGas &ICDCGas::Get()
{
    if ( !fICDCGas ){//make sure only being created one time
        fICDCGas = new ICDCGas();
    }
    return *fICDCGas;
}

ICDCGas::~ICDCGas(){}

void ICDCGas::Initialize(Int_t driftModel, TString xtPath)
{
    fGasType = RunLogManager::Get().GetGasName();
    NUM_OF_BOARDS_CDC = ExperimentConfig::Get().GetNumOfCDCBoard();
    SetDefaultDriftVelocity();
    fADC2ChargeFunction = new TF1("a2c","5.98739+2.6652*x+0.000573394*x*x-5.21769e-05*x*x*x+3.05897e-07*x*x*x*x-7.54057e-10*x*x*x*x*x+8.60252e-13*x*x*x*x*x*x-3.68603e-16*x*x*x*x*x*x*x",-10,800); // Katayama master thesis

    // Ultimate version of x-t relation
    MAX_SENSE_LAYER = ExperimentConfig::Get().GetNumOfLayer();
    fPhi_bin = 2;
    fBeta_bin = 10;
    for(int l=0;l<MAX_SENSE_LAYER;l++)
        for(int p=0;p<fPhi_bin;p++)
            for(int b=0;b<fBeta_bin;b++)
                fXtLayPhiBetaFunction[l][p][b]=NULL;

    fDriftModel=driftModel;
    fXtFunctionPath=xtPath;
    SetDefaultXtPath();
    InitModel();
}

void ICDCGas::Initialize()
{
    NUM_OF_BOARDS_CDC = ExperimentConfig::Get().GetNumOfCDCBoard();
    fDriftVelocity=0.023; // [mm]/[ns]
    fADC2ChargeFunction = new TF1("a2c","5.98739+2.6652*x+0.000573394*x*x-5.21769e-05*x*x*x+3.05897e-07*x*x*x*x-7.54057e-10*x*x*x*x*x+8.60252e-13*x*x*x*x*x*x-3.68603e-16*x*x*x*x*x*x*x",-10,800);// Katayama
    SetDefaultDriftVelocity();
}

void ICDCGas::ReadXtMappingList(){
    char* workingDIR = getenv ("CCWORKING_DIR");
    char* fileName = Form("%s/info/calibration-xt.txt",workingDIR);
    ifstream _fileMapFile(fileName);

    if(!_fileMapFile.is_open()){
        std::cerr << "@@@@ ERROR cannot open WireMapPath file: \"" << fileName<<"\""<< std::endl;;
        return ;
    }
    int layer(0);
    TString forXtType = "";
    std::string line="";
    int c=0;
    // Initial beta
    std::cout << fileName;
    while(getline(_fileMapFile,line)){
        std::istringstream iss(line);
        if(c>0){
            iss>>layer>>forXtType;
            fForceXtUseLayer.push_back(forXtType);
        }
        c++;
    }
    for(int i=0;i<(int)fForceXtUseLayer.size();i++){
        std::cout << "## Layer " << i << " The x-t you are going to use : " << fForceXtUseLayer.at(i) << std::endl;
    }

}

void ICDCGas::InitModel(void){
    // Choose according to model
    if(fDriftModel==0){
        SetDriftModel(kUseConstantVelocity);
        std::cout << "## Constant drift velocity is used " << std::endl;
        //}
        // else if(fDriftModel==1){
    //     std::cout << "## Averaged XT function is used " << std::endl;
    //     SetDriftModel(kUseAvgXtFunction);
    //     GetAveragedXtRelation();
    // }else if(fDriftModel==2){
    //     //do something
    //     std::cout << "## Different XT functions are used for different layers " << std::endl;
    // }else if(fDriftModel==3){
    //     //do something
    //     std::cout << "## Different XT functions are used for different layers with phi dependence" << std::endl;
    }else if(fDriftModel==4){
        //do something
        std::cout << "## Reading ICDCGas::GetLayPhiBetaXtRelation()" << std::endl;
        GetLayPhiBetaXtRelation();
    }
    //Gas gain model from hiroki, we have to update to use nakazawa's
    fADC2ChargeFunction = new TF1("a2c","5.98739+2.6652*x+0.000573394*x*x-5.21769e-05*x*x*x+3.05897e-07*x*x*x*x-7.54057e-10*x*x*x*x*x+8.60252e-13*x*x*x*x*x*x-3.68603e-16*x*x*x*x*x*x*x",-10,800);

}

void ICDCGas::SetDefaultDriftVelocity(){
    //fDriftVelocity=0.023; // [mm]/[ns]
    // if(fGasType.Contains("heic4h10")){
    //     fDriftVelocity=0.023; // [mm]/[ns]
    //     return;
    // }else if(fGasType.Contains("ethane")){
    //     fDriftVelocity=0.036; // [mm]/[ns]
    //     return;
    // }else if(fGasType.Contains("methane")){
    //     fDriftVelocity=0.020; // [mm]/[ns]
    //     return;
    // }else{
    //     fDriftVelocity=0.023;
    //     return;
    // }
}

void ICDCGas::Ls(void)
{
    switch(fDriftModel)
    {
    case 0:
        std::cout << "## Drift Model    : " << fDriftModel << " -- kUseConstantVelocity " << std::endl;
        std::cout << Form("## Value: %f [mm/ns] ",fDriftVelocity) << std::endl;
        break;
    case 1:
        std::cout << "## Drift Model    : " << fDriftModel << " -- kUseAvgXtTable " << std::endl;
        break;
    case 2:
        break;
    case 3:
        std::cout << "## Drift Model    : " << fDriftModel << " -- kUseLayPhiXtTable " << std::endl;
        break;
    case 4:
        std::cout << "## Drift Model    : " << fDriftModel << " -- kUseLayPhiBetaXtTable " << std::endl;
        break;
    case 5:
        std::cout << "## Drift Model    : " << fDriftModel << " -- kUsePhiXtTable " << std::endl;
        break;
    case 6:
        std::cout << "## Drift Model    : " << fDriftModel << " -- kUsePhiBetaXtTable " << std::endl;
        break;
    }
}
void ICDCGas::SetDefaultXtPath(){
    char* WORKDIR = getenv ("CCWORKING_DIR");
    if(fGasType.Contains("heic4h10")){
        TString path = Form("%s/info/xt_HeiC4H10_9010_1800V.default.root",WORKDIR);
        fXtFunctionDefaultPath = path;
    }else if(fGasType.Contains("ethane")){
        return;
    }else if(fGasType.Contains("methane")){
        return;
    }else{
        std::cerr << "@@@@ Error No default x-t relation for this gas" <<std::endl;
        return;
    }
}
// void ICDCGas::GetDefaultXtRelationFunction(void){
//     char* WORKDIR = getenv ("CCWORKING_DIR");
//     // Default x-t relation
//     std::cout << "## The gas type is " << fGasType << std::endl;
//     if(fGasType.Contains("heic4h10")){
//         TString path = Form("%s/info/xt_HeiC4H10_9010_1800V.default.root",WORKDIR);
//         //TString path = Form("%s/info/xt.1012.syml4.i0.slz0p3.root",WORKDIR);
//         //TString path = Form("%s/info/xt.1012.syml4.i1.t7.root",WORKDIR);
//         fXtDefault = new TFile(path);
//         std::cout << "## Reading xt: " << path <<std::endl;
//         return;
//     }else if(fGasType.Contains("ethane")){
//         //TString path = Form("%s/info/xt_HeC2H6_5050_2300V.default.root",WORKDIR);
//         TString path = Form("%s/info/xt.117.syml4.i2.root",WORKDIR);
//         fXtDefault = new TFile(path);
//         std::cout << "## Reading xt: " << path <<std::endl;
//         return;
//     }else if(fGasType.Contains("methane")){
//         TString path = Form("%s/info/xt_HeCH4_7030_2000V.default.root",WORKDIR);
//         fXtDefault = new TFile(path);
//         std::cout << "## Reading xt: " << path <<std::endl;
//         return;
//     }else{
//         std::cerr << "@@@@ Error No default x-t relation for this gas" <<std::endl;
//         return;
//     }
//     if(!fXtDefault->IsOpen()){
//         std::cerr << "@@@@ Error cannot open default x-t relation" << std::endl;
//         return ;
//     }
// }

void ICDCGas::GetDefaultXtRelation(){
    // Read x-t relation
    TFile *xt_file = new TFile(fXtFunctionDefaultPath);
    if(xt_file->IsOpen()){
        std::cout << "## Reading ICDCGas::GetDefaultXtRelation()  x-t relations " << std::endl;
        for(int l=0;l<MAX_SENSE_LAYER;l++)
            for(int p=0;p<fPhi_bin;p++)
                for(int b=0;b<fBeta_bin;b++){
                    fXtLayPhiBetaFunction[l][p][b]=(TF1*)xt_file->Get(Form("avg_xt_%d",p));
                }
    }else{
        std::cerr << "    @@@@ Unknown error on reading x-t files, no default and no x-t" <<std::endl;
    }
}

void ICDCGas::GetAveragedXtRelation(){
    // Read x-t relation
    if(fXtDefault->IsOpen()){
        std::cout << "## Reading GetAveragedXtRelation() Default x-t relations " << std::endl;
        for(int l=0;l<MAX_SENSE_LAYER;l++)
            for(int p=0;p<fPhi_bin;p++)
                for(int b=0;b<fBeta_bin;b++){
                    fXtLayPhiBetaFunction[l][p][b]=(TF1*)fXtDefault->Get(Form("avg_xt_%d",p));
                }
    }else{
        std::cerr << "    @@@@ Unknown error on reading x-t files, no default and no x-t" <<std::endl;
    }
}

void ICDCGas::GetLayPhiBetaXtRelation(){
    // Read x-t relation
    TFile * a_xt_file = new TFile(fXtFunctionPath);
    // Check open of the file
    if(a_xt_file->IsOpen()){
        std::cout << "## Reading root file for x-t relation " << fXtFunctionPath<< std::endl;
        // Initialize all x-t relations
        for(int l=0;l<MAX_SENSE_LAYER;l++){
            for(int p=0;p<fPhi_bin;p++){
                for(int b=0;b<fBeta_bin;b++){
                    // Read avg xt
                    if(fForceXtUseLayer.at(l).Contains("avg")){
                        if(a_xt_file->Get(Form("avg_xt_%d",p))){
                            fXtLayPhiBetaFunction[l][p][b]=(TF1*)a_xt_file->Get(Form("avg_xt_%d",p));
                        }
                    }
                    // Read layer dependent xt
                    if(fForceXtUseLayer.at(l).Contains("layer")){
                        if(a_xt_file->Get(Form("xt_%d_l%d",p,l))){
                            fXtLayPhiBetaFunction[l][p][b]=(TF1*)a_xt_file->Get(Form("xt_%d_l%d",p,l));
                        }
                    }
                    // Read beta dependent xt
                    if(fForceXtUseLayer.at(l).Contains("beta")){
                        if(a_xt_file->Get(Form("xt_l%d_p%d_b%d",l,p,b))){
                            fXtLayPhiBetaFunction[l][p][b]=(TF1*)a_xt_file->Get(Form("xt_l%d_p%d_b%d",l,p,b));
                        }
                    }
                }
            }
        }
    }else{
        // If no file read default x-t
        GetDefaultXtRelation();
    }
    if(!EnsureAllXt()){
        std::cerr << "    ERROR!!! cannot read x-t relations from: "<< fXtFunctionPath << std::endl;
    }
}

bool ICDCGas::EnsureAllXt(){
    // Check the existance of x-t function , if not then read avg_xt x-t at that iteration step
    // if not again read the deafult one
    // else no x-t
    int count_xt=0;
    int count_xt_ok=0;
    for(int l=0;l<MAX_SENSE_LAYER;l++)
        for(int p=0;p<fPhi_bin;p++)
            for(int b=0;b<fBeta_bin;b++){
                if(fXtLayPhiBetaFunction[l][p][b]!=NULL){
                    count_xt_ok++;
                }
                count_xt++;
            }
    printf("## EnsureAllXt Number of successful x-t %d/%d \n",count_xt_ok,count_xt);
    if(count_xt_ok==count_xt)
        return true;
    else
        return false;
}

Double_t ICDCGas::GetDriftDistance(Double_t driftT, Int_t layer, Int_t wire, Double_t phi, Double_t beta)
{
    if(driftT<=0)return 0;
    double      max_beta   = WireManager::Get().GetMaxBeta(layer,wire);
    double      min_beta   = WireManager::Get().GetMinBeta(layer,wire);
    double      beta_range = max_beta-min_beta;
    Int_t       iBeta      = (int)(beta - abs(min_beta))/beta_range*fBeta_bin;
    if(iBeta==fBeta_bin)iBeta=fBeta_bin-1;
    // for now, just consider left and right..
    int iPhi = -1;
    if(phi>=90 && phi<270)
        iPhi=0;
    else
        iPhi=1;
    // ---------
    // For now
    // ---------
    //printf("check: dt %f LW [%d][%d] Phi [%d] beta [%d] phi %f beta %f beta_range %f\n",driftT,layer,wire,iPhi,iBeta,phi,beta,beta_range);
    iBeta = 0;
    if(fDriftModel==0){
        if(phi>90 && phi<=270){
            return -1*driftT*fDriftVelocity;
        }else{
            return driftT*fDriftVelocity;
        }
    }else{
        return fXtLayPhiBetaFunction[layer][iPhi][iBeta]->Eval(driftT);
    }
    return -1 ;
}


Double_t ICDCGas::GetGasGain(Double_t q, Double_t *trackPar)
{
    Double_t ll=16;
    Double_t NPAIR=60;
    ll=sqrt(16*16 + pow(16*trackPar[0],2));
    return fADC2ChargeFunction->Eval(q)*(1e-15)/(1.6e-19*NPAIR*ll/10);
}
