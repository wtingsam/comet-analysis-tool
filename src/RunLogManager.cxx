#include "RunLogManager.hxx"

RunLogManager *RunLogManager::fRunLogManager = NULL;

RunLogManager::RunLogManager(){}

RunLogManager &RunLogManager::Get()
{
    if ( !fRunLogManager ){//make sure only being created one time
        fRunLogManager = new RunLogManager();
    }
    return *fRunLogManager;
}

void RunLogManager::Initialize()
{
    fRunLogPath = ExperimentConfig::Get().GetRunLogFile();
}

void RunLogManager::Initialize(int runNo)
{
    fRunLogPath = ExperimentConfig::Get().GetRunLogFile();
    ReadRunLogText(runNo);
}

void RunLogManager::Ls()
{
    if(ExperimentConfig::Get().GetExperimentName() == "crtsetup2" || ExperimentConfig::Get().GetExperimentName() == "crtsetup1"){
        std::cout<< "##########################################################  " << std::endl ;
        std::cout<< "## Cosmic ray run-info  " << std::endl ;
        std::cout<< "## Run                  : " << GetRunNumber() << std::endl ;
        std::cout<< "## Flow Rate            : " << GetFlowRate() << std::endl ;
        std::cout<< "## Run Grade            : " << GetRunGrade() << std::endl ;
        std::cout<< "## Time (mins)          : " << GetTime() << std::endl ;
        std::cout<< "## High Voltage (V)     : " << GetHighVoltage() << std::endl ;
        std::cout<< "## Threshold (mV)       : " << GetThreshold() << std::endl ;
        std::cout<< "##########################################################  " << std::endl ;
    }
    else if(ExperimentConfig::Get().GetExperimentName() == "tohoku"){
    }
    else if(ExperimentConfig::Get().GetExperimentName() == "spring8"){
        //Spring-8 prototype test
        std::cout<< "##########################################################  " << std::endl ;
        std::cout<< "## This is the info about the Spring8 beam test  " << std::endl ;
        std::cout<< "## Run number               : " <<fRunNumber      <<std::endl;
        std::cout<< "## Run grade                : "<<fRunGrade       <<std::endl;
        std::cout<< "## Run time                 : "<<fTime         <<std::endl;
        std::cout<< "## High Voltage Prototpye 4 : "<< fHighVoltage <<std::endl;
        std::cout<< "## Gas type                 : "<< fGasMixture   <<std::endl;
        std::cout<< "##########################################################  " << std::endl ;
    }
    else if(ExperimentConfig::Get().GetExperimentName() == "kek"){
        //KEK prototype test
        std::cout << "KEK prototype test is under construction" << std::endl;
    }
    else{
        std::cout<< ExperimentConfig::Get().GetExperimentName()<<" is under construction " << std::endl;
    }
}

void RunLogManager::CheckPath()
{
    if(fRunLogPath!=""){
        std::cout << "## "<< fRunLogPath << std::endl;
    }else{
        std::cerr << "Did not set Path" << std::endl;
    }
}

void RunLogManager::ReadRunLogText(Int_t runInput)
{
    /// Under developement should be changed at some points 12/22/2016
    ifstream logFile;
    logFile.open(fRunLogPath);
    if(!logFile.is_open()){
        std::cerr << "## ERROR cannot open logPath file: \"" << fRunLogPath <<"\""<< std::endl;;
        return ;
    }
    Double_t run,grade,time, B, hv, hv_g, th, flow;
    std::string gasMixture;
    std::string line;
    std::stringstream l;
    int count = 0;
    while(getline(logFile,line)){
        std::istringstream iss(line);
        if(count>0){
            iss >> gasMixture
                >> run
                >> grade
                >> time
                >> B
                >> hv
                >> hv_g
                >> th
                >> flow;
            if(runInput==run){
                fGasMixture         = gasMixture;
                fRunNumber          = run;
                fRunGrade           = grade;
                fTime               = time;
                fMagneticField      = B;
                fHighVoltage        = hv;
                fHighVoltage_guard  = hv_g;
                fThreshold          = th;
                fFlowRate           = flow;
            }
        }
        count++;
    }
}
