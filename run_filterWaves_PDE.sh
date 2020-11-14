#!/bin/bash
fullPath_PDE_file=$1
prefix_filter=$2
 cd ./fiterWaves_jobs/ &&
./gen_filterWavesByStep_OnePMT.sh $fullPath_PDE_file $prefix_filter && cd subjobs && ./${name_file}_sub.sh
