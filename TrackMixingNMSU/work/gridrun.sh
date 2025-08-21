#!/bin/bash
RUN=$1
FN_LIST_IN=$2
N_EVT=$3

if [ -z "$CONDOR_DIR_INPUT" -o -z "$CONDOR_DIR_OUTPUT" ] ; then
    echo "!ERROR!  CONDOR_DIR_INPUT/OUTPUT is undefined.  Abort."
    exit 1
fi
echo "INPUT  = $CONDOR_DIR_INPUT"
echo "OUTPUT = $CONDOR_DIR_OUTPUT"
echo "HOST   = $HOSTNAME"
echo "PWD    = $PWD"
echo "$FN_LIST_IN"
tar xzf $CONDOR_DIR_INPUT/input.tar.gz
source setup.sh
echo "E1039_CORE = $E1039_CORE"

touch timestamp.txt
time root.exe -b -l -q "DoTrackMixing.C($RUN, \"$CONDOR_DIR_INPUT\", \"$CONDOR_DIR_INPUT/$FN_LIST_IN\", $N_EVT)"
RET=$?
if [ $RET -ne 0 ] ; then
    echo "Error in Fun4All.C: $RET"
    exit $RET
fi

find . -mindepth 1 -maxdepth 1 -newer timestamp.txt -exec mv {} $CONDOR_DIR_OUTPUT \;

echo "gridrun.sh finished!"
