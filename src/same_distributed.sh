#!/bin/bash
# same_distributed.sh
# Made by Benjamin Negrevergne 
# Started on <2017-05-08 Mon>

# This script runs nrpa in a distributed fashion. Each job is an independant nrpa
# instance, results are gathered at the end of each run and the best score is outputed.
# (Note that each job may be using many core)
#
# Currently, this script runs nrpa for same game, but it should be
# easy to run it for anything else built on top of this nrpa library
# (just replace same by the correct executable name)
#
# Usage: see ./same_distributed (without any argument).
#


usage () {
cat << EOS
./same_distributed <node_list_file> <num_nrpa> <standard_nrpa_arguments>
 Where: 
    <node_list_file> is the list of nodes accessible by ssh using gnu
    parallel syntax (see man parallel)

    For example a file containing the two folling lines
        2/ node1
        2/ node2
    will create two jobs on host1 and two on node2.
    Each host must be fully accessible using ssh, it is recommanded to
    manually login on each node before running this script.
 
    <num_nrpa> is the number of nrpa instance to run, should be lower than the total number of cores available in node_list_file 
   
    <standard_nrpa_arguments> can be any argument supported by
    nrpa. Arguments are passed to nrpa untouched with the exception of
    --num-run (or -r) and --seed (or -a) and --statfilePrefix (or -f)  which are captured by this
    script to ensure that this script behaves like the normal nrpa algorithm.  
EOS
}

if [ $# -lt 2 ]; then
    usage;
    exit 1;
fi

if [ ! -f $1 ]; then
    echo "Error: first argument must be a file containing the list of nodes." 1>&2
    usage;
    exit 1;
fi
    
NRPA_HOME=$(pushd $(dirname $0) > /dev/null; pwd; popd > /dev/null)
echo "Assuming nrpa home at $NRPA_HOME"

NODE_FILE=$1
shift

NUM_NRPA=$1
shift

echo "ARGS $*"
# parse option and exit
$NRPA_HOME/same -o $* > /tmp/nrpa_distributed_options
NUM_RUN=$(cat /tmp/nrpa_distributed_options | grep 'numRun = ' | cut -d '=' -f 2)

echo "Number of runs $NUM_RUN"


#NUM_NRPA=$(cat $NODE_FILE | grep -c -v -e '^#.*$' -e '^$')
echo "Running $NUM_NRPA parallel NRPAs."

BEST=-9999999 # todo: berk

echo "NUM RUNS $NUM_RUN"


COUNT=0;
TOTAL=0; 

NRPA_OUTPUT=$(mktemp)
NRPA_STATS=$(mktemp)

echo "#<RunId> <timereventid> <timestamp> <currentbestscore>" > $NRPA_STATS

for i in $(seq 1 $NUM_RUN); do 

    seq 1 $NUM_NRPA | parallel  --no-notice --sshloginfile $NODE_FILE \
				 $NRPA_HOME/remote_run.sh $NRPA_HOME \
				 same $* -S -r 1 --seed=0 --statfile-prefix="$NRPA_OUTPUT_nrpa_stats_"{}"_"$i ::: > $NRPA_OUTPUT

    echo "All scores:"
    cat $NRPA_OUTPUT | grep Bestscore: | cut -d ' ' -f 2
    cat $NRPA_OUTPUT | grep timer_stats

    LOCALBEST=$(cat $NRPA_OUTPUT | grep Bestscore: | cut -d ' ' -f 2 | sort -n | tail -n 1)
    echo "Best score in this run: $LOCALBEST"

    ((COUNT++))
    TOTAL=$(echo $TOTAL+$LOCALBEST | bc )

    if [ "$LOCALBEST" -gt "$BEST" ]; then
	BEST=$LOCALBEST;
    fi

# generate stats (OMG!)
    cat $NRPA_OUTPUT | awk 'BEGIN{ for(i = 0 ; i < 20; i++){timestamp[i] = -1; best[i] = -99999}}/timer_stats:  [^#]/{ if ($5 > best[$3]) { timestamp[$3] = $4; best[$3] = $5 } }END{ for (i in timestamp){ if(timestamp[i] != -1) print '$i', i, timestamp[i], best[i]} print "\n\n"}' >> $NRPA_STATS

done

cat $NRPA_OUTPUT

#rm $NRPA_OUTPUT
#rm $NRPA_STATS
#rm $NRPA_OUTPUT_nrpa_stats_*

AVGBEST=$(echo "scale=2; $TOTAL / $COUNT" | bc)
echo "Gobal bestscore: $BEST"
echo "Average bestscore: $AVGBEST"


