#include "TFile.h"
#include <sstream>
#include <fstream>
#include "WireConfig.hxx"
#include "ExperimentConfig.hxx"
#include <stdlib.h>

WireConfig::WireConfig(){}
WireConfig::~WireConfig(){}

void WireConfig::Initialize(double rotate, int debug)
{
    fDebug = debug;
    fAngle = rotate;

    TRIG_CH         = ExperimentConfig::Get().GetNumOfTrigChannel();
    MAX_CH          = ExperimentConfig::Get().GetNumOfChannel();
    MAX_WIREpL      = ExperimentConfig::Get().GetNumOfWirePerLayer();
    MAX_SENSE_LAYER = ExperimentConfig::Get().GetNumOfLayer();
    MAX_LAYER       = ExperimentConfig::Get().GetMaxLayers();
    MAX_WIREID      = ExperimentConfig::Get().GetMaxWireID();

    //1D array
    fCDClength            = (double*)malloc(sizeof(double)*MAX_SENSE_LAYER);
    fCDClength_all        = (double*)malloc(sizeof(double)*MAX_LAYER);
    fLayerUsed            = (int*)malloc(sizeof(int)*MAX_CH);
    fCellUsed             = (int*)malloc(sizeof(int)*MAX_CH);
    fBdIDUsed             = (int*)malloc(sizeof(int)*MAX_CH);
    fBdLayIDUsed          = (int*)malloc(sizeof(int)*MAX_CH);
    fBdLocIDUsed          = (int*)malloc(sizeof(int)*MAX_CH);
    fNumSenseWireUsed     = (int*)malloc(sizeof(int)*MAX_SENSE_LAYER);
    fMaxSenseWirePerLayer = (int*)malloc(sizeof(int)*MAX_SENSE_LAYER);
    //2D array

    fChannelUsed = (int**)malloc(sizeof(int*)*MAX_SENSE_LAYER);

    fXhv = (double**)malloc(sizeof(double*)*MAX_SENSE_LAYER);
    fYhv = (double**)malloc(sizeof(double*)*MAX_SENSE_LAYER);
    fXro = (double**)malloc(sizeof(double*)*MAX_SENSE_LAYER);
    fYro = (double**)malloc(sizeof(double*)*MAX_SENSE_LAYER);
    fXc  = (double**)malloc(sizeof(double*)*MAX_SENSE_LAYER);
    fYc  = (double**)malloc(sizeof(double*)*MAX_SENSE_LAYER);

    for(int i=0;i<MAX_SENSE_LAYER;i++){
        fChannelUsed[i] = (int*)malloc(sizeof(int)*MAX_WIREpL);

        fXhv[i]         = (double*)malloc(sizeof(double)*MAX_WIREpL);
        fYhv[i]         = (double*)malloc(sizeof(double)*MAX_WIREpL);
        fXro[i]         = (double*)malloc(sizeof(double)*MAX_WIREpL);
        fYro[i]         = (double*)malloc(sizeof(double)*MAX_WIREpL);
        fXc[i]  = (double*)malloc(sizeof(double)*MAX_WIREpL);
        fYc[i]  = (double*)malloc(sizeof(double)*MAX_WIREpL);
    }

    fXhv_all = (double**)malloc(sizeof(double*)*MAX_LAYER);
    fYhv_all = (double**)malloc(sizeof(double*)*MAX_LAYER);
    fXro_all = (double**)malloc(sizeof(double*)*MAX_LAYER);
    fYro_all = (double**)malloc(sizeof(double*)*MAX_LAYER);

    for(int i=0;i<MAX_LAYER;i++){
        fXhv_all[i] = (double*)malloc(sizeof(double)*MAX_WIREID);
        fYhv_all[i] = (double*)malloc(sizeof(double)*MAX_WIREID);
        fXro_all[i] = (double*)malloc(sizeof(double)*MAX_WIREID);
        fYro_all[i] = (double*)malloc(sizeof(double)*MAX_WIREID);
    }

    TString wiremap = ExperimentConfig::Get().GetWireMapFile();
    SetPath(wiremap);
    for(int i=0;i<MAX_CH;i++){
        fLayerUsed[i]   = -1;
        fCellUsed[i]    = -1;
        fBdIDUsed[i]    = -1;
        fBdLayIDUsed[i] = -1;
        fBdLocIDUsed[i] = -1;
    }
    for(int l=0;l<MAX_SENSE_LAYER;l++){
        for(int w=0;w<MAX_WIREpL;w++){
            fChannelUsed[l][w]=-1;
        }
        fCDClength[l] = -1;
        fCDClength_all[l] = -1;
        fNumSenseWireUsed[l] = 0;
        fMaxSenseWirePerLayer[l] = 0;
    }
    fNumSenseWire        = 0;
    fNumFieldWire        = 0;
    ReadWireMap();
    //GenBetaZMapping();
}

void WireConfig::CheckPath(void){

    if(fWireMapPath!=""){
        std::cout << "## "<< fWireMapPath << std::endl;
    }else{
        std::cerr << "Did not set Path" << std::endl;
    }
}  //for debug

void WireConfig::GenBetaZMapping(void)
{

    TString fileName = ExperimentConfig::Get().GetBetaZFilePath();
    ifstream betaZMapFile(fileName);

    if(!betaZMapFile.is_open()){
        std::cerr << "## ERROR cannot open WireMapPath file: \"" << fileName<<"\""<< std::endl;;
        return ;
    }
    int index(0),layer(0),cell(0), period(0);
    double beta(0), beta_deg(0), dBeta(0),z(0);
    std::string line;
    int c=0;
    // Initial beta
    fMaxBinOfBetaZ=-1e5;
    for(int iLayer=0;iLayer<MAX_SENSE_LAYER;iLayer++)
        for(int iCell=0;iCell<fMaxSenseWirePerLayer[iLayer];iCell++)
            for(int i=0;i<500;i++)
                fBeta[iLayer][iCell][i]=-1;

    while(getline(betaZMapFile,line)){
        std::istringstream iss(line);
        if(c>0){
            iss>>index>>layer>>cell>>beta>>beta_deg>>dBeta>>period>>z;

            fBeta[layer][cell][index]=beta_deg;
            fZ[layer][cell][index]=z;
            if(index>fMaxBinOfBetaZ)
                fMaxBinOfBetaZ=index;
        }
        c++;
    }
    fMaxBinOfBetaZ++;

    // Get Max and min Beta
    for(int iLayer=0;iLayer<MAX_SENSE_LAYER;iLayer++){
        for(int iCell=0;iCell<fMaxSenseWirePerLayer[iLayer];iCell++){
            double max_beta=-1e5;
            double min_beta= 1e5;
            for(int i=0;i<fMaxBinOfBetaZ;i++){
                beta_deg=fBeta[iLayer][iCell][i];
                if(beta_deg>max_beta){
                    max_beta = beta_deg;
                    fBeta_max[iLayer][iCell] = max_beta;
                }
                if(beta_deg<min_beta){
                    min_beta = beta_deg;
                    fBeta_min[iLayer][iCell] = min_beta;
                }
            }
        }
    }
    // Checking
//     layer=10;
//     cell=5;
//     for(int i=0;i<fMaxBinOfBetaZ;i++){
//      printf("[%d][%d][%d] %f %f %f \n",layer,cell,i,fBeta[layer][cell][i], fBeta_max[layer][cell], fBeta_min[layer][cell]);
//     }
}

void WireConfig::ReadWireMap(void)
{
    ifstream wireMapFile(fWireMapPath);
    if(!wireMapFile.is_open()){
        std::cerr << "## ERROR cannot open WireMapPath file: \"" << fWireMapPath <<"\""<< std::endl;;
        return ;
    }

    Double_t LayerLength,xhv,yhv,x0,y0,xro,yro,layer,wire,LayerID,isSenseWire,CellID,BoardID,BrdLayID,BrdLocID,ChanID;
    std::string line;
    std::vector<int> bdid;
    std::vector<int> hwbdid;
    ExperimentConfig::Get().GetExpBoardConfig(&bdid,&hwbdid);
    int count = 0;
    while(getline(wireMapFile,line)){
        std::istringstream iss(line);
        iss>>LayerLength>>layer>>wire>>xhv>>yhv>>x0>>y0>>xro>>yro>>LayerID>>isSenseWire>>CellID>>BoardID>>BrdLayID>>BrdLocID>>ChanID;

        if(count>0){
            fCDClength_all[(int)layer]=LayerLength;
            Rotation(xhv,yhv,xhv,yhv,fAngle);
            Rotation(xro,yro,xro,yro,fAngle);
            Rotation(x0,y0,x0,y0,fAngle);

            fXhv_all[(int)layer][(int)wire]=xhv;
            fYhv_all[(int)layer][(int)wire]=yhv;
            fXro_all[(int)layer][(int)wire]=xro;
            fYro_all[(int)layer][(int)wire]=yro;

            if(isSenseWire>0){
                int l_tmp = (int)LayerID;
                int c_tmp = (int)CellID;
                //Length of the CDC
                fMaxSenseWirePerLayer[l_tmp]++;
                fCDClength[l_tmp]=LayerLength;
                fNumSenseWire++;

                fXhv[l_tmp][c_tmp]=xhv;
                fYhv[l_tmp][c_tmp]=yhv;
                fXro[l_tmp][c_tmp]=xro;
                fYro[l_tmp][c_tmp]=yro;
                fXc[l_tmp][c_tmp]=x0;
                fYc[l_tmp][c_tmp]=y0;

                int isGet=0;
                int index=-1;

                for(int i=0;i<(int)bdid.size();i++){
                    if(bdid.at(i)==(Int_t)BoardID){
                        isGet=1;
                        fNumSenseWireUsed[l_tmp]++;
                        index=i;
                        break;
                    }
                }

                if(isGet){
                    Int_t ch;
                    Int_t hwid = hwbdid.at(index);
                    Int_t ll=(Int_t)LayerID;
                    Int_t cc=(Int_t)CellID;

                    ch = (Int_t)ChanID;

                    fLayerUsed[ch+48*(hwid-1)] = ll;
                    fCellUsed[ch+48*(hwid-1)]  = cc;

                    // //layer wire -> ch
                    //fChannelUsed[ll][cc]=ch+48*(hwid-1);
                    fChannelUsed[ll][cc]=ch+48*(hwid-1);
                    fBdIDUsed[ch+48*(hwid-1)] = (Int_t)BoardID;
                    fBdLayIDUsed[ch+48*(hwid-1)] = (Int_t)BrdLayID;
                    fBdLocIDUsed[ch+48*(hwid-1)] = (Int_t)BrdLocID;
                    //printf("ch+48*(hwid-1), %d+48*(%d-1)=%d [%d][%d]\n",ch,hwid,ch+48*(hwid-1),ll,cc);
                }
            }else{
                fNumFieldWire++;
            }
        }
        count++;
    }
}
