#!/bin/bash

PID=$$

BXB_EXE="bxb -t 0 -w 0.15"
ANALYZE_FROM=0
#ANALYZE_GAIN=50
ANALYZE_GAIN=200

FILEARGS="[123]*.dat"
DBFOLDER=~/proj/physionet.org/physiobank/database/mitdb

if [ "$1" != "" ]; then
	DBFOLDER=$1
fi

if [ "$2" != "" ]; then
	ANALYZE_GAIN=$2
fi

if [[ $DBFOLDER == *mitdb* ]]; then
	ANALYZE_FROM="0"
	FILEARGS="[123]*.dat"
fi

if [[ $DBFOLDER == *ahadb* ]]; then
	ANALYZE_FROM=300
	FILEARGS="?2??.dat"
fi

if [[ $DBFOLDER == *edb* ]]; then
	ANALYZE_FROM=300
	FILEARGS="e*.dat"
fi

if [[ $DBFOLDER == *cudb* ]]; then
	ANALYZE_FROM="0:15"
fi

if [[ $DBFOLDER == *nstdb* ]]; then
	ANALYZE_FROM="0:15"
	FILEARGS="1*.dat"
fi

if [[ $DBFOLDER == *qtdb* ]]; then
	ANALYZE_FROM=0
	FILEARGS="*.dat"
fi

MAKEPWD=$(pwd)
RESULTSDIR=$MAKEPWD/results

mkdir -p $RESULTSDIR

pushd $DBFOLDER >& /dev/null

for i in $FILEARGS; do

	rec=$(echo $i | cut -d . -f 1)
	echo -n "$rec "

	$MAKEPWD/out/photon $rec  $ANALYZE_GAIN

#	echo $BXB_EXE -r $rec -a atr dbr -l $RESULTSDIR/bxb-$PID-results.txt /dev/null -f $ANALYZE_FROM 
	$BXB_EXE -r $rec -a atr dbr -l $RESULTSDIR/bxb-$PID-results.txt /dev/null -f $ANALYZE_FROM >& /dev/null

done

popd >& /dev/null

echo 

# echo sumstats $RESULTSDIR/bxb-$PID-results.txt
sumstats $RESULTSDIR/bxb-$PID-results.txt | grep Gross | awk ' { printf "%.2f %.2f    avg = %.2f\n", $2, $3, ($2 + $3) / 2.0 } '

