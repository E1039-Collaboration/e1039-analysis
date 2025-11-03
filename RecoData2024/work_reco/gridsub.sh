#!/bin/bash
DIR_MACRO=$(dirname $(readlink -f $BASH_SOURCE))
DIR_DST=/pnfs/e1039/persistent/users/kenichi/dst

JOB_NAME=reco
FN_LIST=list_run_spill.txt
DO_OVERWRITE=no
USE_GRID=no
FORCE_PNFS=no
JOB_B=1
JOB_E=1 # 0 = All available signal and/or embedding files
N_EVT=0 # 0 = All events in each signal+embedding file
N_JOB_MAX=0 # N of max jobs at a time.  0 = no limit
OPTIND=1
while getopts ":n:l:ogpj:e:m:" OPT ; do
    case $OPT in
	n ) JOB_NAME=$OPTARG ;;
	l ) FN_LIST=$OPTARG ;;
	o ) DO_OVERWRITE=yes ;;
        g ) USE_GRID=yes ;;
	p ) FORCE_PNFS=yes ;;
        j ) JOB_E=$OPTARG ;;
        e ) N_EVT=$OPTARG ;;
        m ) N_JOB_MAX=$OPTARG ;;
    esac
done
shift $((OPTIND - 1))

declare -a LIST_RUN
declare -a LIST_SPILL
while read RUN SPILL ; do
      LIST_RUN+=($RUN)
    LIST_SPILL+=($SPILL)
done <$FN_LIST
N_DAT=${#LIST_RUN[*]}

if [ "${JOB_E%-*}" != "$JOB_E" ] ; then # Contain '-'
    JOB_B=${JOB_E%-*} # Before '-'
    JOB_E=${JOB_E#*-} # After '-'
fi
test -z $JOB_B || test $JOB_B -lt 1      && JOB_B=1
test -z $JOB_E || test $JOB_E -gt $N_DAT && JOB_E=$N_DAT

echo "JOB_NAME     = $JOB_NAME"
echo "FN_LIST      = $FN_LIST"
echo "N_DAT        = $N_DAT"
echo "DO_OVERWRITE = $DO_OVERWRITE"
echo "USE_GRID     = $USE_GRID"
echo "FORCE_PNFS   = $FORCE_PNFS"
echo "JOB_B...E    = $JOB_B...$JOB_E"
echo "N_EVT        = $N_EVT"
echo "N_JOB_MAX    = $N_JOB_MAX"

##
## Prepare and execute the job submission
##
if [ $USE_GRID == yes -o $FORCE_PNFS == yes ]; then
    DIR_DATA=/pnfs/e1039/scratch/users/$USER/RecoData2024
    DIR_WORK=$DIR_DATA/$JOB_NAME
    ln -nfs $DIR_DATA data # for convenience
else
    DIR_WORK=$DIR_MACRO/scratch/$JOB_NAME
fi

cd $DIR_MACRO
mkdir -p $DIR_WORK
rm -f    $DIR_WORK/input.tar.gz
tar czf  $DIR_WORK/input.tar.gz  config *.C ../setup.sh ../inst

for (( JOB_I = $JOB_B; JOB_I <= $JOB_E; JOB_I++ )) ; do
    RUN=${LIST_RUN[((JOB_I - 1))]}
    SPILL=${LIST_SPILL[((JOB_I - 1))]}
    echo "JOB_I $JOB_I : $RUN $SPILL"
    RUN6=$(printf "%06d" $RUN)
    SPILL9=$(printf "%09d" $SPILL)
    FN_IN=run_${RUN6}_spill_${SPILL9}_spin.root
    if [ ! -e $DIR_DST/run_$RUN6/$FN_IN ] ; then
	echo "  No input DST file.  Skip."
	continue
    fi

    DIR_WORK_JOB=$DIR_WORK/run_$RUN6/spill_$SPILL9
    if [ -e $DIR_WORK_JOB ] ; then
	echo -n "  DIR_WORK_JOB already exists."
	if [ $DO_OVERWRITE = yes ] ; then
	    echo "  Clean up (-o)."
	    rm -rf $DIR_WORK_JOB
	elif [ ! -e $DIR_WORK_JOB/out/status.txt ] ; then
	    echo "  No status file.  Clean up."
	    rm -rf $DIR_WORK_JOB
	elif [ ! -e $DIR_WORK_JOB/out/run_${RUN6}_spill_${SPILL9}_spin_reco.root ] ; then
	    echo "  No DST file.  Clean up."
	    rm -rf $DIR_WORK_JOB
	else
	    echo "  Skip."
	    continue
	fi
    fi
    
    mkdir -p $DIR_WORK_JOB/out
    cp -p $DIR_MACRO/gridrun.sh $DIR_WORK_JOB
    
    if [ $USE_GRID == yes ]; then
	if [ $N_JOB_MAX -gt 0 ] ; then
	    while true ; do
		N_JOB=$(jobsub_q --group spinquest --user=$USER | grep 'gridrun.sh' | wc -l)
		test $N_JOB -lt $N_JOB_MAX && break
		echo "    N_JOB = $N_JOB >= $N_JOB_MAX.  Sleep 600..."
		sleep 600
	    done
	fi
	CMD="/exp/seaquest/app/software/script/jobsub_submit_spinquest.sh"
	#CMD+=" --resource-provides=usage_model=DEDICATED,OPPORTUNISTIC"
	#CMD+=" --expected-lifetime='medium'" # medium=8h, short=3h, long=23h
	CMD+=" --expected-lifetime='96h'"
	CMD+=" -L $DIR_WORK_JOB/log_gridrun.txt"
	CMD+=" -f $DIR_WORK/input.tar.gz"
	CMD+=" -f $DIR_DST/run_$RUN6/$FN_IN"
	CMD+=" -d OUTPUT $DIR_WORK_JOB/out"
	CMD+=" file://$DIR_WORK_JOB/gridrun.sh $RUN $SPILL $N_EVT"
	#echo "CMD = $CMD"
	unbuffer $CMD |& tee $DIR_WORK_JOB/log_jobsub_submit.txt
	RET_SUB=${PIPESTATUS[0]}
	test $RET_SUB -ne 0 && exit $RET_SUB
    else
	export  CONDOR_DIR_INPUT=$DIR_WORK_JOB/in
	export CONDOR_DIR_OUTPUT=$DIR_WORK_JOB/out
	mkdir -p $DIR_WORK_JOB/in
	cp -p $DIR_WORK/input.tar.gz $DIR_WORK_JOB/in
	ln -s $DIR_DST/run_$RUN6/$FN_IN $DIR_WORK_JOB/in/$FN_IN
	mkdir -p $DIR_WORK_JOB/exe
	cd $DIR_WORK_JOB/exe
	$DIR_WORK_JOB/gridrun.sh $RUN $SPILL $N_EVT |& tee $DIR_WORK_JOB/log_gridrun.txt
    fi
done
