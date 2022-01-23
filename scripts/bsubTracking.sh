#!/bin/bash

if [ $# -ne 9 ]; then
    echo "$#"
    echo "./bsubTracking.sh <runNo><nfiles per lay><nevents per lay><startLayer><endLayer><iteration><experiment><input path><xt path>"
    exit -1
fi
# Get input
RUN_NUM=$1
NFILES_PER_LAY=$2
NEVNTS_PER_JOB=$3
START_LAYER=$4
END_LAYER=$5
ITER=$6
EXP=$7
FORCE_INPUT=$8
XT_PATH=$9

END_LAYER=$(( END_LAYER + 1 ))
echo "## run number      : $1"
echo "## number files    : $2"
echo "## NEVNTS_PER_JOB  : $3"
echo "## start           : $4"
echo "## end             : $5"
echo "## iteration       : $6" 
echo "## experiment      : $7"
echo "## input           : $8"
echo "## xt input        : $9"

#MSGDIR=${CCLOG_DIR}/
MSGDIR="/dev/null"
BINDIR="${CCWORKING_DIR}/bin/"

start=`date +%s`

##### loop sub root file
job_counter=0

##### loop layers
for((iLAYER=START_LAYER;iLAYER<END_LAYER;iLAYER++))
do

    echo "==============================="

    echo "Submitting jobs for Layer $iLAYER"
    
    ##### loop files 
    for((iFILE=0;iFILE<NFILES_PER_LAY;iFILE++))
    do
	FROM_IEV=$(( NEVNTS_PER_JOB*iFILE ))
	TO_IEV=$(( NEVNTS_PER_JOB*(iFILE+1) ))
	if [ -r ${FORCE_INPUT} ]; then
	    # Example 20190821
	    # bin/track 
	    # -r 1010 -e spring8 -i output/root-ana/ana_1010.root \
	    # -x info/xt_HeiC4H10_9010_1800V.default.root -S 0 \
	    # -T $LAYER -f $FROM_IEV -t $TO_IEV -F $iFILE -P run_$LAYER \
	    #file="${BINDIR}./track -r ${RUN_NUM} -t ${iLAYER} -I ${ITER} -f ${iFILE} -n ${NFILES_PER_LAY} -e ${EXP} -k 1 -i ${FORCE_INPUT}"
	    file="${BINDIR}./track "
	    file+="-r ${RUN_NUM} -e ${EXP} "
	    file+="-i ${FORCE_INPUT} "
	    file+="-x ${XT_PATH} -S 0 "
	    file+="-T ${iLAYER} -I ${ITER} "
	    file+="-f ${FROM_IEV} -t ${TO_IEV} "
	    file+="-F $iFILE "
	    file+="-P root_r${RUN_NUM}_i${ITER}_l${iLAYER}_${iFILE}"
	else
	    echo "@@@@ No such input file ${FORCE_INPUT}"
	fi
	echo "bsub -q s $file"
	bsub -o $MSGDIR -q s $file
	let job_counter++
    done			
   # Start merging process
done

for((iLAYER=START_LAYER;iLAYER<END_LAYER;iLAYER++))
do
    echo ""
    ./mergeRoot.sh ${RUN_NUM} ${ITER} ${iLAYER}
done

echo "=============================================="

echo "  Job submission run${RUN_NUM} i${ITER} sub:${iSUB}  "    

echo "You have submitted ${job_counter} jobs !!"

echo "=============================================="

