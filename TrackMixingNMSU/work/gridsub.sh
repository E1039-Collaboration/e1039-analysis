#!/bin/bash
DIR_MACRO=$(dirname $(readlink -f $BASH_SOURCE))
#DIR_DST=/pnfs/e1039/scratch/users/harshaka/RecoData2024/reco
#DIR_DST=/pnfs/e1039/persistent/users/kenichi/RecoData2024/reco-202503
DIR_DST=/pnfs/e1039/persistent/users/kenichi/RecoData2024/reco-20250804
#DIR_DST=/pnfs/e1039/scratch/users/kenichi/RecoData2024/reco-empty

##
## Functions
##
function PrintHelp {
    cat <<-EOF
	$(basename $0):  Script to execute the 1st analysis step on local or GRID.

	Typical usage:
	  Default : Anlayze all the runs and spills listed in "list_run_spill.txt" locally.

	  $0 -n 4-100
	  Analyze the 4th-100th runs on local computer.

	  $0 -g -n 101-
	  Analyze the 101th-last runs on grid.

	Options:
	  -l list_run.txt   | Read the run list from 'list_run.txt'.
	  -n 4-100          | Set the number (range) of the runs to be analyzed.
	  -o                | Allow to overwrite existing output files.
	  -g                | Use the Grid computing.
	EOF
}

##
## Main
##
JOB_NAME=default # combinatoric_bkg
FN_LIST=list_run_spill.txt
N_EVT=0
N_SP_MAX=0 # N of spills analyzed per run
RUN_B=1
RUN_E=1
DO_OVERWRITE=no
USE_GRID=no
OPTIND=1
while getopts ":l:n:e:s:og" OPT ; do
    case $OPT in
        l )  FN_LIST=$OPTARG ;;
        n )    RUN_E=$OPTARG ;;
        e )    N_EVT=$OPTARG ;;
        s ) N_SP_MAX=$OPTARG ;;
        o ) DO_OVERWRITE=yes ;;
        g ) USE_GRID=yes     ;;
        * ) PrintHelp ; exit ;;
    esac
done
shift $((OPTIND - 1))

if [ $USE_GRID = yes ] ; then
    echo "USE_GRID is not supported yet.  Abort."
    exit
fi

##
## Make a list of runs to be analyzed
##
echo "Runs to be analyzed:"
LIST_RUN=( $(cut -f 1 $FN_LIST | uniq) )
N_RUN=${#LIST_RUN[*]}

if [ "${RUN_E%-*}" != "$RUN_E" ] ; then # Contain '-'
    RUN_B=${RUN_E%-*} # Before '-'
    RUN_E=${RUN_E#*-} # After '-'
fi
test -z $RUN_B || test $RUN_B -lt 1      && RUN_B=1
test -z $RUN_E || test $RUN_E -gt $N_RUN && RUN_E=$N_RUN

echo "N_RUN        = $N_RUN"
echo "N_EVT        = $N_EVT"
echo "N_SP_MAX     = $N_SP_MAX"
echo "RUN_B...E    = $RUN_B...$RUN_E"
echo "DO_OVERWRITE = $DO_OVERWRITE"
echo "USE_GRID     = $USE_GRID"

##
## Set up the working directory
##

if [ $USE_GRID == yes ]; then
    DIR_DATA=/pnfs/e1039/scratch/users/$USER/TrackMixingNMSU
    DIR_WORK=$DIR_DATA/$JOB_NAME
    ln -nfs $DIR_DATA data # for convenience
else
    DIR_WORK=$DIR_MACRO/scratch
fi

echo "DIR_WORK = $DIR_WORK"

cd $DIR_MACRO
mkdir -p $DIR_WORK
rm -f    $DIR_WORK/input.tar.gz
tar czf  $DIR_WORK/input.tar.gz  *.C ../setup.sh ../inst

for (( RUN_I = $RUN_B ; RUN_I <= $RUN_E ; RUN_I++ )) ; do
	RUN=${LIST_RUN[((RUN_I-1))]}
    	RUN6=$(printf "%06d" $RUN)
    	DIR_WORK_RUN=$DIR_WORK/run_$RUN6
	
	if [ -e $DIR_WORK_RUN ] ; then
                echo -n "  DIR_WORK_RUN already exists."
                if [ $DO_OVERWRITE = yes ] ; then
                	echo "  Clean up."
                        rm -rf $DIR_WORK_RUN
                else
                        echo "  Skip."
                        continue
                fi
        fi

	LIST_SPILL=( $(awk "{if (\$1==$RUN) print \$2;}" $DIR_MACRO/$FN_LIST) )
	if [ $N_SP_MAX -ne 0 -a $N_SP_MAX -lt ${#LIST_SPILL[*]} ] ; then
	    LIST_SPILL=( ${LIST_SPILL[@]:0:$N_SP_MAX-2} )
	fi
	
        mkdir -p $DIR_WORK_RUN/out
	cp -p $DIR_MACRO/gridrun.sh $DIR_WORK_RUN

	##
	## Loop over the spills
	##
	FN_LIST_IN=list_input.txt
	for SPILL in ${LIST_SPILL[*]} ; do
	    SPILL9=$(printf '%09d' $SPILL)
	    FNAME=run_${RUN6}_spill_${SPILL9}_spin_reco.root
	    test -e $DIR_DST/run_$RUN6/spill_$SPILL9/out/$FNAME || continue
	    echo -e "$SPILL\t$FNAME"
	done >$DIR_WORK_RUN/$FN_LIST_IN

    	echo "----------------------------------------------------------------"

    	if [ $USE_GRID == yes ] ; then
	    echo "Not supported."
		#if [ $N_JOB_MAX -gt 0 ] ; then
		#    	while true ; do
		#		N_JOB=$(jobsub_q --group spinquest --user=$USER | grep 'gridrun.sh' | wc -l)
		#		test $N_JOB -lt $N_JOB_MAX && break
		#		echo "    N_JOB = $N_JOB >= $N_JOB_MAX.  Sleep 600..."
		#		sleep 600
	    	#	done
		#fi
		#CMD="/exp/seaquest/app/software/script/jobsub_submit_spinquest.sh"
		#CMD+=" --resource-provides=usage_model=DEDICATED,OPPORTUNISTIC"
		#CMD+=" --expected-lifetime='short'" # medium=8h, short=3h, long=23h
		#CMD+=" -L $DIR_WORK_RUN/log_gridrun.txt"
		#CMD+=" -f $DIR_WORK/input.tar.gz"
		#CMD+=" -f $DIR_WORK_JOB/in/$FN_LIST_IN"
		#while read SPILL FNAME ; do
                #        ln -s $DIR_DST/run_$RUN6/run_${RUN6}_spill_00${SPILL}_spin/out/DST.root $DIR_WORK_RUN/in/$FNAME
                #        #ln -s $DIR_DST/run_$RUN6/spill_00$SPILL/out/$FNAME $DIR_WORK_RUN/in/$FNAME
                #done <$DIR_WORK_RUN/$FN_LIST_IN
		#CMD+=" -d OUTPUT $DIR_WORK_RUN/out"
		#CMD+=" file://$DIR_WORK_RUN/gridrun.sh $RUN $FN_LIST_IN $N_EVT"
		#$CMD |& tee $DIR_WORK_RUN/log_jobsub_submit.txt
		#RET_SUB=${PIPESTATUS[0]}
		#test $RET_SUB -ne 0 && exit $RET_SUB
    	else
	    export  CONDOR_DIR_INPUT=$DIR_WORK_RUN/in
	    export CONDOR_DIR_OUTPUT=$DIR_WORK_RUN/out
	    mkdir -p $DIR_WORK_RUN/in
	    cp -p $DIR_WORK/input.tar.gz $DIR_WORK_RUN/in
	    cp -a $DIR_WORK_RUN/$FN_LIST_IN $DIR_WORK_RUN/in
	    while read SPILL FNAME ; do
		SPILL9=$(printf '%09d' $SPILL)
		ln -s $DIR_DST/run_$RUN6/spill_$SPILL9/out/$FNAME $DIR_WORK_RUN/in/$FNAME
           	#ln -s $DIR_DST/run_$RUN6/spill_00$SPILL/out/$FNAME $DIR_WORK_RUN/in/$FNAME
            done <$DIR_WORK_RUN/$FN_LIST_IN
	    mkdir -p $DIR_WORK_RUN/exe
	    cd       $DIR_WORK_RUN/exe
	    $DIR_WORK_RUN/gridrun.sh $RUN $FN_LIST_IN $N_EVT |& tee $DIR_WORK_RUN/log_gridrun.txt
    	fi
done
