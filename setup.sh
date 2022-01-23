#!/bin/bash

###############################################################
# Choose the directory to hold root files
###############################################################
INPUT="input"
OUTPUT="output"
USER_DIR="/group/had/muon/${USER}/calibration_data"

if [ ! -e $INPUT ]; then
    echo "For KEK user "
    echo "Suggestion Path for input: /group/had/muon/CDC/CRT/root1 (Data taken by DAQMW) "
    read -p "This is the first time lauching this setup! Where can I get raw data (ROOT format, e.g. run_000001_built.root)?"
    while [ -z $REPLY ]; do
        read -p "You must provide a directory!"
    done
    while [ ! -e $REPLY ]; do
        read -p "You must provide a valid directory!"
    done
    ln -s $REPLY $INPUT
fi
if [ ! -e $OUTPUT ]; then
    echo "For KEK user "
    echo "Suggestion Path for output: ${USER_DIR} (Data taken by DAQMW) "
    read -p "This is the first time lauching this setup! Where shall I put analysis result? [./$OUTPUT]"
    while [ -n $REPLY ] && [ ! -e $REPLY ]; do
        read -p "You must provide a valid directory!"
    done
    if [ -z $REPLY ]; then
        echo "I will put the result under ./$OUTPUT!"
        mkdir $OUTPUT
    else
        ln -s $REPLY $OUTPUT
    fi
fi

###############################################################
# Export enviromental variables
###############################################################
export CCWORKING_DIR="${PWD}"
export CCRAWROOT_DIR=${INPUT}
export CCPARAMETER_DIR=${CCWORKING_DIR}/output/parameters
export CCGRAPH_DIR=${CCWORKING_DIR}/output/graph
export CCEVENTDISPLAY_DIR=${CCWORKING_DIR}/output/eventdisplay
export CCPRE_ANALYSIS_DIR=${CCWORKING_DIR}/output/root-ana
export CCWAVEFORMROOT_DIR=${CCWORKING_DIR}/output/root-waveform
export CCHISTOGRAM_DIR=${CCWORKING_DIR}/output/root-hist
export CCTRACKROOT_DIR=${CCWORKING_DIR}/output/root-track
export CCSIMROOT_DIR=${CCWORKING_DIR}/output/root-sim
export CCEFF_DIR=${CCWORKING_DIR}/output/root-eff
export CCLOG_DIR=${CCWORKING_DIR}/output/log

echo "================================="
echo "Exported varibles"
echo "CCWORKING_DIR: $CCWORKING_DIR"
echo "CCRAWROOT_DIR: $CCRAWROOT_DIR: `readlink $CCRAWROOT_DIR`"
echo "CCPARAMETER_DIR: $CCPARAMETER_DIR"
echo "CCGRAPH_DIR: $CCGRAPH_DIR"
echo "CCEVENTDISPLAY_DIR: $CCEVENTDISPLAY_DIR"
echo "CCPRE_ANALYSIS_DIR: $CCPRE_ANALYSIS_DIR"
echo "CCHISTOGRAM_DIR: $CCHISTOGRAM_DIR"
echo "CCTRACKROOT_DIR: $CCTRACKROOT_DIR"
echo "CCSIMROOT_DIR: $CCSIMROOT_DIR"
echo "CCEFF_DIR: $CCEFF_DIR"
echo "CCLOG_DIR: $CCLOG_DIR"

###############################################################
# Check existence of CDC calibration directory
###############################################################
if [ ! -e $OUTPUT/root-ana ]; then
    mkdir $OUTPUT/root-ana
    echo "Missing root-ana. Creating one "
fi
if [ ! -e $OUTPUT/root-waveform ]; then
    mkdir $OUTPUT/root-waveform
    echo "Missing root-waveform. Creating one "
fi
if [ ! -e $OUTPUT/parameters ]; then
    mkdir $OUTPUT/parameters
    echo "Missing parameter. Creating one "
fi
if [ ! -e $OUTPUT/root-hist ]; then
    mkdir $OUTPUT/root-hist
    echo "Missing root-hist. Creating one "
fi
if [ ! -e $OUTPUT/root-track ]; then
    mkdir $OUTPUT/root-track
    echo "Missing root-track. Creating one "
fi
if [ ! -e $OUTPUT/eventdisplay ]; then
    mkdir $OUTPUT/eventdisplay
    echo "Missing eventdisplay. Creating one "
fi
if [ ! -e $OUTPUT/graph ]; then
    mkdir $OUTPUT/graph
    echo "Missing graph. Creating one "
fi
if [ ! -e $OUTPUT/root-sim ]; then
    mkdir $OUTPUT/root-sim
    echo "Missing root-sim. Creating one "
fi
if [ ! -e $OUTPUT/root-eff ]; then
    mkdir $OUTPUT/root-eff
    echo "Missing root-eff. Creating one "
fi
if [ ! -e $OUTPUT/log ]; then
    mkdir $OUTPUT/log
    echo "Missing log. Creating one "
fi
