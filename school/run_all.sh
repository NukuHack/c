#!/usr/bin/env bash

Usage="Usage is :
    $0 <flag>
  flags :
     -mem = memory checking
     -un = undefined & heap checks
     -b = basic, no extra
     -f = list how they are run
"

if [ "$#" -eq 0 ]; then
    echo "$Usage"
    exit 1
fi

base_flags="-W -Wall -Wextra -Wconversion -Warray-bounds -Wunused -Wwrite-strings \
-Wswitch -Wswitch-bool -Wmaybe-uninitialized -Waddress -Waggregate-return \
-Waggressive-loop-optimizations -pedantic -Wpedantic -std=c99 -g "

mem_flags=" -DMY_MEMCHECK -DMY_LOGGING=5 "
mem_files="logging.c memcheck.c "

leaky=" -fsanitize=leak " # this is for mem leaks
undef=" -fsanitize=undefined " # this is for heap corruption

inp="$1"
log_file="latest.log"

run_files() {
    flag="$1"
    files="$2"

    cbuild macro.c my_io.c my_utils.c $files $flags -- 2
    cbuild my_utils.c $files $flag -Dmy_main='main'
    cbuild arr_op.c my_utils.c $files -m arr_main $flag
    # cbuild list_main.c list.c my_utils.c $files $flag
    cbuild logging.c my_utils.c $files -m log_main $flag
    cbuild logging.c my_utils.c $files -m log_main $flag -DLOG_FILE=\"$log_file\" && rm -f $log_file
    cbuild my_io.c my_utils.c $files -m input_main $flag
}

main_fn() {
    case "$1" in
        "-b") # as basic
            flag="-Xcc $base_flags"
            files=""
            run_files "$flag" "$files"
            ;;
        "-un")
            flag="-Xcc $base_flags $undef"
            files=""
            run_files "$flag" "$files"
            ;;
        "-mem")
            flag="-Xcc $base_flags $leaky $mem_flags"
            files="$mem_files "
            run_files "$flag" "$files"
            ;;
        "-f")
            echo "gcc <input-file> -o <output-exe>"
            echo "cbuild <input-file> -o <output-exe> -c <combined-code>"
            echo "flags:
            base : $base_flags
            leak test : $leaky
            manual : $manual_flags
            undefined test : $undef"
            ;;
        *)
            echo "$Usage"
            exit 1
            ;;
    esac
    echo "Done - finished all"
}

main_fn "$inp"


