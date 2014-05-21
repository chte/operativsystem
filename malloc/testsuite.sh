#!/bin/bash
if [ $# -ne 1 ]; then
	echo "Usage: $0 <program>"
	exit;
fi

echo -n "IMPORTANT! Running this test deletes all previous testdata for $1. Proceed? [Y/n] " 
read run
if [[ $run == "n" || $run == "N" || $run == "" ]]; then exit; fi



#CLEAR PREVOUS make_out
rm -f make_out

#0=SYSTEM_MALLOC 1=FIRST_FIT 2=BEST_FIT
STRATEGY="0 1 2"
EPOCHS="100000 200000 300000 400000 500000"
RUNS='seq 5'
TESTFILE="$1"

for S in $STRATEGY; do
	if [ $S -eq 0 ]; then
		echo "RUNNING TESTS FOR SYSTEM_MALLOC"; 
	elif [ $S -eq 1 ]; then
		echo "RUNNING TESTS FOR FIRST_FIT_MALLOC"; 
	elif [ $S -eq 2 ]; then
		echo "RUNNING TESTS FOR BEST_FIT_MALLOC"; 
	fi

	if [ $S -eq 0 ]; then
		make clean all MALLOC= &>> make_out
	else 
		make clean all STRATEGY=$S &>> make_out
	fi

	if [ $? -ne 0 ]; then
		echo "Test failed, something went wrong during make."
	fi

	DAT_FILE="'basename ${TESTFILE}'.$S.dat"

	#CLEAR PREVIOUS RESULT FILES
	rm -f $DAT_FILE

	printf "# Iterations\tHeap memory\t Time (CPU seconds, user + kernel mode)\n" | tee -a $DAT_FILE

	for i in $EPOCHS; do
		#Run the test five times for each epoch
		for j in $RUNS; do
			# Measure time
			output=$(/usr/bin/time --format="Time: %U %S\n" $TESTFILE $i 2>$1)
			m[$j]=$(echo "$output" | grep -o "Memory usage; [0-9]* b" | awk '{print $3}') 
			t[$j]=$(echo "$output" | grep -o "Time; [0-9]*.[0-9]* [0-9]*.[0-9]*" | awk '{print $2 + $3}')
			echo "${t[$j]}" | tr " " "\n"
			#m_tot=t_tot+$(m[$j]) | tr " " "\n" 
			#printf "%d\t%11d\t%f\n" $j $m[$j] $t[$j] | tee -a $DAT_FILE
		done

		#t_avg=$t_tot;
		#m_avg=$m_tot;
		#printf "%d\t%11d\t%f\n" $i $m_avg $t_avg | tee -a $DAT_FILE
	done
done