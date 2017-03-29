#!/bin/bash

# test_driver.sh
# Made by Benjamin Negrevergne 
# Started on <2016-01-24 Sun>

regexp=""
sort=""
keep_output=""
create_test=""
quiet=""
runtime=""

usage="Usage: $0 [-srkctq] <test_file>\n
Execute the command line on the first line and compare output with the rest of the file.\n
Return with success if they are both identical.\n\n

Options: \n
 -s: sort lines before comparing (with sort)\n
 -r <regexp>: filter out lines not matching regexp (with grep)\n
 -k: keep temporary files in case of failure\n
 -c: create test file instead. See 'Adding a test' section\n
 -t: measure test execution time as well\n
 -q: do not output program output (not even stderr). \n\n

Adding a test:\n
  Usage: $0 -c [-sr] <test_file> <command> \n 
  Create a new test file with <command> and store it in test_file.\n
  Other options are still valid and have the expected behaviour.\n
"

# parse options, shift until $1 doesn't start with a dash
while [[ "$1" == \-* ]]; do
    case $1 in
        -s | --sort )           sort=1
                                ;;
        -k | --keep-output )    keep_output=1
                                ;;
	-r | --grep )           shift
				regexp=$1
				;;
	-t | --check-runtime )  runtime=1
	                        ;;
	-c | --create-test )    create_test=1
	                        ;;
	-q | --quiet )          quiet=1
	                        ;;
        * )           echo "Error: unexpected $1."
		      echo -e $usage
                      exit 1
                      ;;
    esac
    shift
done

# Deal with test creation 
if [ ! -z $create_test ]; then
    if [ ! $# -gt 2 ]; then
    echo "Error: unexpected number of arguments. (Expected at least 3, got $#.)" 
    echo -e $usage;
    exit 1;
    fi

    test_file=$1
    shift 
    command=$*
    
    if [ ! -z runtime ]; then
	# The following commands run the command in $command,
	# redirects, program stdout to ref_out, program stderr to
	# stdout and time stderr to tmp
	TIMEFORMAT="%E"
	{ time { $command 2>&1 1>/tmp/ref_out;  }; } 2> /tmp/ref_out.time
    else
	$command > /tmp/ref_out
    fi
    

    echo "$0: Note, creating test file '$test_file' with command '$command'." 

    if [ $? -ne 0 ] ; then
	echo "$0: Error, '$command' returned with non-zero value (use -k switch to keep test output files)."
	if [ -z $keep_output ]; then
	    rm -f /tmp/ref_out 
	else
	    echo "$0: Note, output file /tmp/ref_out was kept." >&2
	fi
	exit 1
    fi

    if [ ! -z $regexp ]; then
	grep -e $regexp  /tmp/ref_out > /tmp/ref_out_filtered 
	mv /tmp/ref_out_filtered /tmp/ref_out
	cmd_string_regexp=" -r $regexp"
    fi

    if [ ! -z $sort ]; then
	sort /tmp/ref_out > /tmp/ref_out_sorted
	mv /tmp/ref_out_sorted /tmp/ref_out
	cmd_string_sort=" -s"
    fi

    if [ $(wc -l /tmp/ref_out | cut -d ' ' -f 1) -eq 0 ] ; then
	echo "$0: Warning, test output is empty." >&2
    fi

    echo $command > $test_file
    cat /tmp/ref_out >> $test_file
    mv /tmp/ref_out.time $test_file.time

    rm -f /tmp/ref_out 
    
    echo -e "$0: Note, test '$test_file' successfully created, you can evaluate it by running the following command."
    echo    "########## NEW TEST ##########"
    echo    "./test_driver.sh $cmd_string_sort $cmd_string_regexp $test_file"
    echo    "##############################" 

    exit 0

fi


   
# Deal with evaluation of an existing test
if [ $# -ne 1 ]; then
    echo -e $usage;
    exit 1;
fi 


test_file=$1

# extract first line
command=$(head -q -n 1 $test_file)
if [ $? -ne 0 ] ; then
    echo "$0: Error, could not extract first line of '$test_file'"
    exit 1
else
    echo "Test command is: $command"
fi

# extract rest 
tail -n +2 $test_file > /tmp/ref_out
if [ $? -ne 0 ] ; then
    echo "$0: Error, could not extract expected output from '$test_file'"
    exit 1
fi

if [ -f "$test_file.time" ]; then
    runtime=1;
else
    if [ -z $runtime ]; then
	echo "$0: Warning, no timing data for '$test_file' (i.e. there should be a '$test_file.time' in the same directory.)"
    fi
fi

	 


if [ ! -z runtime ]; then
    # The following commands run the command in $command,
    # redirects, program stdout to ref_out, program stderr to
    # stdout and time stderr to tmp
    TIMEFORMAT="%E"
    if [ ! -z $quiet ]; then
	{ time { $command 2>/dev/null 1>/tmp/test_out;  }; } 2> /tmp/test_out.time
    else
	{ time { $command 2>&1 1>/tmp/test_out;  }; } 2> /tmp/test_out.time
    fi
else
    if [ ! -z $quiet ]; then
	$command > /tmp/test_out 2> /dev/null
    else
	$command > /tmp/test_out 
    fi
fi


if [ $? -ne 0 ] ; then
    echo "$0: Error while executing '$command'"
    exit 1
fi

if [ ! -z $regexp ]; then
    grep -e $regexp  /tmp/ref_out > /tmp/ref_out_filtered 
    mv /tmp/ref_out_filtered /tmp/ref_out

    grep -e $regexp /tmp/test_out > /tmp/test_out_filtered
    mv /tmp/test_out_filtered /tmp/test_out
fi

if [ ! -z $sort ]; then
    sort /tmp/ref_out > /tmp/ref_out_sorted
    mv /tmp/ref_out_sorted /tmp/ref_out

    sort /tmp/test_out > /tmp/test_out_sorted
    mv /tmp/test_out_sorted /tmp/test_out
fi

if [ $(wc -l /tmp/test_out | cut -d ' ' -f 1) -eq 0 ] ; then
    echo "$0: Warning, test output is empty." >&2
fi

if [ $(wc -l /tmp/ref_out | cut -d ' ' -f 1) -eq 0 ] ; then
    echo "$0: Warning, ref output is empty." >&2
fi

diff -Zy --suppress-common-lines  /tmp/ref_out /tmp/test_out

test_res=$?

if [ $test_res -eq 0 ]; then
    echo "$0: SUCCESS." >&2
    if [ ! -z $runtime ]; then
	echo -n "Original run time : "
	cat $test_file.time
	echo -n "This run time : "
	cat /tmp/test_out.time
    fi
else
    echo "$0: FAILURE. (Use -k switch to keep test output files)" >&2
fi


if [ -z $keep_output ]; then
    rm -f /tmp/ref_out /tmp/test_out
else
    echo "$0: Note, output file /tmp/ref_out and /tmp/test_out were kept." >&2
fi

exit $test_res
