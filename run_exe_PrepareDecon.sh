#!/bin/bash
source ~/.bashrc
source /cvmfs/juno.ihep.ac.cn/centos7_amd64_gcc830/Pre-Release/J20v2r0-Pre0/setup.sh
fullPath_SPE_file=$1
fullPath_PDE_file=$2

name_file=$(basename $fullPath_SPE_file)
echo "name_file : "$name_file
echo  "input path :" $fullPath_SPE_file $fullPath_PDE_file
./main.cpp_exe $fullPath_SPE_file $fullPath_PDE_file &&
env -i ./run_getFilterPY.sh $fullPath_SPE_file &&
 cd ./fiterWaves_jobs/ &&
./gen_filterWavesByStep_OnePMT.sh $fullPath_SPE_file && cd subjobs && ./${name_file}_sub.sh