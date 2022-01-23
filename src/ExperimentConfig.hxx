#ifndef _EXPERIMENTCONFIG_HXX_
#define _EXPERIMENTCONFIG_HXX_

#include "TString.h"
#include <iostream>
#include <fstream>
#include <map>
#include <set>

using namespace std;
/***
    This is a singleton class.
    Only call it one time when you need it at the beginning of the main function
***/

class ExperimentConfig
{

    public:
        ~ExperimentConfig();

        static ExperimentConfig &Get();

        /// Initial the path for this class
        void Initialize(void);
        void Initialize(TString configFile, TString expName);

        /// Set the configuration file name
        void SetConfigFile(TString file){
            fConfigFile = fConfigDIR+"/"+file;
        }

        ///  Read input file and make a mapping
        void ReadInputFile(void);

        /// Set the name of the experiment e.g. crtsetup2
        void SetExperimentName(TString nameExp);

        /// Check if there is the name of the parameters
        bool HasParameter(TString parameterName);

        /// Map containing list of parameters and their values
        map<TString, TString, less<TString> > mapOfExperimentConfig;
        typedef map<TString, TString, less<TString> >::iterator mapIterator;


        /// Information about electronics
        Int_t GetMaxBoards(void){  return GetParameterI("MaxBoards");  }
        Int_t GetMaxBoardLayID(void){  return GetParameterI("MaxBoardLayID");  }
        Int_t GetMaxBoardLocID(void){  return GetParameterI("MaxBoardLocID");  }
        Int_t GetSampleRECBE(void){  return GetParameterI("MaxSamples");  }

        /// Information about CDC
        Int_t GetMaxCellID(void){  return GetParameterI("MaxCellID");  }
        Int_t GetMaxWireID(void){  return GetParameterI("MaxWireID");  }
        Int_t GetMaxLayers(void){  return GetParameterI("MaxLayers");  }
        Int_t GetMaxSense(void){  return GetParameterI("MaxSense");  }
        Int_t GetMaxField(void){  return GetParameterI("MaxField");  }
        Int_t GetMaxWires(void){  return (GetParameterI("MaxField")+GetParameterI("MaxSense"));  }

        /// Some configuration values for the experiment
        Int_t GetNumOfBoard(void){  return (GetParameterI("NumOfCDCBoard")+GetParameterI("NumOfTrigBoard"));  }
        Int_t GetNumOfChannel(void){  return (GetParameterI("NumOfCDCBoard")+ GetParameterI("NumOfTrigBoard"))*48;  }
        Int_t GetNumOfCDCChannel(void){  return GetParameterI("NumOfCDCBoard")*48;  }
        Int_t GetNumOfTrigChannel(void){  return GetParameterI("NumOfTrigBoard")*48;  }
        Int_t GetNumOfLayer(void){  return GetParameterI("NumOfUsedLay");  }
        Int_t GetNumOfWirePerLayer(void){  return GetParameterI("NumOfUsedWirePerLay");  }
        Int_t GetNumOfCDCBoard(void){  return GetParameterI("NumOfCDCBoard");  }
        Int_t GetNumOfTrigBoard(void){ return GetParameterI("NumOfTrigBoard");  }
        Double_t GetSetUpAng(void){ return GetParameterD("RotatedAng"); }

        void GetExpBoardConfig(std::vector<int> *bdid, std::vector<int> * hwbdid);
        /// Print the selected experiment configuration
        void PrintExperiment(void);

        /// This is experiment code for other package
        Int_t GetExperimentID(void){  return fExperimentID;  }
        TString GetExperimentName(void){  return fExperimentName;  }
        TString GetWireMapFile(void){ return GetParameterS("WireMap");}
        TString GetBetaZFilePath(void){ return GetParameterS("betaZfile");}
        TString GetRunLogFile(void){ return GetParameterS("LogFile"); }

        /// Same as ICEDUST, you can get different type from the configuration file
        Int_t GetParameterI(TString parameterName);

        Double_t GetParameterD(TString parameterName);

        TString GetParameterS(TString parameterName);

        /// Clear the mapping
        void ClearMapOfConfig(void);

        /// Print the mapping
        void PrintListOfParameters(void);

    private:
        /// Check emptiness of the string
        bool IsEmpty(TString s_card);

        ExperimentConfig(){}     // Don't Implement
        ExperimentConfig(ExperimentConfig const&){}        // Don't Implement
        void operator=(ExperimentConfig const&); // Don't implement

        static ExperimentConfig* fExperimentConfig;

        std::set<TString> fixedParameters;

        std::vector<int> boardID;
        std::vector<int> hardwareID;

        TString fExperimentName;
        TString fRunLogFile;
        TString fConfigDIR;
        TString fConfigFile;
        Int_t fExperimentID;
};

#endif
