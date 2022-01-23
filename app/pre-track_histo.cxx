
//==================================================================
//    This program is for drawing the histogram having dependency
//    layer, board or cell
//
//    To change/implement output
//    Please go to the
//    src/Histogramming/PreTrackHisto.hxx
//    src/Histogramming/HistoFactory.hxx
//
//-----------------------------------------------------------------
//** Data format
//
//   histogram
//   pdf
//
//-----------------------------------------------------------------
//  Author: Sam Wong
//    Date: 23/05/2017
//==================================================================
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <unistd.h> //get opt
#include "IRunTimeManager.hxx"

#include "HistoFactory.hxx"
#include "PreTrackHisto.hxx"

#define DEBUG  0

using namespace std;

class MyAnalysisCode{
    private:
    public:
        MyAnalysisCode(){}
        ~MyAnalysisCode(){}

        void DrawHistogram(){
            HistoFactory* depHistoFactory = new HistoFactory;
            PreTrackHisto *layerDepHisto;
            layerDepHisto = depHistoFactory->createHisto("layer");
            layerDepHisto -> Excute();
            PreTrackHisto *boardDepHisto;
            boardDepHisto = depHistoFactory->createHisto("board");
            boardDepHisto -> Excute();
//             PreTrackHisto *noiseLevelHisto;
//             noiseLevelHisto = depHistoFactory->createHisto("noise");
//             noiseLevelHisto -> Excute();
        }

        void Finalize(){
        }
};

int usage(char* prog_name)
{
    fprintf(stderr,"=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@\n\n");
    fprintf(stderr,"[How is works]\n");
    fprintf(stderr,"=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@==@=@=@=@=@=@@=@\n");

    fprintf(stderr,"Usage %s [options (args)]\n",prog_name);
    fprintf(stderr,"[options]\n");
    fprintf(stderr,"\t -e\n");
    fprintf(stderr,"\t\t choose experiment:[crtsetup2(default)]\n");
    fprintf(stderr,"\t -c\n");
    fprintf(stderr,"\t\t choose configure file for experiments: [experiemnt-config.txt(default)]\n");
    fprintf(stderr,"\t -n\n");
    fprintf(stderr,"\t\t set maximum event number [0 as default (no limit)]\n");
    fprintf(stderr,"\t -r\n");
    fprintf(stderr,"\t\t force a run number\n");
    fprintf(stderr,"\t -i\n");
    fprintf(stderr,"\t\t set an input file\n");
    fprintf(stderr,"\t -p\n");
    fprintf(stderr,"\t\t set event level information print modulo [1000 as default]\n");
    fprintf(stderr,"\t -h\n");
    fprintf(stderr,"\t\t help message \n\n");
    return 0;
}

int main(int argc, char* argv[]){
    // set run options
    std::string fExperiment = "crtsetup2";
    std::string fConfigure = "experiemnt-config.txt";
    std::string fInputPath = "";
    Long64_t fPrintModulo = 1000; // Print event level information every fPrintModulo events
    Long64_t fNumEvents = 0; // Maximum  number of events to be processed. 0 means no limit;
    int fRunNo = 0;

    int optquery;
    while((optquery=getopt(argc,argv,"he:c:r:i:n:p:"))!=-1){
        switch(optquery){
        case 'e':
            fExperiment = optarg;
            std::transform(fExperiment.begin(),fExperiment.end(),fExperiment.begin(),::tolower);
            printf("Set experiment to : %s\n",fExperiment.c_str());
            break;
        case 'c':
            fConfigure = optarg;
            printf("Set configure file to : %s\n",fConfigure.c_str());
            break;
        case 'r':
            fRunNo = atoi(optarg);
            printf("Set run number to : %d\n",fRunNo);
            break;
        case 'i':
            fInputPath = optarg;
            printf("Set input path to : %s\n",fInputPath.c_str());
            break;
        case 'n':
            fNumEvents = atol(optarg);
            printf("Set maximum event number to : %lld\n",fNumEvents);
            break;
        case 'p':
            fPrintModulo = atol(optarg);
            printf("Set event print modulo to : %lld\n",fPrintModulo);
            break;
        case 'h':
        default:
            usage(argv[0]);
            return 1;
        }
    }
    if(fInputPath==""){
        usage(argv[0]);
        return 1;
    }
    if(optind<2){
        usage(argv[0]);
        return 1;
    }
    // Initialize
    IRunTimeManager::Get().Initialize(fRunNo, fConfigure, fExperiment);

    // Start reading the analysis code
    MyAnalysisCode    *userCode  = new MyAnalysisCode();
    userCode->DrawHistogram();
    return 0;
}
