#!/bin/bash
## Example script to check if each job output exists or not.

while read RUN SPILL ; do
    RUN6=$(printf '%06d' $RUN)
    SPILL9=$(printf '%09d' $SPILL)
    #echo "$RUN6 $SPILL9"
    DIR_JOB=data/reco/run_$RUN6/spill_$SPILL9
    if [ ! -e $DIR_JOB ] ; then
	echo "$DIR_JOB  No job directory"
	continue
    fi
    if [ ! -e $DIR_JOB/out/status.txt ] ; then
	#if [ -e $DIR_JOB/out/_joblogfile ] ; then
	#    grep 'HOST' $DIR_JOB/out/_joblogfile
	#fi
	echo "$DIR_JOB  No status file"
	continue
    fi
    FN_DST=run_${RUN6}_spill_${SPILL9}_spin_reco.root
    #FN_DST=output_PP.root
    if [ ! -e $DIR_JOB/out/$FN_DST ] ; then
	echo "$DIR_JOB  No DST file"
	continue
    fi
done <list_run_spill.txt
