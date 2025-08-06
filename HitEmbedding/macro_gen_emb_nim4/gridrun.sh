#!/bin/bash
RUN=$1
FNAME=$2
LIST_NAME=$3
N_EVT=$4

if [ -z "$CONDOR_DIR_INPUT" -o -z "$CONDOR_DIR_OUTPUT" ] ; then
    echo "!ERROR!  CONDOR_DIR_INPUT/OUTPUT is undefined.  Abort."
    exit 1
fi
echo "INPUT  = $CONDOR_DIR_INPUT"
echo "OUTPUT = $CONDOR_DIR_OUTPUT"
echo "HOST   = $HOSTNAME"
echo "PWD    = $PWD"

tar xzf $CONDOR_DIR_INPUT/input.tar.gz
source setup.sh

touch timestamp.txt # All files created after this will be moved to CONDOR_DIR_OUTPUT

time root -b -q "Fun4All.C($RUN, \"$CONDOR_DIR_INPUT/$FNAME\", \"$LIST_NAME\", $N_EVT)"
RET=$?
if [ $RET -ne 0 ] ; then
    echo "Error in Fun4All.C: $RET"
    exit $RET
fi

echo "$RET" >status.txt

find . -mindepth 1 -maxdepth 1 -newer timestamp.txt -exec mv {} $CONDOR_DIR_OUTPUT \;

echo "gridrun.sh finished!"
