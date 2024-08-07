#!/bin/bash

FN_BKG=$1
N_EVT=$2
KMAG_POL=$3 # +1 or -1
KMAG_SC=$4 # scaling factor

if [ -z "$CONDOR_DIR_INPUT" -o -z "$CONDOR_DIR_OUTPUT" ] ; then
    echo "!ERROR!  CONDOR_DIR_INPUT/OUTPUT is undefined.  Abort."
    exit 1
fi
echo "INPUT  = $CONDOR_DIR_INPUT"
echo "OUTPUT = $CONDOR_DIR_OUTPUT"
echo "HOST   = $HOSTNAME"
echo "PWD    = $PWD"

tar xzf $CONDOR_DIR_INPUT/input.tar.gz

FN_SETUP=/exp/seaquest/app/software/osg/software/e1039/this-e1039.sh
if [ ! -e $FN_SETUP ] ; then # On grid
    FN_SETUP=/cvmfs/seaquest.opensciencegrid.org/seaquest/${FN_SETUP#/exp/seaquest/app/software/osg/}
fi
echo "SETUP = $FN_SETUP"
source $FN_SETUP
export   LD_LIBRARY_PATH=inst/lib:$LD_LIBRARY_PATH
export ROOT_INCLUDE_PATH=inst/include:$ROOT_INCLUDE_PATH

time root -b -q "Fun4All.C(\"$CONDOR_DIR_INPUT/$FN_BKG\", $N_EVT, $KMAG_POL, $KMAG_SC)"
RET=$?
if [ $RET -ne 0 ] ; then
    echo "Error in Fun4All.C: $RET"
    exit $RET
fi

mv  bg_data.root *.tsv  $CONDOR_DIR_OUTPUT

echo "gridrun.sh finished!"
