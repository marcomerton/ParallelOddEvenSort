if [[ $# < 5 ]]; then
	echo USE: $0 nruns output-file N niter chunksize
	exit 0
fi

RUNS=$1
OUT_FILE=$2
N=$3
NITER=$4
CHUNKSIZE=$5

make clean
make odd-even-ff

rm $OUT_FILE 2>/dev/null
touch $OUT_FILE

for (( nw = 1; nw < 17; nw++ )); do
	for (( i = 0; i < RUNS; i++ )); do
		./odd-even-ff $N $NITER 42 $nw $CHUNKSIZE >> $OUT_FILE
		sleep 1
	done
done