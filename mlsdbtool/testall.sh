#!/bin/bash

declare -i mcc
declare -i net
declare -i area
declare -i cell
count=0
infile=$1
mcc=$2

if [ -z "$2" ] ; then
    echo "Usage: $0 [CSV file] [mcc]" >&2
    exit 1
fi
num_lines=`grep "[M,E,S],$mcc," $infile | wc -l`

grep "[M,E,S],$mcc," $infile | while read LINE ; do
    ((count++))
    if [ $count -eq 1 ] ; then
        continue
    fi
    radio=`echo $LINE | cut -d ',' -f1`
    net=`echo $LINE | cut -d ',' -f3`
    area=`echo $LINE | cut -d ',' -f4`
    cell=`echo $LINE | cut -d ',' -f5`
    lon=`echo $LINE | cut -d ',' -f7`
    lat=`echo $LINE | cut -d ',' -f8`
    result=`./reader $mcc $net $area $cell $radio`
    res_lon=`echo $result | cut -d ' ' -f2`
    res_lat=`echo $result | cut -d ' ' -f4`
    diff_lon=`echo "$res_lon - $lon" | bc | tr -d '-'`
    diff_lat=`echo "$res_lat - $lat" | bc | tr -d '-'`
    if [[ $diff_lon != "0" && ${diff_lon:0:5} != ".0000" ]] ; then
        echo
        echo -e "Result mismatch on line $count\nStarted with $LINE\ngot $result" >&2
        echo $diff_lon
        exit 1
    fi
    if [[ $diff_lat != "0" && ${diff_lat:0:5} != ".0000" ]] ; then
        echo
        echo -e "Result mismatch on line $count\nStarted with $LINE\ngot $result" >&2
        echo $diff_lat
        exit 1
    fi
    echo -ne "$count / $num_lines\r"
done
