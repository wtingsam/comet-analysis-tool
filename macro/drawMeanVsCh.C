#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

const int size = 1000;
double gx_channel[size];
double gy_mean[size];
double gy_mean_err[size];
TGraphErrors *g_mean;
void init(){
  for(int i=0;i<size;i++){
    gx_channel[i]  = -99;
    gy_mean[i]	   = -99;
    gy_mean_err[i] = -99;
  }
}
void plot(char* calib_path, int max_ch){
  ifstream logFile(calib_path);
  if(!logFile.is_open()){
    std::cerr << "## ERROR cannot open logPath file: \"" << calib_path <<"\""<< std::endl;;
    return ;
  }
  Int_t run,testLayer,iter,hch,cell,entries;
  Double_t mean, mean_err, sig;
  Int_t graphEntries = 0;
  Int_t numOfChs = 0;
  std::string line; 
  double max_gy=-1e6;
  double min_gy= 1e6;
  while(getline(logFile,line)){
    std::istringstream iss(line);
    if(numOfChs>0){
      iss >> run >> testLayer >> iter >> cell >> hch >> mean >> mean_err >> sig >> entries;
      gx_channel[graphEntries] = (double)hch;
      gy_mean[graphEntries] = mean*1000;
      if(mean*1000>max_gy)max_gy=mean*1000;
      if(mean*1000<min_gy)min_gy=mean*1000;
      //      printf("%f %f \n",channel[graphEntries],mean[graphEntries]);
      gy_mean_err[graphEntries] = mean_err*1000;
      //      std::cout << numOfChs<<". "<< line << std::endl;
      graphEntries++;
    }
    numOfChs++;
  }
  g_mean=new TGraphErrors(numOfChs);
  for(int i=0;i<numOfChs;i++){
    printf("%f %f+-%f\n",gx_channel[i],gy_mean[i],gy_mean_err[i]);
    if(gx_channel[i]==-99)continue;
    g_mean->SetPoint(i,gx_channel[i],gy_mean[i]);
    g_mean->SetPointError(i,0,gy_mean_err[i]);
  }
  g_mean->SetMarkerColor(kRed);
  g_mean->SetMarkerStyle(4);
  g_mean->SetMarkerSize(0.2);
  TH2D* frame = new TH2D("h","For Calibraiton: Mean vs channel",max_ch+1,0,max_ch,100,min_gy-50,max_gy+50);
  gStyle->SetOptFit(0);
  gStyle->SetOptStat(0);
  frame->Draw();
  frame->SetXTitle("Channel");
  frame->SetYTitle("Mean of residue [um]");
  frame->SetAxisColor(17,"X");
  frame->SetAxisColor(17,"Y");
  g_mean->Draw("same p");
  gPad->SetGrid();
  gPad->SaveAs("calib_mean_ch.pdf");
}
