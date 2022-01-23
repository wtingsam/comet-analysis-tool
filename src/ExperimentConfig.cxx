#include "ExperimentConfig.hxx"
#include <sstream>
#include <fstream>
#include <stdlib.h>

ExperimentConfig* ExperimentConfig::fExperimentConfig = NULL;

ExperimentConfig &ExperimentConfig::Get()
{
    if ( !fExperimentConfig ){
        fExperimentConfig = new ExperimentConfig();
    }
    return *fExperimentConfig;
}

void ExperimentConfig::Initialize()
{
    char* workingDIR = getenv ("CCWORKING_DIR");
    if(workingDIR==NULL){
        fprintf(stderr,"##!! No working directory \"CCWORKING_DIR\" in env!");
        sprintf(workingDIR, ".");
        fprintf(stderr,"##!! Using default value:\"%s\"",workingDIR);
    }
    TString s( workingDIR );
    s=s+"/info";
    fConfigDIR = s;
}

void ExperimentConfig::Initialize(TString configFile, TString expName)
{
    char* workingDIR = getenv ("CCWORKING_DIR");
    if(workingDIR==NULL){
        fprintf(stderr,"##!! No working directory \"CCWORKING_DIR\" in env!");
        sprintf(workingDIR, ".");
        fprintf(stderr,"##!! Using default value:\"%s\"",workingDIR);
    }
    TString s( workingDIR );
    fConfigDIR = s+"/info";
    fConfigFile = fConfigDIR+"/"+configFile;

    SetExperimentName(expName);
    ReadInputFile();
    PrintExperiment();
}

void ExperimentConfig::SetExperimentName(TString nameExp)
{
    fExperimentName=nameExp;
    fExperimentName.ToLower();
}

bool ExperimentConfig::IsEmpty(TString s_card)
{
    bool flag = false;
    int offset=0;
    for(;offset<s_card.Length();offset++){
        if( s_card[offset] != ' ' && s_card[offset] != '\t')  break;
    }
    if(s_card.Length() == offset){// Empty line
        flag =true;
    }
    else if( s_card[offset] == '#'){// For comments
        flag = true;
    }
    else if( s_card[offset] == '*'){// Decoration
        flag = true;
    }

    return flag;
}

void ExperimentConfig::ReadInputFile(void)
{
    ifstream inputFile(fConfigFile, ios::in);
    if (!inputFile){
        std::cerr << "## No experiment configuration file: \"" << fConfigFile <<"\""<< std::endl;
        return ;
    }
    int inputState=0;
    int subInputState = 0;

    string line;
    string inputString;
    string parameterName;
    string parameterValue;

    while(getline(inputFile,line)){
        std::istringstream iss(line);
        string expName;
        string buf;
        iss >> expName >> buf;
        string tmpName = expName.substr(0,expName.length()-1);
        if(IsEmpty(line))continue;

        if( expName[expName.length()-1] == ':'){
            if( tmpName == fExperimentName ){
                inputState=1;
            }else if( tmpName == "cdc" ){
                inputState=1;
            }else if( tmpName == "recbe" ){
                inputState=1;
            }else{
                inputState=0;
            }
        }
        else{

            if(inputState==1){
                if( expName == "</BeginTable>" ){
                    subInputState=1;
                    continue;
                }
                if( expName == "</EndTable>" ){
                    subInputState=2;
                    continue;
                }

                if(subInputState==0){
                    parameterName = expName;
                    parameterValue = buf;
                    mapOfExperimentConfig[parameterName] = parameterValue;
                }else if(subInputState==1){
                    boardID.push_back( atoi( expName.c_str() ) );
                    hardwareID.push_back( atoi( buf.c_str() ) );
                }else if(subInputState==2){
                    subInputState=0;
                }else{
                    std::cerr << "## Unknown state" << std::endl;
                }
            }
        }
    }

    inputFile.close();
}

void ExperimentConfig::ClearMapOfConfig(void)
{
    mapOfExperimentConfig.clear();
}

bool ExperimentConfig::HasParameter(TString parameterName)
{
    mapIterator i = mapOfExperimentConfig.find(parameterName);
    if(i != mapOfExperimentConfig.end())
        return 1;
    else{
        std::cerr << "## No experiment configuration files" << std::endl;
        return 0;
    }
}

Int_t ExperimentConfig::GetParameterI(TString parameterName)
{
    if(HasParameter(parameterName)){
        return atoi(mapOfExperimentConfig[parameterName]);
    }else{
        std::cerr << "## ExperimentConfig::GetParameterI\n";
        std::cerr << "## Cannot find parameter\n";
        std::cerr << "## Name: " << parameterName << "\n";
        return -1;
    }
    return -1;
}

Double_t ExperimentConfig::GetParameterD(TString parameterName) {
    if(HasParameter(parameterName)){
        return atof(mapOfExperimentConfig[parameterName]);
    }else{
        std::cerr << "## ExperimentConfig::GetParameterD\n";
        std::cerr << "## Cannot find parameter\n";
        std::cerr << "## Name: " << parameterName << "\n";
        return -1;
    }
    return -1;
}

TString ExperimentConfig::GetParameterS(TString parameterName)
{
    if(HasParameter(parameterName)){
        return mapOfExperimentConfig[parameterName];
    }else{
        std::cerr << "## ExperimentConfig::GetParameterS\n";
        std::cerr << "## Cannot find parameter\n";
        std::cerr << "## Name: " << parameterName << "\n";
    }
    return TString();
}

void ExperimentConfig::PrintExperiment(void)
{
    std::cout << "## Print out the list of experiment's parameters from mapping\n";
    std::cout << "---   RECEBE " << std::endl;
    std::cout << "------   Max boards                : " << GetMaxBoards() << std::endl;
    std::cout << "------   Max boards LayerID        : " << GetMaxBoardLayID() << std::endl;
    std::cout << "------   Max boards LocalID        : " << GetMaxBoardLocID() << std::endl;
    std::cout << "------   Max Sampling              : " << GetSampleRECBE() << std::endl;
    std::cout << "---   CDC " << std::endl;
    std::cout << "------   Max number of cellID/lay  : " << GetMaxCellID() << std::endl;
    std::cout << "------   Max number of wire/lay    : " << GetMaxWireID() << std::endl;
    std::cout << "------   Max number of layers      : " << GetMaxLayers() << std::endl;
    std::cout << "------   Max number of Sense wires : " << GetMaxSense() << std::endl;
    std::cout << "------   Max number of Field wires : " << GetMaxField() << std::endl;
    std::cout << "------   Max number of wires       : " << GetMaxWires() << std::endl;
    std::cout << "---   Experiments used " << std::endl;
    std::cout << "------   Experiment                : " << GetExperimentName() << std::endl;
    std::cout << "------   Number of Board           : " << GetNumOfBoard() << std::endl;
    std::cout << "------   Number of Channel         : " << GetNumOfChannel() << std::endl;
    std::cout << "------   Number of CDC Ch          : " << GetNumOfCDCChannel() << std::endl;
    std::cout << "------   Number of Trig Ch         : " << GetNumOfTrigChannel() << std::endl;
    std::cout << "------   Number of Layer           : " << GetNumOfLayer() << std::endl;
    std::cout << "------   Number of Wire/Layer      : " << GetNumOfWirePerLayer() << std::endl;
    std::cout << "------   Number of CDC Board       : " << GetNumOfCDCBoard() << std::endl;
    std::cout << "------   Number of Trig Board      : " << GetNumOfTrigBoard() << std::endl;
    std::cout << "------   RECBE sample              : " << GetSampleRECBE() << std::endl;
    std::cout << "------   Rotated angle             : " << GetSetUpAng() << std::endl;
    std::cout << "------   BoardID HardwareID "  << std::endl;
    for(int i=0;i<(int)boardID.size();i++){
        printf("--------   %d        %d \n",boardID.at(i),hardwareID.at(i));
    }
    std::cout << "## Finshed print out list of experiment parameters" <<std::endl;
}

void ExperimentConfig::GetExpBoardConfig(
    std::vector<int> *bdid,
    std::vector<int> *hwbdid)
{

    // This function is only for those who have a board experiment configuration.
    if(boardID.size()<1){
        std::cerr << "## No table of board ID and hardwareboard ID \n" ;
        return ;
    }
    for(int i=0;i<(int)boardID.size();i++){
        bdid->push_back(boardID.at(i));
        hwbdid->push_back(hardwareID.at(i));
    }
}

void ExperimentConfig::PrintListOfParameters(void)
{
    printf("## Print out the list of experiment's parameters from mapping \n");
    for (mapIterator i = mapOfExperimentConfig.begin();i != mapOfExperimentConfig.end(); i++)
        std::cout << "## "<< (*i).first << " = " << (*i).second << std::endl;
}
