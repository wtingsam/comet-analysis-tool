#ifndef __EVENTDISPLAY_HXX__
#define __EVENTDISPLAY_HXX__

#include "TStyle.h"
#include "TCanvas.h"
#include "TEllipse.h"
#include "TGraph.h"
#include "TH2D.h"
#include "math.h"
#include "TLine.h"
#include "TLatex.h"

#include "WireManager.hxx"
#include "VectorAnalysis.hxx"

#include "IHit.hxx"
#include "ITrackContainer.hxx"
#include "IUtility.hxx"

class EventDisplay{
    private:
        IHit *fHitContainer;
        ITrackContainer *fTrackCon;
    public:

        EventDisplay();
        virtual ~EventDisplay();

        /// Set IHit class
        void SetIHit(IHit *h,  ITrackContainer*t){
            fHitContainer=h;
            fTrackCon=t;
        }

        /// Set  size of canvas.
        void SetCanvasSize(int w, int h){
            fWidthCanvas=w;
            fHeightCanvas=h;
        }
        /***
             Event display: Linear track
             The format of arguments are vector.
             Options are:
             s: Labelize position of scintillators
             p: Labelize tracking parameters
             a: Labelize all
             z: Fixing the z position of drift circles
        ***/
        void DrawEvent( TString suffix, TString option);

        /// Select frame
        void SelectHistoFrame(TString name);
    private:
        //Canvas
        int fWidthCanvas;
        int fHeightCanvas;
        //configuration variable
        int TRIG_CH;
        int MAX_CH;
        double CDC_TILTED_ANGLE;

        void Init();

        /// draw
        void DrawSlideView();

        /// rotation
        void Rotation(double x, double y, double& x1, double& y1, double ang);

        /// Draw hits
        void DrawHit(std::vector<double>* x0, std::vector<double>* y0);

        TCanvas* fCanvas;
        TGraph* fCDCWirePos;
        TGraph* fCDCWirePos_CRT;
        TH2D* fCDCFrame;
        TH2D* fCDCFrame_CRT;
        TH2D* fCDCFrame_ZY;
        TH2D* fCDCFrame_ZX;
        TLine* track;
        TLine* track_cdc;
        TEllipse *fCDCEndplate[4];
        TLine *fWall[4];
        //hit
        TEllipse* fHitCircle;
        TLatex* fQ_label[300];
        double fFrameMax_x;
        double fFrameMax_y;
        double fFrameMin_x;
        double fFrameMin_y;
        double fSciPos[2];
};


#endif
