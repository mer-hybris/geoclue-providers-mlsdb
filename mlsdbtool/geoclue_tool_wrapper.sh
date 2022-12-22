#!/bin/bash
set -e
rundir=`dirname $0`
source $rundir/mccmapping
TOOL=$rundir/geoclue-mlsdb-tool

infile=$1
country=`echo $2 | tr [:lower:] [:upper:]`
if [ -z "$country" ] ; then
    echo "Usage: $0 [mls_data_file] [country_code]" >&2
    exit 1
fi

if [ ! -e $infile ] ; then
    echo "ERROR: Can't find infile $infile" >&2
    exit 1
fi

if [ ! -x "$TOOL" ] ; then
    echo "ERROR: Can't find executable $TOOL" >&2
    exit 1
fi

mccs=""
grepopts="grep"
for i in {202..750} ; do
    for entry in ${MCC[$i]} ; do
        if [[ "$entry" == "$country" ]] ; then
            mccs="$mccs $i"
            # Grep for field that ends with M, E or S (i.e. GSM, LTE & UMTS). The mcc field is the next one.
            grepopts="$grepopts -e '[M,E,S],$i,'"
        fi
    done
done

if [ -z "$mccs" ] ; then
    echo "ERROR: Can't find any country with code $country" >&2
    exit 1
fi

echo "Will encode the mlsdb data for country: $country encompassing the mccs: $mccs"
tail -n +2 $infile | eval "$grepopts" | sort -t, -k2n,2 -k3n,3 -k4n,4 -k5n,5 -k1,1 | $TOOL

for mcc in $mccs ; do
    cat $mcc.ntw $mcc.loc > $mcc.dat
    rm $mcc.ntw
    rm $mcc.loc
done
