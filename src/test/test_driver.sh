# test_driver.sh
# Made by Benjamin Negrevergne 
# Started on <2016-01-24 Sun>

#!/bin/bash

regexp=""
sort=""
keep_output=""
create_test=""

usage="Usage: $0 [-srkc] <test_file>\n
Execute the command line on the first line and compare output with the rest of the file.\n
Return with success if they are both identical.\n\n

Options: \n
 -s: sort lines before comparing (with sort)\n
 -r <regexp>: filter out lines not matching regexp (with grep)\n
 -k: keep temporary files in case of failure\n
 -c: create test file instead. See 'Adding a test' section. \n\n

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
	-c | --create-test )    create_test=1
	                        ;;
        * )           echo -e $usage
                      exit 1
                      ;;
    esac
    shift
done

# Deal with test creation 
if [ ! -z $create_test ]; then
    if [ ! $# -gt 2 ]; then
    echo -e $usage;
    exit 1;
    fi

    test_file=$1
    shift 
    command=$*
    $command > /tmp/ref_out

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
    echo "$0: Error, Could not extract first line of '$test_file'"
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

$command > /tmp/test_out


if [ $? -ne 0 ] ; then
    echo "$0: Error, Could not execute '$command'"
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
else
    echo "$0: FAILURE. (Use -k switch to keep test output files)" >&2
fi


if [ -z $keep_output ]; then
    rm -f /tmp/ref_out /tmp/test_out
else
    echo "$0: Note, output file /tmp/ref_out and /tmp/test_out were kept." >&2
fi

exit $test_res
