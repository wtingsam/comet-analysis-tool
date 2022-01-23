#!/bin/bash
###################################################
####                                           ####
####    X-t relation calibration procedure     ####
####    Written by Sam Wong		       ####
####					       ####
###################################################

#nohup ./doIteration_avgXT_s8.sh 1012 11 9 30 0 9 &
#for i in 1012 1047 1072 1042 1053 52 58 61 67 82 103 84 83 104 42 51 46 64 1015 1016 1050 1049 1048;do
#for i in  52 58 61 67 82 103 84 83 104 42 51 46 64;do

function RunCMD(){
    STEP=$1
    OPEN=$2
    
    # Check if you have the routine
    if [ ! -r $OPEN ]; then
	echo "@@@@ ERROR cannot find $OPEN" 
	exit 1; 
    fi
    echo -n "## Use $2"

    if [ $STEP -ne 0 ];
    then
        # Check if the symbolic link already exist
        # If so delete and link it to average xt routine and then run job submission
	ln -fs ${OPEN} ${SB_LINK}
        CMD="./calibXt.sh ${RUN_NUMBER} ${START_ITERATION} ${STEP} ${NUM_FILE} ${FROM_LAYER} ${TO_LAYER} ${EXP}"
        echo " $CMD"
	$CMD
	START_ITERATION=$(($START_ITERATION+$STEP))
    else
	echo " "
    fi
}

#EXP="crtsetup2"
EXP="spring8"
FROM_LAYER=3
TO_LAYER=5
STEP_USE_AVG_XT=0
STEP_USE_MID_XT=0
STEP_USE_ALL_XT=0
STEP_USE_EVE_XT=0
STEP_USE_ODD_XT=0
STEP_USE_L4_XT=0
MAX_JOBS=300

usage () {
cat<<EOF
EOF                                                                                                                                                                     
usage: `basename $0` [options] 
================================
Options:                        

-h, --help                          This help message

-d, --debug                         Enable debug output

-e, --experiment                    Select experiment [default: spring8]

-f, --from                          From layer you want to submit

-r, --runNo                         Run number(s)

-t, --to                            To layer you want to submit

-m, --maxJobs                       Set Maximum job numbers [default: 300]

    --avgXt                         Steps of averaged xt you want to use

    --midXt                         Steps of middle layer xt you want to use

    --allXt                         Steps of all layer xt you want to use
EOF
  exit $1

}

while [ "$#" != 0 ]; do
    case "$1" in
	-h | --help )
	    usage 0;;
	-d | --debug )
	    shift;Debug=true; set -x; DEBUG_STREAM=/dev/stderr ;;
 	-e | --experiment )
	    shift EXP="$1"; shift;;
	-r | --from )
	    shift; RUN_LIST="$1" ; shift;;
	-f | --from )
	    shift; FROM_LAYER="$1" ; shift;;
	-t | --to )
	    shift; TO_LAYER="$1" ; shift;;
	-m | --maxJobs )
	    shift; MAX_JOBS="$1" ; shift;;
	--avgXt )
	    shift; STEP_USE_AVG_XT="$1" ; shift;;
	--midXt )
	    shift; STEP_USE_MID_XT="$1" ; shift;;
	--allXt )
	    shift; STEP_USE_ALL_XT="$1" ; shift;;
	-*)
	    echo Unknown option $1 ; usage 1 ;;
	*)
	    shift || true;;
    esac
done

CALIB_ROUTINE="spring8"
if [ $EXP != "spring8" ];then
    CALIB_ROUTINE="cdc"
fi

ITERATION_STEP=$(( STEP_USE_AVG_XT + STEP_USE_MID_XT + STEP_USE_ALL_XT + STEP_USE_ODD_XT + STEP_USE_EVE_XT + STEP_USE_L4_XT ))

# Calculate number of sensitive layers
let " NUM_LAYERS=${TO_LAYER}-${FROM_LAYER}+1 "
# Calculate number of files per layers
let " NUM_FILE=${MAX_JOBS}/(${NUM_LAYERS})"

#for RUN_NUMBER in 1012;do
#TODO, make run -> run list
for RUN_NUMBER in $RUN_LIST ;do
    START_ITERATION=0
    
    echo "%% Run pre-tracking"
    CMD_PRE=""
    ## Check if the file exist or not
    if [ -r $CCPRE_ANALYSIS_DIR/ana_${RUN_NUMBER}.root ]; 
    then
    	echo "%% WARNING: Already have $CCPRE_ANALYSIS_DIR/ana_${RUN_NUMBER}.root"
    else
	RUN_NUM=`printf "%.6d" ${RUN_NUMBER}`
	BIN_DIR="${CCWORKING_DIR}/bin"
	INPUT_DIR="${CCWORKING_DIR}/input"
	if [ $EXP == "spring8" ];
	then
	    INPUT_DIR+="_s8"
	    CMD_PRE="${BIN_DIR}/pre-track -r ${RUN_NUMBER} -i ${INPUT_DIR}/run_${RUN_NUM}_built.root -e spring8 -n 200000 "
	elif [ $EXP == "cdc" ];
	then
	    CMD_PRE="${BIN_DIR}/pre-track -r ${RUN_NUMBER} -i ${INPUT_DIR}/run_${RUN_NUM}_built.root -e crtsetup2"
	fi
	echo $CMD_PRE
	$CMD_PRE    
    fi

    echo "%% ================================================="
    echo "%% Iteration step from     : $START_ITERATION"
    echo "%% Iteration step to       : $ITERATION_STEP"
    echo "%% Iteration step Layers   : $FROM_LAYER - $TO_LAYER"
    echo "%% Number of layers        : $NUM_LAYERS"
    echo "%% Max. jobs per iteration : $MAX_JOBS"
    echo "%% Jobs per layer          : $NUM_FILE"
    echo "%% Step for using avg xt   : $STEP_USE_AVG_XT"
    echo "%% Step for using mid xt   : $STEP_USE_MID_XT"
    echo "%% Step for using all xt   : $STEP_USE_ALL_XT"
    echo "%% Step for using odd xt   : $STEP_USE_ODD_XT"
    echo "%% Step for using even xt  : $STEP_USE_EVE_XT"
    echo "%% Step for using lay 4 xt : $STEP_USE_L4_XT"

    SB_LINK="${CCWORKING_DIR}/info/calibration-xt.txt"
    OPEN_AVG_XT="${CCWORKING_DIR}/info/calibration-xt-avg-${CALIB_ROUTINE}.txt"
    OPEN_MID_XT="${CCWORKING_DIR}/info/calibration-xt-mid-${CALIB_ROUTINE}.txt"
    OPEN_ALL_XT="${CCWORKING_DIR}/info/calibration-xt-all-${CALIB_ROUTINE}.txt"
    OPEN_ODD_XT="${CCWORKING_DIR}/info/calibration-xt-odd-${CALIB_ROUTINE}.txt"
    OPEN_EVE_XT="${CCWORKING_DIR}/info/calibration-xt-eve-${CALIB_ROUTINE}.txt"
    OPEN_L4_XT="${CCWORKING_DIR}/info/calibration-xt-l4-${CALIB_ROUTINE}.txt"
    
    # From iteration 0 to STEP_USE_AVG_XT, Open average x-t for all layer
    RunCMD $STEP_USE_AVG_XT $OPEN_AVG_XT
    RunCMD $STEP_USE_MID_XT $OPEN_MID_XT
    RunCMD $STEP_USE_ALL_XT $OPEN_ALL_XT
    RunCMD $STEP_USE_ODD_XT $OPEN_ODD_XT
    RunCMD $STEP_USE_EVE_XT $OPEN_EVE_XT
    RunCMD $STEP_USE_L4_XT $OPEN_L4_XT
done