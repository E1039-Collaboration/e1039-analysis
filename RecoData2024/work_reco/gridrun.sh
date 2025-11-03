#!/bin/bash
RUN=$1
SPILL=$2
N_EVT=$3

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

FN_IN=$(printf 'run_%06d_spill_%09d_spin.root' $RUN $SPILL)
FN_OUT=$(printf 'run_%06d_spill_%09d_spin_reco.root' $RUN $SPILL)
time root -l -b -q "Fun4All.C($RUN, $SPILL, \"$CONDOR_DIR_INPUT/$FN_IN\", \"$FN_OUT\", $N_EVT)"
#time root -l -b -q "Fun4AllTracklet.C($RUN, $SPILL, \"$CONDOR_DIR_INPUT/$FN_IN\", \"$FN_OUT\", $N_EVT)"
RET=$?
if [ $RET -ne 0 ] ; then
    echo "Error in Fun4All.C: $RET"
    exit $RET
fi
echo "$RET" >status.txt

find . -mindepth 1 -maxdepth 1 -newer timestamp.txt -exec mv {} $CONDOR_DIR_OUTPUT \;

echo "gridrun.sh finished!"
