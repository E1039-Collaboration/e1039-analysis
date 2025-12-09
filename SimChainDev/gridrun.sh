#!/bin/bash

nevents=$1
job_id=$2

if [ -z ${CONDOR_DIR_INPUT+x} ];
  then
    CONDOR_DIR_INPUT=./input;
    echo "CONDOR_DIR_INPUT is initiallized as $CONDOR_DIR_INPUT"
  else
    echo "CONDOR_DIR_INPUT is set to '$CONDOR_DIR_INPUT'";
fi

if [ -z ${CONDOR_DIR_OUTPUT+x} ];
  then
    CONDOR_DIR_OUTPUT=./out;
    mkdir -p $CONDOR_DIR_OUTPUT
    echo "CONDOR_DIR_OUTPUT is initiallized as $CONDOR_DIR_OUTPUT"
  else
    echo "CONDOR_DIR_OUTPUT is set to '$CONDOR_DIR_OUTPUT'";
fi

echo "hello, grid." | tee out.txt $CONDOR_DIR_OUTPUT/out.txt
echo "HOST = $HOSTNAME" | tee -a out.txt $CONDOR_DIR_OUTPUT/out.txt
pwd | tee -a out.txt $CONDOR_DIR_OUTPUT/out.txt

tar -xzvf $CONDOR_DIR_INPUT/input.tar.gz
ls -lh | tee -a out.txt $CONDOR_DIR_OUTPUT/out.txt

#FN_SETUP=/exp/seaquest/app/software/osg/software/e1039/this-e1039.sh
FN_SETUP=/exp/seaquest/app/software/osg/users/kenichi/e1039/core-20251203/this-e1039.sh
if [ ! -e $FN_SETUP ] ; then # On grid
    FN_SETUP=/cvmfs/seaquest.opensciencegrid.org/seaquest/${FN_SETUP#/exp/seaquest/app/software/osg/}
fi
echo "SETUP = $FN_SETUP"
source $FN_SETUP
echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH"

touch timestamp.txt # All files created after this will be moved to CONDOR_DIR_OUTPUT

time root -b -q Fun4Sim.C\($nevents\)
RET=$?
echo "$RET" >status.txt
if [ $RET -ne 0 ] ; then
    echo "Error in Fun4Sim.C: $RET"
    exit $RET
fi

find . -mindepth 1 -maxdepth 1 -newer timestamp.txt -exec mv {} $CONDOR_DIR_OUTPUT \;
#mv *.root $CONDOR_DIR_OUTPUT/

echo "gridrun.sh finished!"
