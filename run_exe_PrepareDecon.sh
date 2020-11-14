#!/bin/bash
fullPath_SPE_file=$1
fullPath_PDE_file=$2
prefix_filter=$(basename $fullPath_SPE_file)
name_file=$(basename $fullPath_SPE_file)
echo "name_file : "$name_file
echo  "input path :" $fullPath_SPE_file $fullPath_PDE_file
#./main.cpp_exe $fullPath_SPE_file $fullPath_PDE_file &&
#env -i ./run_getFilterPY.sh $fullPath_SPE_file &&
env -i ./run_getFilterPY.sh $fullPath_SPE_file && cd ./fiterWaves_jobs/ && ./gen_filterWavesByStep_OnePMT.sh $fullPath_SPE_file $prefix_filter && cd subjobs && ./${name_file}_sub.sh


