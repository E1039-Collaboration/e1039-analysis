#!/bin/bash
DIR_MACRO=$(dirname $(readlink -f $BASH_SOURCE))
#DIR_DST=/pnfs/e1039/scratch/users/$USER/RecoData2024/reco
#DIR_DST=/pnfs/e1039/persistent/users/lcalerod/RecoData2024/phys_pos/reco
#DIR_DST=/pnfs/e1039/persistent/users/kenichi/RecoData2024/reco-20250619
DIR_DST=/pnfs/e1039/persistent/users/kenichi/RecoData2024/reco-20250804

JOB_NAME=ana
DO_OVERWRITE=no
USE_GRID=no
FORCE_PNFS=no
JOB_B=1
JOB_E=1
N_EVT=0
N_JOB_MAX=0 # N of max jobs at a time.  0 = no limit
OPTIND=1
while getopts ":n:ogpj:e:m:" OPT ; do
    case $OPT in
	n ) JOB_NAME=$OPTARG ;;
	o ) DO_OVERWRITE=yes ;;
        g ) USE_GRID=yes ;;
        p ) FORCE_PNFS=yes ;;
        j ) JOB_E=$OPTARG ;;
        e ) N_EVT=$OPTARG ;;
	m ) N_JOB_MAX=$OPTARG ;;
    esac
done
shift $((OPTIND - 1))

FN_LIST=list_run_spill.txt
LIST_RUN=( $(cut -f 1 $FN_LIST | uniq) )
N_RUN=${#LIST_RUN[*]}

if [ "${JOB_E%-*}" != "$JOB_E" ] ; then # Contain '-'
    JOB_B=${JOB_E%-*} # Before '-'
    JOB_E=${JOB_E#*-} # After '-'
fi
test -z $JOB_B || test $JOB_B -lt 1      && JOB_B=1
test -z $JOB_E || test $JOB_E -gt $N_RUN && JOB_E=$N_RUN

echo "N_RUN        = $N_RUN"
echo "JOB_NAME     = $JOB_NAME"
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
tar czf  $DIR_WORK/input.tar.gz  $FN_LIST *.C ../setup.sh ../inst

for (( JOB_I = $JOB_B; JOB_I <= $JOB_E; JOB_I++ )) ; do
    RUN=${LIST_RUN[((JOB_I - 1))]}
    echo "JOB_I $JOB_I : $RUN"
    RUN6=$(printf "%06d" $RUN)
    DIR_WORK_JOB=$DIR_WORK/run_$RUN6
    if [ -e $DIR_WORK_JOB ] ; then
	echo -n "  DIR_WORK_JOB already exists."
	if [ $DO_OVERWRITE = yes ] ; then
	    echo "  Clean up (-o)."
	    rm -rf $DIR_WORK_JOB
	elif [ ! -e $DIR_WORK_JOB/out/status.txt ] ; then
	    echo "  Clean up (no status file)."
	    rm -rf $DIR_WORK_JOB
	else
	    echo "  Skip."
	    continue
	fi
    fi
    
    mkdir -p $DIR_WORK_JOB/out
    cp -p $DIR_MACRO/gridrun.sh $DIR_WORK_JOB

    mkdir -p $DIR_WORK_JOB/in
    FN_LIST_IN=list_input.txt
    for SPILL in $(awk "{if (\$1==$RUN) print \$2;}" $DIR_MACRO/$FN_LIST) ; do
	SPILL9=$(printf '%09d' $SPILL)
	BASE_NAME=run_${RUN6}_spill_${SPILL9}_spin_reco
	test -e $DIR_DST/run_$RUN6/spill_$SPILL9/out/$BASE_NAME.root || continue
	echo -e "$SPILL\t$BASE_NAME"
    done >$DIR_WORK_JOB/in/$FN_LIST_IN
    
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
	CMD+=" --resource-provides=usage_model=DEDICATED,OPPORTUNISTIC"
	CMD+=" --expected-lifetime='short'" # medium=8h, short=3h, long=23h
	CMD+=" -L $DIR_WORK_JOB/log_gridrun.txt"
	CMD+=" -f $DIR_WORK/input.tar.gz"
	CMD+=" -f $DIR_WORK_JOB/in/$FN_LIST_IN"
	while read SPILL BASE_NAME ; do
	    SPILL9=$(printf '%09d' $SPILL)
	    CMD+=" -f $DIR_DST/run_$RUN6/spill_$SPILL9/out/$BASE_NAME.root"
	done <$DIR_WORK_JOB/in/$FN_LIST_IN
	CMD+=" -d OUTPUT $DIR_WORK_JOB/out"
	CMD+=" file://$DIR_WORK_JOB/gridrun.sh $RUN $FN_LIST_IN $N_EVT"
	#echo "CMD = $CMD"
	unbuffer $CMD |& tee $DIR_WORK_JOB/log_jobsub_submit.txt
	RET_SUB=${PIPESTATUS[0]}
	test $RET_SUB -ne 0 && exit $RET_SUB
    else
	export  CONDOR_DIR_INPUT=$DIR_WORK_JOB/in
	export CONDOR_DIR_OUTPUT=$DIR_WORK_JOB/out
	cp -p $DIR_WORK/input.tar.gz $DIR_WORK_JOB/in
	while read SPILL BASE_NAME ; do
	    SPILL9=$(printf '%09d' $SPILL)
	    ln -s $DIR_DST/run_$RUN6/spill_$SPILL9/out/$BASE_NAME.root $DIR_WORK_JOB/in/$BASE_NAME.root
	done <$DIR_WORK_JOB/in/$FN_LIST_IN
	mkdir -p $DIR_WORK_JOB/exe
	cd $DIR_WORK_JOB/exe
	$DIR_WORK_JOB/gridrun.sh $RUN $FN_LIST_IN $N_EVT |& tee $DIR_WORK_JOB/log_gridrun.txt
    fi
done
