#ifndef _WIRECONFIG_HXX_
#define _WIRECONFIG_HXX_

#include <iostream>

#include "TFile.h"
#include "TTree.h"
#include "TString.h"

class WireConfig{

    public:
        WireConfig();
        ~WireConfig();

        void Initialize(double rotate=0, int debug=0);
        /// Path for info
        virtual void CheckPath(void);

    private:
        virtual void SetPath(TString path){ fWireMapPath=path; }
        /// The path for obtaining the wire map root file
        TString  fWireMapPath;
        // TFile *fRootFile;
        // TTree *fTree;
        Int_t fAngle;

        /// Read a mapping at info/
        void ReadWireMap(void);//modification

        /// Read a beta Z table
        void GenBetaZMapping(void);

        /// Rotate the setup by an angle (Easy for linear fitting)
        void Rotation(double x1, double y1, double& x2, double& y2, double ang){
            const double pi=3.14159265359;
            double A = pi*ang/180;
            x2 =  x1*cos(A)+y1*sin(A);
            y2 = -x1*sin(A)+y1*cos(A);
        }

    protected:
        /// Beta Z mapping, this is hard code,
        /// because changes are not expected.
        /// 500 bins for now..
        double fBeta[20][700][500];
        double fBeta_min[20][700];
        double fBeta_max[20][700];
        double fZ[20][5000][500];
        int fMaxBinOfBetaZ;

        // Configuration variable
        int MAX_CH;
        int MAX_WIREpL;
        int MAX_SENSE_LAYER;
        int TRIG_CH;

        int MAX_LAYER;
        int MAX_WIREID;

        //CDC used wires
        int *fLayerUsed;
        int *fCellUsed;
        int *fBdIDUsed;
        int *fBdLayIDUsed;
        int *fBdLocIDUsed;
        int **fChannelUsed;
        int *fNumSenseWireUsed;

        //Geometry
        double *fCDClength;
        double **fXhv;
        double **fYhv;
        double **fXro;
        double **fYro;
        double **fXc;
        double **fYc;

        int *fMaxWirePerLayer;
        int *fMaxSenseWirePerLayer;
        double *fCDClength_all;
        double **fXhv_all;
        double **fYhv_all;
        double **fXro_all;
        double **fYro_all;

        int fNumSenseWire;
        int fNumFieldWire;

        // debug
        int fDebug;
};




#endif
