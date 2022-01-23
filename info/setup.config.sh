COMPUTER_NAME=$1
if [ $# -eq 0 ];then
    echo "Usage"
    echo " source setup.config.sh <option>"
    echo " - options_list"
    echo "   [kekcc][samlocal]"
fi

# KEKCC
if [ $COMPUTER_NAME == "kekcc" ];then
    echo $COMPUTER_NAME
    ln -sf $CCWORKING_DIR/info/experiment_setups/experiment-kekcc.config $CCWORKING_DIR/info/experiemnt-config.txt
fi
# LOCAL SAM
if [ $COMPUTER_NAME == "samlocal" ];then
    echo $COMPUTER_NAME
    ln -sf $CCWORKING_DIR/info/experiment_setups/experiment-local_sam.config $CCWORKING_DIR/info/experiemnt-config.txt
fi
