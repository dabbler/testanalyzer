#!/bin/bash

PID=$$

BXB_EXE="bxb -t 0 -w 0.15"
#ANALYZE_FROM=15
ANALYZE_FROM=300

DBFOLDER=~/proj/physionet.org/physiobank/database/mitdb
#DBFOLDER=~/proj/physionet.org/physiobank/database/edb

if [ "$1" != "" ]; then
	DBFOLDER=$1
fi

if [[ $DBFOLDER == *ahadb* ]]; then
	ANALYZE_FROM=300
fi

RESULTSDIR=$(pwd)/results

mkdir -p $RESULTSDIR

pushd $DBFOLDER >& /dev/null

for g in $(seq 25 25 250); do

	echo -n "$g : "

	for i in [ce123]*.dat; do

		rec=$(echo $i | cut -d . -f 1)
#		echo -n "$rec "

		~/proj/photon/out/photon $rec $g

		$BXB_EXE -r $rec -a atr dbr -l $RESULTSDIR/bxb-$PID-$g-results.txt /dev/null -f $ANALYZE_FROM >& /dev/null

	done

#	echo sumstats $RESULTSDIR/bxb-$PID-$g-results.txt
	sumstats $RESULTSDIR/bxb-$PID-$g-results.txt | grep Gross | awk ' { printf "%.2f %.2f    %.2f\n", $2, $3, ($2 + $3) / 2.0 } '

done

popd >& /dev/null

echo 

