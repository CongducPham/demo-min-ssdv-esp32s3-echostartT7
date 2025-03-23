#!/bin/bash
if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the jpeg original file (no .jpg extension), the SSDV quality and the SSDV packet size"
    echo "e.g. ssdv_tool_chain person240x240 4 80"
    exit
fi
    
python convert_to_ycbcr.py $1
./ssdv -e -c ECHOT7 -i 0 -q $2 -l $3 ${1}_yCbCr.jpg ${1}_yCbCr_${3}_q${2}.bin
./ssdv -d -l $3 ${1}_yCbCr_${3}_q${2}.bin output_${1}_${3}_q${2}.jpg
python convert_back_to_sRGB.py output_${1}_${3}_q${2}