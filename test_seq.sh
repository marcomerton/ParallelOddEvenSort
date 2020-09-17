if [[ $# < 4 ]]; then
	echo USE: $0 nruns output-file N niter
	exit 0
fi

RUNS=$1
OUT_FILE=$2
N=$3
NITER=$4

make clean
make odd-even-seq

rm $OUT_FILE 2>/dev/null
touch $OUT_FILE

for (( i = 0; i < RUNS; i++ )); do
	./odd-even-seq $N $NITER 42 >> $OUT_FILE
done