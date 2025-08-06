#!/bin/bash
DIR_MACRO=$(dirname $(readlink -f $BASH_SOURCE))
DIR_DST=/pnfs/e1039/persistent/users/spinquestpro/decoded_data/DST_NIM4

N_EVT=0
JOB_B=1
JOB_E=1
DO_OVERWRITE=no
USE_GRID=no
OPTIND=1
while getopts ":j:e:og" OPT ; do
    case $OPT in
        j )    JOB_E=$OPTARG ;;
        e )    N_EVT=$OPTARG ;;
        o ) DO_OVERWRITE=yes ;;
        g ) USE_GRID=yes     ;;
        * ) PrintHelp ; exit ;;
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
test -z "$JOB_B" || test $JOB_B -lt 1      && JOB_B=1
test -z "$JOB_E" || test $JOB_E -gt $N_RUN && JOB_E=$N_RUN
echo "FN_LIST      = $FN_LIST"
echo "N_RUN        = $N_RUN"
echo "N_EVT        = $N_EVT"
echo "JOB_B...E    = $JOB_B...$JOB_E"
echo "DO_OVERWRITE = $DO_OVERWRITE"
echo "USE_GRID     = $USE_GRID"

if [ $USE_GRID == yes ] ; then
    #DIR_WORK=/pnfs/e1039/persistent/users/$USER/data_emb_nim4
    DIR_WORK=/pnfs/e1039/scratch/users/$USER/HitEmbedding/data_emb_nim4
    ln -nfs $DIR_WORK data # for convenience
else
    DIR_WORK=$DIR_MACRO/scratch
fi
echo "DIR_WORK = $DIR_WORK"

cd $DIR_MACRO
mkdir -p $DIR_WORK
rm -f    $DIR_WORK/input.tar.gz
tar czf  $DIR_WORK/input.tar.gz  *.C *.txt ../setup.sh ../inst

for (( JOB_I = $JOB_B; JOB_I <= $JOB_E; JOB_I++ )) ; do
    RUN=${LIST_RUN[((JOB_I - 1))]}
    echo "JOB_I $JOB_I : $RUN"
    RUN6=$(printf "%06d" $RUN)
    FN_ROOT=run_${RUN6}_NIM4_spin.root
    
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
    
    if [ $USE_GRID == yes ] ; then
	CMD="/exp/seaquest/app/software/script/jobsub_submit_spinquest.sh"
	CMD+=" --expected-lifetime='short'" # medium=8h, short=3h, long=23h
	CMD+=" -L $DIR_WORK_JOB/log_gridrun.txt"
	CMD+=" -f $DIR_WORK/input.tar.gz"
	CMD+=" -f $DIR_DST/$FN_ROOT"
	CMD+=" -d OUTPUT $DIR_WORK_JOB/out"
	CMD+=" file://$DIR_WORK_JOB/gridrun.sh $RUN $FN_ROOT $FN_LIST $N_EVT"
	unbuffer $CMD |& tee $DIR_WORK_JOB/log_jobsub_submit.txt
	RET_SUB=${PIPESTATUS[0]}
	test $RET_SUB -ne 0 && exit $RET_SUB
    else
	export  CONDOR_DIR_INPUT=$DIR_WORK_JOB/in
	export CONDOR_DIR_OUTPUT=$DIR_WORK_JOB/out
	mkdir -p $DIR_WORK_JOB/in
	cp -p $DIR_WORK/input.tar.gz $DIR_WORK_JOB/in
	ln -nfs $DIR_DST/$FN_ROOT $DIR_WORK_JOB/in
	mkdir -p $DIR_WORK_JOB/exe
	cd $DIR_WORK_JOB/exe
	$DIR_WORK_JOB/gridrun.sh $RUN $FN_ROOT $FN_LIST $N_EVT |& tee $DIR_WORK_JOB/log_gridrun.txt
    fi
done

