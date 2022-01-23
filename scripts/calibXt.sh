#!/bin/bash

if [ $# -ne 7 ]; then
    echo "./calibXt.sh <runNo><StartIter><nfiles_per_layer><nevents_per_lay><fromLayer><toLayer><experiment>"
    exit -1
fi

RUN_NUMBER=$1;
START_ITER=$2;
ITERATION_STEP=$3;
NFILES_PER_LAY=$4;
NEVNTS_PER_JOB=$5;
FROMLAYER=$5;
TOLAYER=$6
EXP=$7

# Make sure the 
if [ $FROMLAYER -gt $TOLAYER ];
then
    echo "@@@@ [ToLayer] must be greater than [FromLayer]"
    exit 1
fi


for((iter=START_ITER;iter<START_ITER+ITERATION_STEP;iter++))
do
    SKIP_STATE="NO"
    # Check if the file already exist
    for((iLAYER=$FROMLAYER;iLAYER<$(( TOLAYER+1 ));iLAYER++))
    do
	XT_FILE="${CCWORKING_DIR}/output/root-track/xt_${RUN_NUMBER}_i${iter}_l${iLAYER}.root"
	if [ -r ${XT_FILE} ]; 
	then
	    SKIP_STATE="YES"
	fi
    done

    if [ $SKIP_STATE == "YES" ];
    then
	echo "####  WARNING: Existence of root file: ${CCWORKING_DIR}/output/root-track/ for iteration $iter ---- Skip"
	continue;
    else
	echo "####  Iteration $iter: Job submission ready!"
    fi
    
# Job submission
     #./bsubTracking.sh 60 50 1000 5 5 0 spring8 ../output/root-ana/ana_60.root ~/xtf_58_i5_l5.p3p1.i0.chen.0305xt053l5.i18.root
    ./bsubTracking.sh ${RUN_NUMBER} ${NFILES_PER_LAY} ${NEVNTS_PER_JOB} ${FROMLAYER} ${TOLAYER} ${iter} ${EXP} ${FORCE_INPUT} ${XT_PATH}
# Get x-t relation
    # Get averaged x-t relation using middle layers
    let " NUM_LAYER=($TOLAYER-$FROMLAYER+1)" 
    if [ ${FROMLAYER} -eq 0 ]; then
	NUM_LAYER=$(( NUM_LAYER - 1 ))
    fi
    let " IS_ODD=$NUM_LAYER%2"
    let " MIDDLE_LAYER= $FROMLAYER + ( $NUM_LAYER )/2 "

    # Sometimes you wanna use all layers for fitting, then you set the testlayer -- fromLayer=0
    if [ ${FROMLAYER} -eq 0 ] ;then
	MIDDLE_LAYER=$(( MIDDLE_LAYER+1 ))
    fi	
    
    let " FROMLAYER_AVG=$MIDDLE_LAYER-1"
    let " TO_AVG=$MIDDLE_LAYER+1"

    # If not odd number, then FROMLAYER should -1
    if [ ${IS_ODD} -eq 0 ];then
	FROMLAYER_AVG=$(( $FROMLAYER_AVG - 1 ))
    fi
     
    # echo "$IS_ODD"
    # echo "num layer $NUM_LAYER | fromLayer $FROMLAYER to $TOLAYER"
    # echo "middle ${MIDDLE_LAYER}  fromLayer_avg ${FROMLAYER_AVG} to_avg ${TO_AVG}"

    # check if you are testing only 1 layer
    if [ $MIDDLE_LAYER -eq 1 ] ;then 
	exit 1 
    fi
    
    # check if fromLayer/to is out of range
    if [ $FROMLAYER_AVG -lt $FROMLAYER ] ;then 
	FROMLAYER_AVG=$FROMLAYER
    fi
    if [ $TO_AVG -gt $TOLAYER ] ;then 
	TO_AVG=$TOLAYER
    fi

    FROMLAYER_LAYER=$FROMLAYER
    let "TO_LAYER=$TOLAYER+1"
    for ((iLAYER=$FROMLAYER_LAYER;iLAYER<$TO_LAYER;iLAYER++))
    do
	CMD_AVG_xt="${CCWORKING_DIR}/bin/getXt -r ${RUN_NUMBER} -I ${iter} -t $iLAYER -e ${EXP}"
	if [ $iLAYER -eq 6 ];
	then
            CMD_AVG_xt+=" -m avg"
	fi
	echo ${CMD_AVG_xt}
	#${CMD_AVG_xt}    
    done
    
done