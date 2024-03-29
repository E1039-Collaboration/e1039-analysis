DIR_TOP=$(dirname $(readlink -f $BASH_SOURCE))

source /exp/seaquest/app/software/osg/software/e1039/this-e1039.sh
#source /exp/seaquest/app/software/osg/users/$USER/e1039/core/this-e1039.sh
export LD_LIBRARY_PATH=$DIR_TOP/inst/lib:$LD_LIBRARY_PATH

function cmake-trigger-ana {
    cmake -DCMAKE_INSTALL_PREFIX=$DIR_TOP/inst $DIR_TOP/src
    ret=$?
    test $ret -eq 0 && echo "OK, run 'make install'."
}

if [ ${HOSTNAME:0:13} != 'spinquestgpvm' ] ; then
    echo "!!CAUTION!!"
    echo "This TriggerAna package does not support your computer ($HOSTNAME)."
    echo "It might not run properly even if you follow 'README.md'."
    echo
fi
