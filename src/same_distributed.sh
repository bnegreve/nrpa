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
#!/bin/bash

function usage {
cat << EOS
./same_distributed <node_list_file> <standard_nrpa_arguments>
 Where: 
    <node_list_file> is the list of nodes accessible by ssh using gnu
    parallel syntax (see man parallel)

    For example a file containing the two folling lines
        2/ node1
        2/ node2
    will create two jobs on host1 and two on node2.
    Each host must be fully accessible using ssh, it is recommanded to
    manually login on each node before running this script.
    
    <standard_nrpa_arguments> can be any argument supported by
    nrpa. Arguments are passed to nrpa untouched with the exception of
    --num-run (or -r) and --seed (or -a) which are captured by this
    script to ensure that this script behaves like the normal nrpa algorithm.  
EOS
}

if [ $# -lt 1 ]; then
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


echo "ARGS $*"
# parse option and exit
./same -o $* > /tmp/nrpa_distributed_options
NUM_RUN=$(cat /tmp/nrpa_distributed_options | grep 'numRun = ' | cut -d '=' -f 2)

echo "Number of runs $NUM_RUN"


NUM_NODES=$(cat $NODE_FILE | grep -c -v -e '^#.*$' -e '^$')
echo "Running nrpa on $NUM_NODES nodes."

BEST=-9999999 # todo: berk
for i in {1..$NUM_RUN}; do 

    seq 1 $NUM_NODES | parallel  --no-notice --sshloginfile $NODE_FILE \
				 $NRPA_HOME/remote_run.sh $NRPA_HOME \
				 same $* -r 1 --seed={} ::: > /tmp/nrpa_output

    echo "All scores:"
    cat /tmp/nrpa_output | grep Bestscore: | cut -d ' ' -f 2

    LOCALBEST=$(cat /tmp/nrpa_output | grep Bestscore: | cut -d ' ' -f 2 | sort -n | tail -n 1)
    echo "Best score in this run: $LOCALBEST"

    if [ $LOCALBEST -gt $BEST ]; then
	BEST=$LOCALBEST;
    fi
done

echo "Bestscore: $BEST"

