#!/usr/bin/env bash

Usage="Usage is :
    $0 <flag>
  flags :
     -mem = memory checking
     -un = undefined & heap checks
     -f = list how they are run
"

if [ "$#" -eq 0 ]; then
    echo "$Usage"
    exit 1
fi

base_flags="-W -Wall -Wextra \
    -Wconversion -Warray-bounds -Wunused -Wwrite-strings -Wswitch -Wswitch-bool \
    -Wmaybe-uninitialized -Waddress -Waggregate-return -Waggressive-loop-optimizations \
    -pedantic -Wpedantic -std=c99 -g "
bad_flags=" -Wtraditional"
leaky="-fsanitize=leak " # this is for mem leaks
undef="-fsanitize=undefined " # this is for heap corruption
flags3="gcc my_utils.c logging.c input.c memcheck.h -o exe -Dinput_main='main' -DDYN_STR -DMY_MEMCHECK $flags2"
inp="$1"
log_file="latest.log"

main_fn() {
    case "$1" in
        "-un")
            flag="-Xcc $base_flags $undef"
            cbuild my_utils.c $flag -Dmy_main='main'
            cbuild arr_op.c my_utils.c -m arr_main $flag
            # cbuild list_main.c list.c my_utils.c $flag
            cbuild logging.c my_utils.c -m log_main $flag
            cbuild logging.c my_utils.c -m log_main $flag -DLOG_FILE=\"$log_file\" && rm -f $log_file
            cbuild input.c my_utils.c -m input_main $flag
            cbuild input.c my_utils.c -m input_main $flag -DDYN_STR
            ;;
        "-mem")
            flag="-Xcc $base_flags $leaky"
            cbuild my_utils.c $flag -Dmy_main='main'
            cbuild arr_op.c my_utils.c -m arr_main $flag
            # cbuild list_main.c list.c my_utils.c $flag
            cbuild logging.c my_utils.c -m log_main $flag
            cbuild logging.c my_utils.c -m log_main $flag -DLOG_FILE=\"$log_file\" && rm -f $log_file
            cbuild input.c my_utils.c -m input_main $flag
            cbuild input.c my_utils.c -m input_main $flag -DDYN_STR
            ;;
        "-f")
            echo "gcc <input-file> -o <output-file> $base_flags $leaky"
            ;;
        *)
            echo "$Usage"
            exit 1
            ;;
    esac
    echo "Done - finished all"
}

main_fn "$inp"


