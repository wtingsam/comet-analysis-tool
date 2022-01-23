#!/bin/bash

if [ $# -ne 3 ]; then
    echo "./mergeRoot.sh <runNo><iteration><testlayer>"
    exit -1
fi
RUN_NUM=$1
ITER=$2
TESTLAYER=$3
XTROOTDIR=${CCTRACKROOT_DIR}
MAX_LAYER=19

jobsStatus=0
while [ $jobsStatus -ne 1 ];do
    sleep 5
    a=`bjobs`
    if [ -z "$a" ];
    then
    # Start merging process
	jobsStatus=1
	# Check Root file #
	root="${XTROOTDIR}/root_r${RUN_NUM}_i${ITER}_l${TESTLAYER}_0.root"
	echo " "
	echo $root
	if [ -e $root ]; then
	    echo "mergeRoot.sh:Proceeding Merging of root files"
	else
	    echo "mergeRoot.sh:No Root file ## ${root}"  
	    exit
	fi
	echo "----------- Merging Layer${ll} ------------"
	FILES_TO_BE_ADD="${XTROOTDIR}/root_r${RUN_NUM}_i${ITER}_l${TESTLAYER}_*.root"
	CMD="output.root ${FILES_TO_BE_ADD}"
	echo $CMD
	hadd -f $CMD
	mv output.root ${XTROOTDIR}/xt_${RUN_NUM}_i${ITER}_l${TESTLAYER}.root
	rm -f $FILES_TO_BE_ADD
	echo "mergeRoot.sh:Job finished !"
    else 
	echo -n "."
    fi 
done