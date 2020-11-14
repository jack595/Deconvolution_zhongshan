#!/bin/bash
source ~/.bashrc
RawDataName=$(basename $1)
path_ProfileFile=`head -n 1 ./output_pdf/py_needed_info/${RawDataName}_PYInfo.txt`
echo $path_ProfileFile
source /afs/ihep.ac.cn/users/l/luoxj/junofs_500G/anaconda3/etc/profile.d/conda.sh && conda activate tf&& echo "setup tf successfully" && which python && ~/junofs_500G/anaconda3/envs/tf/bin/python getFilterUseNoise.py -i $path_ProfileFile

