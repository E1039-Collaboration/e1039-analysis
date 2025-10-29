#!/bin/bash
IN_FILE=$1      # Input file name (passed dynamically from gridsub.sh)
OUT_FILE=$2     # Output file name (passed dynamically from gridsub.sh)
N_EVT=$3      # Number of events


if [ -z "$CONDOR_DIR_INPUT" -o -z "$CONDOR_DIR_OUTPUT" ] ; then
    echo "!ERROR!  CONDOR_DIR_INPUT/OUTPUT is undefined.  Abort."
    exit 1
fi
# Debug: Print environment and arguments
echo "INPUT  = $CONDOR_DIR_INPUT"
echo "OUTPUT = $CONDOR_DIR_OUTPUT"
echo "HOST   = $HOSTNAME"
echo "PWD    = $PWD"
echo "IN_FILE = $IN_FILE"
echo "OUT_FILE = $OUT_FILE"

tar xzf $CONDOR_DIR_INPUT/input.tar.gz
source setup.sh

touch timestamp.txt # All files created after this will be moved to CONDOR_DIR_OUTPUT

# Run the processing
time root -l -b -q "Fun4Sim.C(\"$CONDOR_DIR_INPUT/$IN_FILE\", \"$OUT_FILE\", $N_EVT)"

RET=$?
if [ $RET -ne 0 ] ; then
    echo "Error in Fun4All.C: $RET"
    exit $RET
fi
echo "$RET" >status.txt

# Move output files created after the timestamp to the output directory
find . -mindepth 1 -maxdepth 1 -newer timestamp.txt -exec mv {} $CONDOR_DIR_OUTPUT \;

echo "gridrun.sh finished!"
