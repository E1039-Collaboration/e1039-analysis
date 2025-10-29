#!/bin/bash
DIR_MACRO=$(dirname $(readlink -f $BASH_SOURCE))
DIR_DST=/pnfs/e1039/scratch/mhossain/vec_data_in/BT
LIST_VEC=list_vec.txt # File containing the list of input files
JOB_NAME=Test_Jan11_2025
DO_OVERWRITE=no
USE_GRID=no
FORCE_PNFS=no
N_EVT=100
N_JOB_MAX=0
OPTIND=1
while getopts ":n:ogpm:j:" OPT ; do
    case $OPT in
        n ) JOB_NAME=$OPTARG ;;
        o ) DO_OVERWRITE=yes ;;
        g ) USE_GRID=yes ;;
        p ) FORCE_PNFS=yes ;;
        m ) N_JOB_MAX=$OPTARG ;;
        j ) JOB_RANGE=$OPTARG ;;  # Capture the job range
    esac
done
shift $((OPTIND - 1))

# Read the input file list and calculate N_FILES
declare -a INPUT_FILES
while read -r FILE_PATH; do
    INPUT_FILES+=("$FILE_PATH")
done <"$LIST_VEC"
N_FILES=${#INPUT_FILES[@]}

# Parse the job range
START_JOB=0
END_JOB=$((N_FILES - 1))  # Default to processing all files

if [ -n "$JOB_RANGE" ]; then
    IFS='-' read -r START_JOB END_JOB <<< "$JOB_RANGE"
    if (( START_JOB < 0 || END_JOB >= N_FILES || START_JOB > END_JOB )); then
        echo "Error: Invalid job range ($START_JOB > $END_JOB or out of bounds)."
        exit 1
    fi  
fi

# Process files in the specified range
echo "Processing files from index $START_JOB to $END_JOB..."
for (( i = START_JOB; i <= END_JOB; i++ )); do
    FILE=${INPUT_FILES[i]}
    echo "Processing file: $FILE"
    # Add your processing commands here, for example:
    # ./my_program "$FILE"
done


echo "N_FILES      = $N_FILES"
echo "JOB_NAME     = $JOB_NAME"
echo "DO_OVERWRITE = $DO_OVERWRITE"
echo "USE_GRID     = $USE_GRID"
echo "FORCE_PNFS   = $FORCE_PNFS"
echo "N_JOB_MAX    = $N_JOB_MAX"

##
## Prepare and execute the job submission
##
if [ $USE_GRID == yes -o $FORCE_PNFS == yes ]; then
    #DIR_DATA=/pnfs/e1039/scratch/mhossain/vec_data_in
    DIR_DATA=/pnfs/e1039/scratch/users/$USER/vec_data_in
    DIR_WORK=$DIR_DATA/$JOB_NAME
    ln -nfs $DIR_DATA data # for convenience
else
    DIR_WORK=$DIR_MACRO/scratch/$JOB_NAME
fi

cd $DIR_MACRO
mkdir -p $DIR_WORK
rm -f    $DIR_WORK/input.tar.gz
tar czf  $DIR_WORK/input.tar.gz  config *.C ../setup.sh ../inst

#for (( FILE_I = 0; FILE_I < $N_FILES; FILE_I++ )) ; do
for (( FILE_I = START_JOB; FILE_I <= END_JOB && FILE_I < N_FILES; FILE_I++ )) ; do

    FILE_PATH=${INPUT_FILES[$FILE_I]}
    FILE_NAME=$(basename "$FILE_PATH")
    echo "Processing file: $FILE_NAME"
    BASE_NAME=${FILE_NAME%.*}
    OUT_FILE="out_${BASE_NAME}.root"

    DIR_WORK_JOB=$DIR_WORK/job_${FILE_I}
    if [ -e $DIR_WORK_JOB ] ; then
        echo -n "  DIR_WORK_JOB already exists."
        if [ $DO_OVERWRITE = yes ] ; then
            echo "  Clean up (-o)."
            rm -rf $DIR_WORK_JOB
        else
            echo "  Skip."
            continue
        fi
    fi

    mkdir -p $DIR_WORK_JOB/out
    cp -p $DIR_MACRO/gridrun.sh $DIR_WORK_JOB
    echo "DIR_WORK_JOB is: $DIR_WORK_JOB"

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
        CMD+=" --expected-lifetime='long'"
        CMD+=" -L $DIR_WORK_JOB/log_gridrun.txt"
        CMD+=" -f $DIR_WORK/input.tar.gz"
        CMD+=" -f $FILE_PATH"
        CMD+=" -d OUTPUT $DIR_WORK_JOB/out"
	CMD+=" file://$DIR_WORK_JOB/gridrun.sh $FILE_NAME $OUT_FILE $N_EVT"
        unbuffer $CMD |& tee $DIR_WORK_JOB/log_jobsub_submit.txt
        RET_SUB=${PIPESTATUS[0]}
        test $RET_SUB -ne 0 && exit $RET_SUB
    else
        export  CONDOR_DIR_INPUT=$DIR_WORK_JOB/in
        export CONDOR_DIR_OUTPUT=$DIR_WORK_JOB/out
        mkdir -p $DIR_WORK_JOB/in
        cp -p $DIR_WORK/input.tar.gz $DIR_WORK_JOB/in
        ln -s $FILE_PATH $DIR_WORK_JOB/in/$FILE_NAME
        mkdir -p $DIR_WORK_JOB/exe
        cd $DIR_WORK_JOB/exe
	$DIR_WORK_JOB/gridrun.sh $FILE_NAME $OUT_FILE $N_EVT |& tee $DIR_WORK_JOB/log_gridrun.txt

    fi
done

