#!/bin/bash

# This will take the mcc data from the file "data/MCC" in the repository
# tzdata-timed ( https://github.com/sailfishos/tzdata-timed ) and use it
# to update the code in this repository.

declare -i mcc
HEADER="../common/mccmapping.h"
SCRIPT="./mccmapping"
mcc_file=$1
if [ -z "$mcc_file" ] ; then
    echo "Usage: $0 [MCC file]" >&2
    exit 1
fi
if [ ! -e "$HEADER" ] ; then
    echo "ERROR: Unable to locate header $HEADER" >&2
    exit 1
fi
if [ ! -e "$SCRIPT" ] ; then
    echo "ERROR: Unable to locate script file $SCRIPT" >&2
    exit 1
fi

mccs=`grep -v ^# "$mcc_file" | cut -d ' ' -f1 | sort -nu | tr '\n' ',' | sed 's/^,//;s/,$/,1024/'`
count=`echo -n $mccs | tr -dc ',' | wc -c`

if [ $count -gt 255 ] ; then
    echo -e "We have a problem. The functionality of mlsdbtools assumes there are fewer than\n256 unique mccs. The file provided gives us $count. Some manual intervention is needed." >&2
    exit 1
fi

echo "Updating $HEADER"
sed -i "s/=.*/= \{$mccs\}\;/;s/... unique/$count unique/" $HEADER

echo "Updating $SCRIPT"
last_mcc=0
first=0
echo "# MCC -> country code mapping from tzdata-timed repository data/MCC" > $SCRIPT
IFS=$'\n' # Set for each separator to newline only
for element in `grep -v ^# $mcc_file | cut -d ' ' -f1,3 | sort -u` ; do
    mcc=`echo "$element" | cut -d ' ' -f1`
    country=`echo "$element" | cut -d ' ' -f2`
    if [ $mcc -eq $last_mcc ] ; then
        echo -n " $country" >> $SCRIPT
        continue
    fi
    if [ $first -ne 0 ] ; then
        echo '"' >> $SCRIPT
    fi
    first=1
    echo -n "MCC[$mcc]=\"$country" >> $SCRIPT
    last_mcc=$mcc
done
echo '"' >> $SCRIPT
echo "Done"
