#include "CalibrationFile.hxx"

CalibrationFile *CalibrationFile::fCalibrationFile = NULL;

CalibrationFile &CalibrationFile::Get()
{
    if ( !fCalibrationFile ){//make sure only being created one time
        fCalibrationFile = new CalibrationFile();
    }

    return *fCalibrationFile;
}

void CalibrationFile::LoadXtFile(TString path){
    if(Iteration::Get().GetXtCalState()=="constant"){
        std::cout << "## CalibrationFile: Constant x-t relation " << std::endl;
        return ;
    }
    if(fXtFunctionFile) fXtFunctionFile=NULL;
    fXtFunctionFile=new TFile(path);
    if(fXtFunctionFile->IsOpen()){
        std::cout << "## Reading root file for x-t relation \n----    Name: "<< fXtFunctionFile->GetName() << std::endl;
    }else{
        char* PAR_DIR = getenv ("CCPARAMETER_DIR");
        fXtFunctionFile = new TFile(Form("%s/xt.iso.default.root",PAR_DIR));
        std::cout << "## Reading default file for x-t relation \n----    Name: "<< fXtFunctionFile->GetName() << std::endl;
    }
}
