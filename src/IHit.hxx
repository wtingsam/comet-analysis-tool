#ifndef _IHIT_HXX_
#define _IHIT_HXX_

#include <vector>
#include <iostream>
#include <stdio.h>
#include "TString.h"

using namespace std;
/***
    A class to define what is a hit
***/
class IHit{
    private:
        int fTriggerNumber;
        int fEventNumber;
        int fTestLayer;
        int fNumOfHits;
        double fQEvent;
        vector<int> fLayerID;
        vector<int> fCellID;
        vector<int> fLeftRight;
        vector<int> fHitFlag;
        vector<int> fNumOfLayHits;
        vector<int> fNumOfPeaks;
        vector<int> fPeakWidth;
        vector<double> fHitR;
        vector<double> fDriftTime;
        vector<double> fADCPeak;
        vector<double> fQ;
        vector<double> fQCut;
        vector<bool> fChoosePeakFlag;
        vector<double> fMAXADCSUM;
        vector<double> fMAXADCPeak;
        vector< vector<double> > fDriftTimeAll;
        vector< vector<double> > fWaveform;
        vector< vector<double> > fWaveformClk;
        TString fName;
    public:
        IHit(TString name=""){
            fTriggerNumber = -1;
            fEventNumber = -1;
            fTestLayer = -1;
            fNumOfHits = -1;
            fName = name;
        }
        virtual ~IHit(){
            Clear();
        }
        // Adders only for vectors
        void AddHitWire(int layer,int cell){
            fLayerID.push_back(layer);
            fCellID.push_back(cell);
        }

        void AddLeftRight(int LeftRight){fLeftRight.push_back(LeftRight);}
        void AddHitR(double HitR){fHitR.push_back(HitR);}
        void AddDriftTime(double driftTime){fDriftTime.push_back(driftTime);}
        void AddDriftTimeAll(vector<double >driftTimeAll){fDriftTimeAll.push_back(driftTimeAll);}
        void AddADCPeak(double p){fADCPeak.push_back(p);}
        void AddQ(double q){fQ.push_back(q);}
        void AddQCut(double qcut){fQCut.push_back(qcut);}
        void AddNumLayerHits(int lh){fNumOfLayHits.push_back(lh);}
        void AddNumOfPeaks(int np){fNumOfPeaks.push_back(np);}
        void AddWidthOfPeaks(int pw){fPeakWidth.push_back(pw);}
        void AddChoosePeakFlag(bool cp){fChoosePeakFlag.push_back(cp);}
        void AddHitFlag(int hf){fHitFlag.push_back(hf);}
        void AddMaxADCSUM(double q){fMAXADCSUM.push_back(q);}
        void AddMaxADCPeak(double p){fMAXADCPeak.push_back(p);}

        // Setters
        void SetQEvent(double qevnt){ fQEvent=qevnt; }
        void SetNumOfHits(int nhits){ fNumOfHits=nhits; }
        void SetEventID(int iev, int trig){
            fEventNumber=iev;
            fTriggerNumber=trig;
        }
        void SetTestLayer(int layer){ fTestLayer=layer; }
        void SetHitWire(vector<int> layer, vector<int> cell){
            fLayerID=layer;
            fCellID=cell;
        }
        // Setters vectors
        void SetLeftRight(vector<int>LeftRight){ fLeftRight=LeftRight; }
        void SetHitR(vector<double>HitR){ fHitR=HitR; }
        void SetDriftTime(vector<double>driftTime){ fDriftTime=driftTime; }
        void SetDriftTimeAll(vector< vector<double> >driftTimeAll){ fDriftTimeAll=driftTimeAll; }
        void SetADCPeak(vector<double>p){ fADCPeak=p; }
        void SetQ(vector<double>q){ fQ=q; }
        void SetQCut(vector<double>qcut){ fQCut=qcut; }
        void SetNumLayerHits(vector<int>lh){ fNumOfLayHits=lh; }
        void SetNumOfPeaks(vector<int>np){ fNumOfPeaks=np; }
        void SetWidthOfPeaks(vector<int>pw){ fPeakWidth=pw; }
        void SetChoosePeakFlag(vector<bool>cp){ fChoosePeakFlag=cp; }
        void SetHitFlag(vector<int>hf){ fHitFlag=hf; }
        void SetMaxADCSUM(vector<double>q){ fMAXADCSUM=q; }
        void SetMaxADCPeak(vector<double>p){ fMAXADCPeak=p; }
        void SetWaveform(vector< vector<double> >waveform){  fWaveform=waveform; }
        void SetWaveformClk(vector< vector<double> >waveformClk){  fWaveformClk=waveformClk; }
        // Getters
        int GetEventID(){ return fEventNumber;}
        int GetTriggerID(){ return fTriggerNumber;}
        void SelectAllRawHits(int select=1){
            for(int i=0;i<(int)fHitFlag.size();i++){
                fHitFlag.at(i)=select;
                if(fLayerID.at(i)==fTestLayer)
                    fHitFlag.at(i)=0;
            }
        }
        vector<int>  GetHitLayer(){  return fLayerID;  }
        vector<int>  GetHitCell(){  return fCellID;  }
        vector<int>  GetLeftRight(){  return fLeftRight;  }
        vector<double>  GetHitR(){  return fHitR;  }
        vector<double>  GetDriftTime(){  return fDriftTime;  }
        vector< vector<double> > GetDriftTimeAll(){  return fDriftTimeAll;  }
        vector<double>  GetADCPeak(){  return fADCPeak;  }
        vector<double>  GetQ(){  return fQ;  }
        vector<double>  GetQCut(){  return fQCut;  }
        vector<int>  GetNumOfLayHits(){  return fNumOfLayHits;  }
        vector<int>  GetNumOfPeaks(){  return fNumOfPeaks;  }
        vector<int>  GetWidthOfPeaks(){  return fPeakWidth;  }
        vector<bool>  GetChoosePeakFlag(){  return fChoosePeakFlag;  }
        vector<int>  GetHitFlag(){  return fHitFlag;  }
        int GetTestLayer(){  return fTestLayer;  }
        int GetNumOfHits(){  return fNumOfHits;  }
        double GetMaxADCSUM(int layer){  return fMAXADCSUM[layer]; }
        double GetMaxADCPeak(int layer){ return fMAXADCPeak[layer]; }
        double GetQEvent(){  return fQEvent;  }
        vector< vector<double> >  GetWaveform(){  return fWaveform;  }
        vector< vector<double> >  GetWaveformClk(){  return fWaveformClk;  }

        void Show();

        void Clear();
};
#endif
