#include "Iteration.hxx"
Iteration* Iteration::fIteration = NULL;

bool Iteration::IsEmpty(TString s_card)
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

    return flag;
}

Iteration &Iteration::Get()
{
    if ( !fIteration ){
        fIteration = new Iteration();
    }
    return *fIteration;
}

void Iteration::Initialize(Int_t iterStep){
    fInputIterStep = iterStep;
    fRoutine      = "";
    fXtCalState           = "";
    fT0CalState           = "";
    fErrCalState  = "";
    fWireCalState = "";
    fEPCalState           = "";

    fXtAvgFromLay = -1;
    fXtAvgToLay           = -1;
    fXtUseFromLay = -1;
    fXtUseToLay           = -1;

    fErrAvgFromLay = -1;
    fErrAvgToLay   = -1;
    fErrUseFromLay = -1;
    fErrUseToLay   = -1;
}

bool Iteration::ReadRoutine(TString routine){
    char* workingDIR = getenv ("CCWORKING_DIR");
    if(workingDIR==NULL){
        fprintf(stderr,"##!! No working directory \"CCWORKING_DIR\" in env!");
        sprintf(workingDIR, ".");
        fprintf(stderr,"##!! Using default value:\"%s\"",workingDIR);
    }
    fRoutine = Form("%s/info/",workingDIR) + routine ;

    int inputState=0;
    string line;
    string inputString;
    string parameterName;
    string parameterValue;

    ifstream inputFile(fRoutine, ios::in);
    if (!inputFile){
        std::cerr << "## No routine file: \"" << fRoutine <<"\""<< std::endl;
        return false;
    }
    while(getline(inputFile,line)){
        std::istringstream iss(line);
        string iterationStep;
        string buf;
        iss >> iterationStep >> buf;
        string tmpName = iterationStep.substr(0,iterationStep.length()-1);
        if(IsEmpty(line))continue;
        Int_t iterStep = atoi( buf.c_str() );
        if( iterationStep[iterationStep.length()-1] == ':'){
            if( tmpName == "iteration" && iterStep == fInputIterStep){
                inputState=1;
            }else{
                inputState=0;
            }
        }else{
            if(inputState==1){
                parameterName = iterationStep;
                if(parameterName=="xt") fXtCalState       = buf ;
                if(parameterName=="t0") fT0CalState       = buf ;
                if(parameterName=="error") fErrCalState   = buf ;
                if(parameterName=="wire") fWireCalState   = buf ;
                if(parameterName=="endplate") fEPCalState = buf ;
            }
        }
    }
    inputFile.close();
    return true;
}


void Iteration::Print(){
    printf("=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!\n");
    printf("## The calibration is following this routine\n");
    printf("## This is %d time iteration \n",fInputIterStep);
    printf("## Map            : %s \n", fRoutine.Data());
    printf("## Xt Calib       : %s \n", fXtCalState.Data());
    printf("## T0 Calib       : %s \n", fT0CalState.Data());
    printf("## Err Calib      : %s \n", fErrCalState.Data());
    printf("## Wire Calib     : %s \n", fWireCalState.Data());
    printf("## Endplate Calib : %s \n", fEPCalState.Data());
    printf("=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!\n");
}
