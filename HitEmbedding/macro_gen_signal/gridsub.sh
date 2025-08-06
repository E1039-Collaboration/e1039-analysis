#!/bin/bash
DIR_MACRO=$(dirname $(readlink -f $BASH_SOURCE))

DO_OVERWRITE=no
USE_GRID=no
GEN_MODE=dy # dy, jpsi or psip
JOB_B=1
JOB_E=1
N_EVT=100
OPTIND=1
while getopts ":ogm:j:e:" OPT ; do
    case $OPT in
	o ) DO_OVERWRITE=yes ;;
        g ) USE_GRID=yes ;;
	m ) GEN_MODE=$OPTARG ;;
        j ) JOB_E=$OPTARG ;;
        e ) N_EVT=$OPTARG ;;
    esac
done
shift $((OPTIND - 1))
JOB_NAME=$1
test -z $JOB_NAME && echo "Need a job name.  Abort." && exit

if [ "${JOB_E%-*}" != "$JOB_E" ] ; then # Contain '-'
    JOB_B=${JOB_E%-*} # Before '-'
    JOB_E=${JOB_E#*-} # After '-'
fi

echo "GEN_MODE     = $GEN_MODE"
echo "JOB_NAME     = $JOB_NAME"
echo "DO_OVERWRITE = $DO_OVERWRITE"
echo "USE_GRID     = $USE_GRID"
echo "JOB_B...E    = $JOB_B...$JOB_E"
echo "N_EVT        = $N_EVT"
if [ $USE_GRID == yes ]; then
    DIR_DATA=/pnfs/e1039/scratch/users/$USER/HitEmbedding/data_signal
    DIR_WORK=$DIR_DATA/${GEN_MODE}_$JOB_NAME
    ln -nfs $DIR_DATA data # for convenience
else
    DIR_WORK=$DIR_MACRO/scratch/${GEN_MODE}_$JOB_NAME
fi

cd $DIR_MACRO
mkdir -p $DIR_WORK
rm -f    $DIR_WORK/input.tar.gz
tar czf  $DIR_WORK/input.tar.gz  *.C

for (( JOB_I = $JOB_B; JOB_I <= $JOB_E; JOB_I++ )) ; do
    DIR_WORK_JOB=$DIR_WORK/$(printf "%04d" $JOB_I)
    if [ -e $DIR_WORK_JOB ] ; then
	echo -n "  DIR_WORK_JOB already exists."
	if [ $DO_OVERWRITE = yes ] ; then
	    echo "  Clean up."
	    rm -rf $DIR_WORK_JOB
	else
	    echo "  Skip."
	    continue
	fi
    fi

    mkdir -p $DIR_WORK_JOB/out
    cp -p $DIR_MACRO/gridrun.sh $DIR_WORK_JOB
    
    if [ $USE_GRID == yes ]; then
	CMD="/exp/seaquest/app/software/script/jobsub_submit_spinquest.sh"
	CMD+=" --expected-lifetime='long'" # medium=8h, short=3h, long=23h
	CMD+=" -L $DIR_WORK_JOB/log_gridrun.txt"
	CMD+=" -f $DIR_WORK/input.tar.gz"
	CMD+=" -d OUTPUT $DIR_WORK_JOB/out"
	CMD+=" file://$DIR_WORK_JOB/gridrun.sh $GEN_MODE $JOB_I $N_EVT"
	unbuffer $CMD |& tee $DIR_WORK_JOB/log_jobsub_submit.txt
	RET_SUB=${PIPESTATUS[0]}
	test $RET_SUB -ne 0 && exit $RET_SUB
    else
	export  CONDOR_DIR_INPUT=$DIR_WORK_JOB/in
	export CONDOR_DIR_OUTPUT=$DIR_WORK_JOB/out
	mkdir -p $DIR_WORK_JOB/in
	cp -p $DIR_WORK/input.tar.gz $DIR_WORK_JOB/in
	cd $DIR_WORK_JOB
	$DIR_WORK_JOB/gridrun.sh $GEN_MODE $JOB_I $N_EVT |& tee $DIR_WORK_JOB/log_gridrun.txt
    fi
done
