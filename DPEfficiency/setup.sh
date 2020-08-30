DIR_TOP=$(dirname $(readlink -f $BASH_SOURCE))
export LD_LIBRARY_PATH=$DIR_TOP/AnaModule/lib:$LD_LIBRARY_PATH

function rebuild-ana {
    CURRDIR=`pwd`
    cd $DIR_TOP/AnaModule
    mkdir -p build
    cd build
    make clean
    #rm -rf *
    cmake -DCMAKE_INSTALL_PREFIX=$DIR_TOP/AnaModule ..
    make install
    ret=$?
    test $ret -eq 0 && echo "Re-build done'."
    cd $CURRDIR
}
