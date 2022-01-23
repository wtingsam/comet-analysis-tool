//==================================================================
//    This program is for calibration of the cosmic ray tracks
//
// - Waveform analysis is considered in this program,
//   one should not use waveform information for analysis also
//-----------------------------------------------------------------
//** Data format
//
//    Unit: mm/ns
//
//-----------------------------------------------------------------
//  Author: Sam Wong
//    Date: 28/9/2016
//==================================================================
#include <stdlib.h>
#include <unistd.h> // for getopt
#include <string>
#include <algorithm>   // for tolower
#include <iostream>
#include <TStyle.h>
#include <TCanvas.h>

#include "InputRootCRT.hxx"
#include "EventLoop.hxx"

#include "IRunTimeManager.hxx"

#include "RawModeAnalysisStrategy.hxx"
#include "SuppressModeAnalysisStrategy.hxx"
#include "SimModeAnalysisStrategy.hxx"
#include "P4SPring8AnalysisStrategy.hxx"

using namespace std;

class MyAnalysisCode : public EventLoop{
    private :
        //Class
        AnalysisStrategy *_preTrackingMethod;
        InputRootCRT* fInputRootCRT;
        TString fInputRootCRTPath;
        Int_t fRunNo;
        Int_t fNumEvents;
        bool fTriggerDelay;
        TString fExperiment;
        TString fRECBEMode;
    public:
        /// Constructor
        MyAnalysisCode(Int_t runNo,std::string experiment, std::string mode)
            :_preTrackingMethod(NULL),
             fInputRootCRT(NULL),
             fInputRootCRTPath(""),
             fRunNo(runNo),fNumEvents(1000),
             fTriggerDelay(false),
             fExperiment(experiment),
             fRECBEMode(mode)
        {
            std::cout << "Run Number "<< fRunNo << " " << fExperiment << " " << fRECBEMode << std::endl;
        }
        /// Distructor
        virtual ~MyAnalysisCode(){}
        /// Force max events
        void ForceMaxEvent(int nevent){   fNumEvents=nevent;   }

        /// 
        void ForceTriggerDelay(bool triggerDelay){   fTriggerDelay=triggerDelay;   }

        /// Set up the input root , this part is wrapped up, for details go to src/InputRootCRT.cxx
        void SetInputRoot(TString path){  fInputRootCRTPath=path;  }

        /// Setup a method for different data input
        void SetAnalysisMethod(TString type){
            delete _preTrackingMethod;
            if(type.Contains("crtsetup")){
                // Two modes
                if(type.Contains("raw")){
                    fInputRootCRT = new InputRootCRT(fInputRootCRTPath,"read","tree");
                    _preTrackingMethod = new RawModeAnalysisStrategy(fInputRootCRT,fTriggerDelay);
                }else if(type.Contains("suppress")){
                    fInputRootCRT = new InputRootCRT(fInputRootCRTPath,"read","tree");
                    _preTrackingMethod = new SuppressModeAnalysisStrategy(fInputRootCRT);
                }
            }else if(type.Contains("simulation")){
                fInputRootCRT = new InputRootCRT(fInputRootCRTPath,"read","tree",true);
                _preTrackingMethod = new SimModeAnalysisStrategy(fInputRootCRT);
            }else if(type.Contains("spring8")){
                fInputRootCRT = new InputRootCRT(fInputRootCRTPath,"read","tree");
                _preTrackingMethod = new P4SPring8AnalysisStrategy(fInputRootCRT);
            }else if(type.Contains("p4kek16")){

            }else{
                std::cerr << "## Unknown analysis method" << std::endl;
            }
        }

        void Routine(){
            // Decide the mode of analysis
            SetAnalysisMethod(fRECBEMode+fExperiment);
            //Force a number of events and excute
            _preTrackingMethod->ForceMaxEvents(fNumEvents);
            _preTrackingMethod->Excute();
            std::cout << "## End of the Routine" << std::endl;
        }
};

int usage(char* prog_name)
{
    fprintf(stderr,"=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@\n\n");
    fprintf(stderr,"[How is works]\n");
    fprintf(stderr,"1. Input a raw root file from CRT DAQMW\n");
    fprintf(stderr,"2. Follow the routine of analysis\n");
    fprintf(stderr,"3. Select the analysis method using preset name\n");
    fprintf(stderr,"4. Excute the wrapped up method in its corresponding class\n");
    fprintf(stderr,"   -  One can develope its own method easily by looking at AnalysisStrategy.hxx \n");
    fprintf(stderr,"5. Produce a root file according to that method \n\n");
    fprintf(stderr,"=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@==@=@=@=@=@=@@=@\n");

    fprintf(stderr,"Usage %s [options (args)]\n",prog_name);
    fprintf(stderr,"[options]\n");
    fprintf(stderr,"\t -e\n");
    fprintf(stderr,"\t\t choose experiment:[crtsetup2(default)]\n");
    fprintf(stderr,"\t -m\n");
    fprintf(stderr,"\t\t choose RECBE mode:[rawmode(default)]\n");
    fprintf(stderr,"\t -c\n");
    fprintf(stderr,"\t\t choose configure file for experiments: [experiemnt-config.txt(default)]\n");
    fprintf(stderr,"\t -n\n");
    fprintf(stderr,"\t\t set maximum event number [0 as default (no limit)]\n");
    fprintf(stderr,"\t -r\n");
    fprintf(stderr,"\t\t force a run number\n");
    fprintf(stderr,"\t -i\n");
    fprintf(stderr,"\t\t set an input file\n");
    fprintf(stderr,"\t -T\n");
    fprintf(stderr,"\t\t Force a trigger time [default:(int)0]\n");
    fprintf(stderr,"\t -p\n");
    fprintf(stderr,"\t\t set event level information print modulo [1000 as default]\n");
    fprintf(stderr,"\t -h\n");
    fprintf(stderr,"\t\t help message \n\n");
    return 0;
}

int main(int argc, char* argv[]){
    // set run options
    std::string experiment = "crtsetup2";
    std::string recbeMode = "raw";
    std::string configure = "experiemnt-config.txt";
    std::string inputPath = "";
    Long64_t fPrintModulo = 1000; // Print event level information every fPrintModulo events
    Long64_t fNumEvents = 0; // Maximum  number of events to be processed. 0 means no limit;
    int runNo = 0;
    int fTriggerDelay = 0;
    int optquery;
    while((optquery=getopt(argc,argv,"he:m:c:r:i:n:p:T:"))!=-1){
        switch(optquery){
            case 'e':
                experiment = optarg;
                std::transform(experiment.begin(),experiment.end(),experiment.begin(),::tolower);
                printf("Set experiment to : %s\n",experiment.c_str());
                break;
            case 'm':
                recbeMode = optarg;
                std::transform(recbeMode.begin(),recbeMode.end(),recbeMode.begin(),::tolower);
                printf("Set recbeMode to : %s\n",recbeMode.c_str());
                break;
            case 'c':
                configure = optarg;
                printf("Set configure file to : %s\n",configure.c_str());
                break;
            case 'r':
                runNo = atoi(optarg);
                printf("Set run number to : %d\n",runNo);
                break;
            case 'i':
                inputPath = optarg;
                printf("Set input path to : %s\n",inputPath.c_str());
                break;
            case 'n':
                fNumEvents = atol(optarg);
                printf("Set maximum event number to : %lld\n",fNumEvents);
                break;
            case 'p':
                fPrintModulo = atol(optarg);
                printf("Set event print modulo to : %lld\n",fPrintModulo);
                break;
            case 'T':
                fTriggerDelay = atol(optarg);
                printf("Force a trigger delay  to : %d\n",fTriggerDelay);
                break;
            case 'h':
            default:
                usage(argv[0]);
                return 1;
        }
    }
    if(inputPath==""){
        usage(argv[0]);
        return 1;
    }
    if(optind<2){
        usage(argv[0]);
        return 1;
    }
    bool useTrigDelayer = false;
    if(fTriggerDelay==1){
        useTrigDelayer = true;
    }
    // Initialize
    IRunTimeManager::Get().Initialize(runNo, configure, experiment);

    // User code
    MyAnalysisCode  *userCode     = new MyAnalysisCode(runNo,experiment,recbeMode);
    userCode->SetInputRoot(inputPath);
    userCode->ForceMaxEvent(fNumEvents);
    userCode->ForceTriggerDelay(useTrigDelayer);
    userCode->Routine();
    return 0;
}
