#include "EventDisplay.hxx"
#include "ITrackingUtility.hxx"
#include <stdlib.h>
#include "IRunTimeManager.hxx"
#include "TVector2.h"

EventDisplay::EventDisplay()
    :
    fCanvas(NULL),
    fCDCWirePos(NULL),
    fCDCWirePos_CRT(NULL),
    fCDCFrame(NULL),
    fCDCFrame_CRT(NULL),
    fCDCFrame_ZY(NULL),
    fCDCFrame_ZX(NULL),
    track(NULL),
    track_cdc(NULL)
{
    TRIG_CH          = ExperimentConfig::Get().GetNumOfTrigChannel();
    MAX_CH           = ExperimentConfig::Get().GetNumOfChannel();
    CDC_TILTED_ANGLE = ExperimentConfig::Get().GetSetUpAng();
    fWidthCanvas  = 1024;
    fHeightCanvas = 1024;
    Init();
}
EventDisplay::~EventDisplay()
{
    delete fHitCircle;
    delete fCDCFrame_CRT;
    delete fCDCFrame;
    delete fCDCFrame_ZY;
    delete fCDCFrame_ZX;
    delete fCanvas;
}

void EventDisplay::Init()
{
    SelectHistoFrame(ExperimentConfig::Get().GetExperimentName());
}

void EventDisplay::SelectHistoFrame(TString name){
    Int_t NBIN = 128;
    fCanvas=new TCanvas("CDCEventDisplay","CDCEventDisplay",fWidthCanvas,fHeightCanvas);
    // fCanvas->Divide(2,1);
    if(name.Contains("crtsetup2")){
        fFrameMin_x=-850;       fFrameMin_y=-200;
        fFrameMax_x=-450;       fFrameMax_y=200;
        fCDCFrame_CRT = new TH2D("fCDCFrameCRT","CDD Cosmic rays test setup2",NBIN*10,fFrameMin_x,fFrameMax_x,NBIN*10,fFrameMin_y,fFrameMax_y);
        fCDCFrame     = new TH2D("fCDCFrame","Cylindrical Drift Chamber (CDC)",NBIN*10,-855,855,NBIN*10,-855,855);
    }if(name.Contains("crtsetup1")){
        fFrameMin_x=-850;       fFrameMin_y=850;
        fFrameMax_x=-850;       fFrameMax_y=850;
        fCDCFrame_CRT = new TH2D("fCDCFrameCRT","CDC Cosmic rays test setup1",NBIN*10,fFrameMin_x,fFrameMax_x,NBIN*10,fFrameMin_y,fFrameMax_y);
        fCDCFrame     = new TH2D("fCDCFrame","Cylindrical Drift Chamber (CDC)",NBIN*10,-855,855,NBIN*10,-855,855);
    }else if(name.Contains("spring")){
        fFrameMin_x=500;        fFrameMin_y=-90;
        fFrameMax_x=650;        fFrameMax_y= 90;
        fCDCFrame_CRT = new TH2D("fCDCFrameCRT","CDC Prototype 4 Spring-8 beam test",NBIN*10,fFrameMin_x,fFrameMax_x,NBIN*10,fFrameMin_y,fFrameMax_y);
        fCDCFrame     = new TH2D("fCDCFrame","Cylindrical Drift Chamber (CDC)",NBIN*10,-855,855,NBIN*10,-855,855);
        fCDCFrame_ZY  = new TH2D("fCDCFrameZY","Wire projection of Prototype 4 Spring-8 beam test",128,-300,300,128,-120,120);
        fCDCFrame_ZX  = new TH2D("fCDCFrameXY","Wire projection of Prototype 4 Spring-8 beam test",128,-300,300,128,500,650);
        fSciPos[0] = 573 + 180 + 85;
        fSciPos[1] = 573 - 180 - 85;
    }
}

void EventDisplay::DrawEvent( TString suffix, TString option)
{
    int optionFlag[128]={0};
    option.ToLower();
    for(int i=0;i<option.Length();i++){
        if(option[i]=='s') optionFlag[0]=1; // Scintillator position
        if(option[i]=='p') optionFlag[1]=1; // Parameters
        if(option[i]=='a') optionFlag[2]=1; // all
        if(option[i]=='z') optionFlag[3]=1; // Fixing z position of the hits
    }
    int fTestLayer = fHitContainer->GetTestLayer();
    ITrackingUtility fTrackFitTool;
    // Set up frame
    //Initialize Graph, Canvas and Histogram
    const int nTracks = (int)fTrackCon->GetNTracks();
    printf("Drawing %d tracks \n",nTracks);
    std::vector<double> chi2_checked_repeat;

    // Get hit information
    int iev = fHitContainer->GetEventID();
    int tn = fHitContainer->GetTriggerID();
    double qEvent = fHitContainer->GetQEvent();
    std::vector<int> v_layer = fHitContainer->GetHitLayer();
    std::vector<int> v_cell = fHitContainer->GetHitCell();
    std::vector<int> v_nPeak = fHitContainer->GetNumOfPeaks();
    std::vector<int> v_leftRight = fHitContainer->GetLeftRight();
    std::vector<double> v_hitR = fHitContainer->GetHitR();
    std::vector<double> v_driftT = fHitContainer->GetDriftTime();
    std::vector<double> v_q      = fHitContainer->GetQ();
    std::vector< std::vector<double> > vv_driftTAll = fHitContainer->GetDriftTimeAll();
    std::vector< std::vector<double> > vv_waveform = fHitContainer->GetWaveform();
    std::vector< std::vector<double> > vv_waveformClk = fHitContainer->GetWaveformClk();

    /// Draw waveform
    gStyle->SetOptStat(0);
    fCanvas->Divide(3,3);
    bool check_layer[20]={0};
    int color_code[20]={0}; for(int i=0;i<20;i++) color_code[i]=1;
    double q_layer[20]={0};
    for(int iHit=0;iHit<(int)vv_waveform.size();iHit++){
        int LayerID    = v_layer.at(iHit);
        q_layer[LayerID] += v_q[iHit];
        TH2D *hFrameWf = new TH2D(Form("hFrameWf%d",iHit),
                                  Form("Evt #%d TN #%d LayerID %d Q %8.3f ;Time[ns];ADC",iev,tn,LayerID,q_layer[LayerID]),
                                  128,0,1032,128,-100,600);
        hFrameWf->GetYaxis()->SetTitleOffset(1.3);
        if(LayerID==fTestLayer){
            hFrameWf->SetLabelColor(2);
            hFrameWf->GetYaxis()->SetLabelColor(2);
            hFrameWf->GetYaxis()->SetTitleColor(2);
            hFrameWf->GetXaxis()->SetTitleColor(2);
            hFrameWf->GetXaxis()->SetLabelColor(2);
            hFrameWf->SetTitle(Form("Evt #%d TN #%d LayerID %d (Testlayer);Time[ns];ADC",iev,tn,LayerID));
        }
        TGraph *gWaveform = new TGraph(32);
        gWaveform->SetName(Form("gWaveform%d",iHit));
        gWaveform->SetMarkerStyle(4);
        gWaveform->SetMarkerColor(color_code[LayerID]);
        gWaveform->SetLineColor(color_code[LayerID]);
        TGraph *gWaveformClk = new TGraph(32);
        gWaveformClk->SetName(Form("gWaveformClk%d",iHit));
        gWaveformClk->SetMarkerStyle(20);
        gWaveformClk->SetMarkerColor(color_code[LayerID]);
        gWaveformClk->SetLineColor(color_code[LayerID]);
        fCanvas->cd(LayerID+1)->SetGrid();
        std::vector<double> a_waveform = vv_waveform.at(iHit);
        std::vector<double> a_waveform_dt = vv_waveformClk.at(iHit);
        std::vector<double> dtv = vv_driftTAll.at(iHit);
        for(int i=0;i<(int)a_waveform.size();i++){
            gWaveform   ->SetPoint(i,               i*32,a_waveform[i]);
            double gy = (a_waveform_dt[i]>=0)?a_waveform[a_waveform_dt[i]]:-1000;
            gWaveformClk->SetPoint(i,a_waveform_dt[i]*32,gy);

        }
        //TString opt_tmp = (check_layer[LayerID])?"same p":"";
        if(!check_layer[LayerID])hFrameWf->Draw();
        gWaveform->Draw("same pl");
        gWaveformClk->Draw("same p");
        check_layer[LayerID]=true;
        color_code[LayerID]++;
    }
    fCanvas->SaveAs(Form("eventDisplay%d_waveform",iev) + suffix + ".png");

    for(int iTrk=0;iTrk<nTracks;iTrk++){
        ITrackSummary a_trackSummary = fTrackCon->GetTrackSummaries()[iTrk];

        double trackPara[4];
        // Get infomation from track container
        a_trackSummary.GetTrackPar(trackPara);
        double chi2 = a_trackSummary.GetChi2();
        int DOF = a_trackSummary.GetDOF();
        if(IUtility::Repeated(&chi2_checked_repeat,chi2)) continue; else chi2_checked_repeat.push_back(chi2);
        fCDCWirePos_CRT = new TGraph(MAX_CH);
        fCDCWirePos_CRT->SetMarkerStyle(20);
        fCDCWirePos_CRT->SetMarkerSize(0.5);
        fCDCWirePos_CRT->SetMarkerColor(17);
        int count = 0;
        for(int ch=TRIG_CH;ch<MAX_CH;ch++){
            int  layer = WireManager::Get().GetUsedLayer(ch);
            int  cell  = WireManager::Get().GetUsedCell(ch);
            double  x     = WireManager::Get().GetSenseWireXPosCen(layer,cell);
            double  y     = WireManager::Get().GetSenseWireYPosCen(layer,cell);
            double  z;
            if(optionFlag[3]==1)
                fTrackFitTool.GetHitPosition(layer,cell,x,y,z,trackPara,"wire");
            fCDCWirePos_CRT->SetPoint(count,x,y);
            count ++;
        }
        fCanvas->Clear();
        fCanvas->cd();
        fCDCFrame_CRT->Draw();
        //fCDCFrame_CRT->SetTitle(Form("Pattern Space: Trigger#%d iev#%d",tn,iev));
        fCDCWirePos_CRT->Draw("p same");
        //Draw Hits
        std::vector<double> q = fHitContainer->GetQ();
        for(int iHit=0;iHit<(int)v_layer.size();iHit++){
            int LayerID    = v_layer.at(iHit);
            int CellID    = v_cell.at(iHit);
            double x,y,z = 0;
            x = WireManager::Get().GetSenseWireXPosCen(LayerID,CellID);
            y = WireManager::Get().GetSenseWireYPosCen(LayerID,CellID);
            if(optionFlag[3]==1){
                fTrackFitTool.GetHitPosition(LayerID,CellID,x,y,z,trackPara,"wire");
            }
            std::vector<double> dtv = vv_driftTAll.at(iHit);
            int npeak = (int)dtv.size();
            for(int ipeak=0;ipeak<npeak;ipeak++){
                double              dt   = dtv[ipeak];
                Double_t    phi  = fTrackFitTool.GetIncidentAngle(LayerID,CellID, trackPara);
                double xw,yw,zw; // Point P
                fTrackFitTool.GetHitPosition(LayerID,CellID,xw,yw,zw,trackPara,"wire");
                double              beta = WireManager::Get().GetBetafromZ(LayerID,CellID,zw);
                double r = fabs(ICDCGas::Get().GetDriftDistance(dt,LayerID,CellID,phi,beta));
                //printf("layer %d cell %d iHit %d peak %d x %f y %f r %f dt %f \n",LayerID,CellID,iHit,ipeak,x,y,r,dt);
                fHitCircle = new TEllipse(x,y,r,r);
                fHitCircle->SetFillStyle(0);
                fHitCircle->SetFillColor(1);
                // if(fHitFlag->at(iHit)==1){
                //     fHitCircle->SetLineColor(2);
                // }
                if(LayerID==fTestLayer){
                    fHitCircle->SetLineColor(3);
                }
                if(dt!=v_driftT.at(iHit)){
                    fHitCircle->SetLineStyle(8);
                }
                fHitCircle->Draw("same");
            }
        }

        //Draw Tracks
        double x1,x2,y1,y2;
        x1 =  fFrameMin_x;
        x2 =  fFrameMax_x;
        y1 = x1*trackPara[0] + trackPara[1];
        y2 = x2*trackPara[0] + trackPara[1];
        // Rotation(x1,y1,x1,y1,CDC_TILTED_ANGLE);
        // Rotation(x2,y2,x2,y2,CDC_TILTED_ANGLE);
        track = new TLine(x1,y1,x2,y2);
        track -> SetLineColor(2);
        track->Draw("same");

        Double_t lex_x = fFrameMin_x ;
        Double_t lex_y = fFrameMax_y + 2;

        // Calculate the position of scintillation
        TLatex* lex_scint[2];
        double m1 = (y2-y1)/(x2-x1);
        double c1 = y1-m1*x1;
        double scint_x = fSciPos[0];
        double scint_y = scint_x*m1 + c1;
        double scint_z = scint_x*trackPara[2]+trackPara[3];
        lex_scint[0]=new TLatex(lex_x,lex_y, Form("Hit@ScintU (%f,%f,%f)", scint_x, scint_y, scint_z));
        scint_x = fSciPos[1];
        scint_y = scint_x*m1 + c1;
        scint_z = scint_x*trackPara[2]+trackPara[3];
        lex_scint[1]=new TLatex(lex_x ,lex_y + 5, Form("Hit@ScintD (%f,%f,%f)", scint_x, scint_y, scint_z));
        lex_scint[0]->SetTextSize(0.02);
        lex_scint[1]->SetTextSize(0.02);

        if(optionFlag[0]==1){
            lex_scint[0]->Draw("same");
            lex_scint[1]->Draw("same");
        }

        fCDCFrame_CRT->SetTitle(Form("Evt #%d TN #%d (%f %f %f %f) Qevent %8.3f #chi^{2}_{r} %f/%d = %f  \n",
                                     iev,tn,trackPara[0],trackPara[1],trackPara[2],trackPara[3],
                                     qEvent,
                                     chi2,DOF,chi2/DOF));
        fCanvas->SaveAs(Form("eventDisplay%d_%d",iev,iTrk) + suffix + ".png");
        // Draw Wires
        fCanvas->Clear();
        fCanvas->cd()->SetGrid(0,0);
        fCDCFrame_ZY->Draw();
        for(int iLay=0;iLay<ExperimentConfig::Get().GetNumOfLayer();iLay++)
            for(int iCel=0;iCel<WireManager::Get().GetNumSenseWires(iLay);iCel++){
                double y0 = WireManager::Get().GetSenseWireYPosRO(iLay,iCel);
                double z0 = WireManager::Get().GetSenseWireZPosHV(iLay);
                double y1 = WireManager::Get().GetSenseWireYPosHV(iLay,iCel);
                double z1 = -z0;
                TLine *wire = new TLine(z0,y0,z1,y1);
                wire->SetLineColor(18);
                wire->SetLineStyle(3112);
                wire->Draw("same");
            }
        // Draw hitted wires overlap
        int draw_count = 1;
        for(int iHit=0;iHit<(int)v_layer.size();iHit++){
            // Draw wires
            int iLay  = v_layer.at(iHit);
            int iCel  = v_cell.at(iHit);
            int colorCode = iLay;
            double y0 = WireManager::Get().GetSenseWireYPosRO(iLay,iCel);
            double z0 = WireManager::Get().GetSenseWireZPosRO(iLay);
            double y1 = WireManager::Get().GetSenseWireYPosHV(iLay,iCel);
            double z1 = -z0;
            TLine *wire = new TLine(z0,y0,z1,y1);
            wire->SetLineColor(colorCode);
            wire->Draw("same");
            //Draw labels
            double offset = fabs(z1)*0.001*draw_count;
            if(draw_count%10==0)
                draw_count=1;
            draw_count++;
            double z1_draw = z0 + offset;
            double y1_draw = y0;
            TLatex *latex = new TLatex(z1_draw,y1_draw,Form("(%d,%d)",iLay,iCel));
            latex->SetTextColor(colorCode);
            latex->SetTextSize(0.01);
            latex->Draw("same");
            draw_count++;
            // Cross points
            for(int jHit=0;jHit<(int)v_layer.size();jHit++){
                int jLay  = v_layer.at(jHit);
                int jCel  = v_cell.at(jHit);
                if(iLay==jLay-1){
                    //TVector3 cpt=ITrackingUtility::GetCrossPoint(iLay,iCel,jLay,jCel);
                    TVector2 pt =ITrackingUtility::GetIntersectPoint(iLay,iCel,jLay,jCel,"zy");
                    TEllipse *fHitCircle = new TEllipse(pt.X(),pt.Y(),1,1);
                    fHitCircle->SetLineColor(colorCode);
                    fHitCircle->SetFillColor(colorCode);
                    fHitCircle->Draw("same");
                }
            }
            // Draw left right
            double hitR = v_hitR.at(iHit);
            double theta = atan((y1-y0)/(z1-z0));
            if(v_leftRight.at(iHit)==1){
                y0 = y0+fabs(hitR/cos(theta));
                y1 = y1+fabs(hitR/cos(theta));
            }else if(v_leftRight.at(iHit)==-1){
                y0 = y0-fabs(hitR/cos(theta));
                y1 = y1-fabs(hitR/cos(theta));
            }else
                continue;
            TLine *wireLR = new TLine(z0,y0,z1,y1);
            wireLR->SetLineColor(colorCode);
            wireLR->SetLineStyle(10);
            wireLR->Draw("same");

        }
        fCanvas->SaveAs(Form("eventDisplay%dYZ_%d",iev,iTrk) + suffix + ".png");
    }
    return ;
}

void EventDisplay::Rotation(double x1, double y1, double& x2, double& y2, double ang)
{
    const double pi=3.141592;
    double A = pi*ang/180;
    x2 =  x1*cos(A)+y1*sin(A);
    y2 = -x1*sin(A)+y1*cos(A);
}
