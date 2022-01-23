#!/bin/bash

if [ $# -ne 2 ]; then
    echo "./makePDF.sh <RunNo><input>"
    exit -1
fi

if [ -z $CCWORKING_DIR ];then
    echo "## No CDCCF environment, please do source setup.sh at CDC_Calibration"
    exit -1;
fi

RootFile=`basename $2`
RunNo=`printf "%.5d" $1`
InputFile="${CCWORKING_DIR}/input/$RootFile"
BinDir="${CCWORKING_DIR}/bin"
GraphDir="${CCWORKING_DIR}/output/graph"
AnaInput="${CCWORKING_DIR}/output/root-ana/ana_$1.root"

echo "Run        : $RunNo"
echo "Bin        : $BinDir"
echo "Graph      : $GraphDir"
echo "Input      : $InputFile"
echo "Analysis   : $AnaInput"

#${BinDir}/pre-track -i ${InputFile} -r $1
${BinDir}/pre-track_histo -i ${AnaInput} -r $1

gs -dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -sOutputFile="${GraphDir}/Run${RunNo}_layerDep.pdf" ${GraphDir}/*layerDep_$1*
gs -dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -sOutputFile="${GraphDir}/Run${RunNo}_boardDep.pdf" ${GraphDir}/*boardDep_$1*
