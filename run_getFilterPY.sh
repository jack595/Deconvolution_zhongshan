#!/bin/bash
RawDataName=$(basename $1)
path_ProfileFile=`head -n 1 ./output_pdf/py_needed_info/${RawDataName}_PYInfo.txt`
echo $path_ProfileFile
source ~/.bashrc&& conda activate tf&& echo "setup tf successfully" && python getFilterUseNoise.py -i $path_ProfileFile