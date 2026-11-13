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

flags1="-W -Wall -Wextra -pedantic -Wpedantic -std=c99 -g -fsanitize=leak " # this is for mem leaks
flags2="-W -Wall -Wextra -pedantic -Wpedantic -std=c99 -g -fsanitize=undefined " # this is for heap corruption
flags3="gcc my_utils.c logging.c input.c memcheck.h -o exe -Dinput_main='main' -DDYN_STR -DMY_MEMCHECK -W -Wall -Wextra -pedantic -Wpedantic -std=c99 -g -fsanitize=undefined"
inp="$1"
log_file="latest.log"

main_fn() {
    case "$1" in
        "-un")
            flag="-Xcc $flags2"
            cbuild my_utils.c $flag -Dmy_main='main'
            cbuild arr_op.c my_utils.c -m arr_main $flag
            # cbuild list_main.c list.c my_utils.c $flag
            cbuild logging.c my_utils.c -m log_main $flag
            cbuild logging.c my_utils.c -m log_main $flag -DLOG_FILE=\"$log_file\" && rm -f $log_file
            cbuild input.c my_utils.c -m input_main $flag
            cbuild input.c my_utils.c -m input_main $flag -DDYN_STR
            ;;
        "-mem")
            flag="-Xcc $flags1"
            cbuild my_utils.c $flag -Dmy_main='main'
            cbuild arr_op.c my_utils.c -m arr_main $flag
            # cbuild list_main.c list.c my_utils.c $flag
            cbuild logging.c my_utils.c -m log_main $flag
            cbuild logging.c my_utils.c -m log_main $flag -DLOG_FILE=\"$log_file\" && rm -f $log_file
            cbuild input.c my_utils.c -m input_main $flag
            cbuild input.c my_utils.c -m input_main $flag -DDYN_STR
            ;;
        "-f")
            echo "mem leak stuff : "
            echo "gcc <input-file> -o <output-file> $flags1"
            echo "undefined + heap check: "
            echo "gcc <input-file> -o <output-file> $flags2"
            ;;
        *)
            echo "$Usage"
            exit 1
            ;;
    esac
    echo "Done - finished all"
}

main_fn "$inp"


