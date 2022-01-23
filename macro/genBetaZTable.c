#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include "TTimer.h"
#include "TSystem.h"
#include "TLatex.h"
#include "iostream"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH2D.h"
#include "TLine.h"
#include "TStyle.h"
#include "TGraph.h"
#include "TVector2.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <exception>
//
// This macro will draw the sum fo ADC distribution
// The arrangement is according to each RECBE board
//
// Aurthor: Sam Wong
//
ofstream betaTxtFile;
double pre_beta;
double min_ang;
int period=0;

#define num_of_layers 39
#define num_of_sense_layers 20
#define num_of_wires 612

double xf_hv[num_of_layers][num_of_wires];
double yf_hv[num_of_layers][num_of_wires];
double zf_hv[num_of_layers][num_of_wires];
double xf_ro[num_of_layers][num_of_wires];
double yf_ro[num_of_layers][num_of_wires];
double zf_ro[num_of_layers][num_of_wires];
int max_wirePerLayer[num_of_layers];
int max_senseWirePerLayer[num_of_sense_layers];

TTree* t;
bool isOpen =false;
TFile *_file0;
TTree* tree;

// graph
TGraph *g_cell;
TLine *line[8];
TCanvas *c;
TH2D *frame_cdc;

bool ReadWireMapRootFile(char* infoName)
{
    _file0 = TFile::Open(infoName);
    if(_file0==NULL){
        std::cerr << "## No mapping file" << std::endl;
        return false;
    }
    tree = (TTree*)_file0->Get("t");
    Double_t LayerLength,xhv,yhv,x0,y0,xro,yro,layer,wire,LayerID,isSenseWire,CellID,BoardID,BrdLayID,BrdLocID,ChanID;
    tree->SetBranchAddress("LayerLength",&LayerLength);
    tree->SetBranchAddress("layer",&layer);
    tree->SetBranchAddress("wire",&wire);
    tree->SetBranchAddress("xhv",&xhv);
    tree->SetBranchAddress("yhv",&yhv);
    tree->SetBranchAddress("xro",&xro);
    tree->SetBranchAddress("yro",&yro);
    tree->SetBranchAddress("LayerID",&LayerID);
    tree->SetBranchAddress("isSenseWire",&isSenseWire);
    tree->SetBranchAddress("CellID",&CellID);
    tree->SetBranchAddress("ChanID", &ChanID);

    for(int i=0;i<num_of_sense_layers;i++){
        max_senseWirePerLayer[i]=0;
    }
    for(int i=0;i<tree->GetEntries();i++){
        tree->GetEntry(i);

        xf_hv[(int)layer][(int)wire] = xhv;
        yf_hv[(int)layer][(int)wire] = yhv;
        zf_hv[(int)layer][(int)wire] = -LayerLength/2;
        xf_ro[(int)layer][(int)wire] = xro;
        yf_ro[(int)layer][(int)wire] = yro;
        zf_ro[(int)layer][(int)wire] = LayerLength/2;
        max_wirePerLayer[(int)layer]++;

        if((int)isSenseWire==1)
            max_senseWirePerLayer[(int)LayerID]++;
    }
    return true;
}

void getZ(int layer,int wire, double z, double &x, double &y){
    double x1=xf_hv[layer][wire];
    double y1=yf_hv[layer][wire];
    double z1=zf_hv[layer][wire];
    double x2=xf_ro[layer][wire];
    double y2=yf_ro[layer][wire];
    double z2=zf_ro[layer][wire];

    double t = (z1-z)/(z1-z2);
    x = x1-(x1-x2)*t;
    y = y1-(y1-y2)*t;
}

void rotate(double x,double y, double &xx, double& yy, double ang)
{
    xx= x*cos(ang)+y*sin(ang);
    yy=-x*sin(ang)+y*cos(ang);
}

void translate(double x,double y, double &xx, double &yy){
    xx -= x;
    yy -= y;
}

void printD(const std::vector<double> & v, const char * msg)
{
    int size = v.size();
    for (int i = 0; i < size; ++i)
        cout << v[i] << " ";
    cout << msg << endl;
}

void printI(const std::vector<int> & v, const char * msg)
{
    int size = v.size();
    for (int i = 0; i < size; ++i)
        cout << v[i] << " ";
    cout << msg << endl;
}

struct MyComparator
{
        const std::vector<double> & value_vector;

        MyComparator(const std::vector<double> & val_vec):
            value_vector(val_vec) {}

        bool operator()(int i1, int i2)
            {
                return value_vector[i1] < value_vector[i2];
            }
};

// option 1:drawing
// 0: no drawing
void plotSense(int layer, int wire, double z, char *option){
    double sense_x, sense_y;
    getZ(layer,wire,z,sense_x,sense_y);
    double x2,y2,dr;
    int w_index[3][3]={0};
    // No guard layer
    if(layer==0 || layer==38){
        return ;
    }

    for(int l=0;l<3;l++){
        double minR = 1e5;
        // Scan the wires on upper and lower layer to get the closest wire
        for(int w=0;w<max_wirePerLayer[layer-1+l];w++){
            double dr,x2,y2;
            getZ(layer-1+l,w,z,x2,y2);
            dr = sqrt((y2-sense_y)*(y2-sense_y)+(x2-sense_x)*(x2-sense_x));
            if(dr==0) continue;
            if(dr < minR){
                // Index the wire with smallest distance between the sense wire and others
                w_index[l][0]=w;
                minR = dr;
            }
        }
        //printf("[%d][%d] %f \n",l, w_index[l][0],minR);
        minR = 1e5;
        // Scan again to get the second smallest
        for(int w=0;w<max_wirePerLayer[layer-1+l];w++){
            double dr,x2,y2;
            getZ(layer-1+l,w,z,x2,y2);
            dr = sqrt((y2-sense_y)*(y2-sense_y)+(x2-sense_x)*(x2-sense_x));
            if(dr==0) continue;
            if(dr < minR){
                int tmp_w = w;
                if(tmp_w==w_index[l][0])continue;
                w_index[l][1]=tmp_w;
                minR = dr;
            }
        }
        minR = 1e5;
        // Scan again to get the third smallest
        for(int w=0;w<max_wirePerLayer[layer-1+l];w++){
            double dr,x2,y2;
            getZ(layer-1+l,w,z,x2,y2);
            dr = sqrt((y2-sense_y)*(y2-sense_y)+(x2-sense_x)*(x2-sense_x));
            if(dr==0) continue;
            if(dr < minR){
                int tmp_w = w;
                if(tmp_w==w_index[l][0] || tmp_w==w_index[l][1])continue;
                w_index[l][2]=w;
                minR = dr;
            }
        }
    }

    // Now you have index of wire on upper and lower size of the sense
    // that has the smallest distance compare to the sense wire.
    // Only for the middle layer, I need to change the one of the index
    w_index[1][2] = (w_index[1][0] + w_index[1][1])/2;
    // Check:
//     printf("-----\n");
//     for(int l=0;l<3;l++){
//              for(int w=0;w<3;w++){
//                  printf("w_index[%d][%d] %d ",l,w,w_index[l][w]);
//              }
//              printf("\n");
//     }

    /// Draw cell
    int counter=0;
    double xx_rot_trans[3][3]={0}; // Row
    double yy_rot_trans[3][3]={0}; // Col
    int xx_min_index[3]={0}; // Layer
    int xx_max_index[3]={0};
    int counter_label=0;
    for(int l=0;l<3;l++){
        int ll = layer-1+l;
        xx_min_index[l]=1e5;
        xx_max_index[l]=-1e5;
        double min_x = 1e5;
        double max_x = -1e5;
        for(int c=0;c<3;c++){
            int ww = w_index[l][c];
            double xx,yy;
            getZ(ll,ww,z,xx,yy);
            double RotAng;
            const double PI=3.14159265;
            if(sense_y>0 && sense_x>0){
                RotAng = -1*atan(sense_x/sense_y);
            }else if(sense_y>0 && sense_x<0){
                RotAng = atan(-sense_x/sense_y);
            }else if(sense_y<0 && sense_x<0){
                RotAng = atan(sense_y/sense_x)+PI/2;
            }else if(sense_y<0 && sense_x>0){
                RotAng = -(atan(-sense_y/sense_x)+PI/2);
            }
            double x_tmp,y_tmp;
            //printf("[%d][%d] %f %f %f ",ll,ww,xx,yy,RotAng);
            rotate(sense_x,sense_y,x_tmp,y_tmp,RotAng);
            rotate(xx,yy,xx,yy,RotAng);
            translate(x_tmp,y_tmp,xx,yy);
            //printf("[%d][%d] %f ",ll,ww,xx);
            xx_rot_trans[l][c]=xx;
            yy_rot_trans[l][c]=yy;

            //printf("[%d][%d] (%f,%f)",ll,ww,xx,yy);
            if(xx<min_x){
                xx_min_index[l]=c;
                min_x = xx;
            }
            if(xx>max_x){
                xx_max_index[l]=c;
                max_x = xx;
            }
            // Not getting the center position
            counter++;
        }
        //      printf("\n");
    }

    /// Sort wires
    std::vector<double>x_sort[3];
    std::vector<double>y_tmp[3];
    std::vector<int> index[3];
    std::vector<double>y_sort[3];
    for(int l=0;l<3;l++){
        //    printf("---------\n");
        for(int w=0;w<3;w++){
            index[l].push_back(w);
            x_sort[l].push_back(xx_rot_trans[l][w]);
            y_tmp[l].push_back(yy_rot_trans[l][w]);
        }
        //    printD(x_sort[l]," <- x");
        //    printD(y_tmp[l]," <- y");
        //    printI(index[l]," <- index");

        std::sort(index[l].begin(),index[l].end(), MyComparator(x_sort[l]));
        //    printI(index[l]," <- index sorted");

        for(int w=0;w<3;w++){
            int ii = index[l].at(w);
            y_sort[l].push_back(yy_rot_trans[l][ii]);
        }
        std::sort(x_sort[l].begin(),x_sort[l].end());
        //    printD(y_sort[l]," <- y sorted");
    }

    // Finally, we can find beta
    //   O __ Q
    //  P /
    //  corner of unit cell

    double dx_OQ = x_sort[2].at(1)-x_sort[2].at(0);
    double dy_OQ = y_sort[2].at(1)-y_sort[2].at(0);
    double mag_OQ = sqrt(dy_OQ*dy_OQ + dx_OQ*dx_OQ);
    double dx_OP = x_sort[1].at(0)-x_sort[2].at(0);
    double dy_OP = y_sort[1].at(0)-y_sort[2].at(0);
    double mag_OP = sqrt(dy_OP*dy_OP + dx_OP*dx_OP);
    double OPdotOQ = dx_OQ*dx_OP+dy_OQ*dy_OP;
    double beta = acos(OPdotOQ/(mag_OQ*mag_OP));

//   printf("(%f,%f) (%f,%f) (%f,%f)\n",
//       x_sort[2].at(0),y_sort[2].at(0),
//       x_sort[2].at(1),y_sort[2].at(1),
//       x_sort[2].at(2),y_sort[2].at(2)
//       );
//   printf("(%f,%f) (%f,%f)\n",
//       x_sort[1].at(0),y_sort[1].at(0),
//       x_sort[1].at(1),y_sort[1].at(1)
//       );
//   printf("(%f,%f) (%f,%f) (%f,%f)\n",
//       x_sort[0].at(0),y_sort[0].at(0),
//       x_sort[0].at(1),y_sort[0].at(1),
//       x_sort[0].at(2),y_sort[0].at(2)
//       );
    double dAngle = beta*180/3.141519-pre_beta;
    if(fabs(dAngle)>30)period++;
    pre_beta = beta*180/3.141519;
//    printf("%f %f %f %f %d \n",beta, beta*180/3.141519,dAngle,period, z);
    betaTxtFile << beta << " " ;
    betaTxtFile << beta*180/3.141519 << " " ;
    betaTxtFile << dAngle << " " ;
    betaTxtFile << period << " " ;
    betaTxtFile << z << " \n" ;
    x_sort[0].clear();
    y_sort[0].clear();
    x_sort[1].clear();
    y_sort[1].clear();
    x_sort[2].clear();
    y_sort[2].clear();
}

/// Generate all layers except guard layer: 0,38
void generate(char* wireConfigFile, int maxzbin, char* outFileName="betaZ.txt"){
    gStyle->SetOptStat(0);
    if(!ReadWireMapRootFile(wireConfigFile)){   return ;   }
    double min_z;
    char *fileName = outFileName;
    bool isAddTitle = false ;
    betaTxtFile.open(fileName,ios::out | ios::trunc);
    betaTxtFile << "index layer wire beta_deg dBeta period z \n";
    for(int iLayer=1;iLayer<num_of_sense_layers-1;iLayer++){
        period=0;
        for(int iCell=0;iCell<max_senseWirePerLayer[iLayer];iCell++){
            for(int zbin=0;zbin<maxzbin;zbin++){
                min_z = zf_hv[iLayer*2][iCell*2];
                double range = zf_hv[iLayer*2][iCell*2] - zf_ro[iLayer*2][iCell*2];
                double z = min_z + zbin*1./(maxzbin-1)*fabs(range);
                betaTxtFile << zbin << " ";
                betaTxtFile << iLayer << " ";
                betaTxtFile << iCell << " ";
                plotSense(iLayer*2,iCell*2,z,"b");
            }
        }
    }
    _file0->Close();
}

/// Generate ONE layer except guard layer: 0,38
void generate(char* wireConfigFile, int maxzbin, int layer){
    if(layer==0 || layer==num_of_layers)
        std::cerr << "## Error This is guard layer!! Please ENTER [1,37]" << std::endl;
    gStyle->SetOptStat(0);
    if(!ReadWireMapRootFile(wireConfigFile)){   return ;    }
    double min_z;
    char *fileName = Form("betaZ.txt");
    bool isAddTitle = false ;
    betaTxtFile.open(fileName,ios::out | ios::trunc);
    betaTxtFile << "index layer wire beta_deg dBeta period z \n";
    for(int iCell=0;iCell<max_senseWirePerLayer[layer];iCell++){
        period=0;
        for(int zbin=0;zbin<maxzbin;zbin++){
            min_z = zf_hv[layer*2][iCell*2];
            double range = zf_hv[layer*2][iCell*2] - zf_ro[layer*2][iCell*2];
            double z = min_z + zbin*1./(maxzbin-1)*fabs(range);
            betaTxtFile << zbin << " ";
            betaTxtFile << layer << " ";
            betaTxtFile << iCell << " ";
            plotSense(layer*2,iCell*2,z,"b");
        }
    }
    _file0->Close();
}


/// Generate ONE cell except guard layer: 0,38
void generate(char *wireConfigFile, int maxzbin, int layer, int cell){
    if(layer==0 || layer==num_of_layers)
        std::cerr << "## Error This is guard layer!! Please ENTER [1,37]" << std::endl;
    gStyle->SetOptStat(0);
    if(!ReadWireMapRootFile(wireConfigFile)){   return ;    }
    double min_z;
    char *fileName = Form("betaZ.txt");
    bool isAddTitle = false ;
    betaTxtFile.open(fileName,ios::out | ios::trunc);
    betaTxtFile << "index layer wire beta_rad beta_deg dBeta period z \n";
    for(int zbin=0;zbin<maxzbin;zbin++){
        min_z = zf_hv[layer*2][cell*2];
        double range = zf_hv[layer*2][cell*2] - zf_ro[layer*2][cell*2];
        double z = min_z + zbin*1./(maxzbin-1)*fabs(range);
        betaTxtFile << zbin << " ";
        betaTxtFile << layer << " ";
        betaTxtFile << cell << " ";
        plotSense(layer*2,cell*2,z,"b");
    }
    _file0->Close();
}
