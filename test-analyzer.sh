#!/bin/bash

PID=$$

DBFOLDER=~/proj/physionet.org/physiobank/database/mitdb

if [ "$1" != "" ]; then
	DBFOLDER=$1
fi

RESULTSDIR=$(pwd)/results

mkdir -p $RESULTSDIR

pushd $DBFOLDER >& /dev/null

for i in [12]*.dat; do

	rec=$(echo $i | cut -d . -f 1)
	echo -n "$rec "

	~/proj/photon/trunk/out/photon $rec

	bxb -r $rec -a atr dbr -l $RESULTSDIR/bxb-$PID-results.txt - >& /dev/null

done

popd >& /dev/null

echo 

# echo sumstats $RESULTSDIR/bxb-$PID-results.txt
sumstats $RESULTSDIR/bxb-$PID-results.txt | grep Gross

