#!/usr/bin/env bash

Usage="Usage is :
    $0 <flag> <arg>
  flags :
     -main = run main.c with everything (-mem by default)
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

log_file="latest.log"
log_flag="-DLOG_FILE=\"$log_file\" "

inp="$1"
arg="$2"

run_files() {
    local flags="$1"
    local files="$2"
    local inp="$3"

    cbuild macro.c my_io.c my_utils.c $files $flags -- $inp
    cbuild my_utils.c $files $flags -Dmy_main='main' -- $inp
    cbuild arr_op.c my_utils.c $files -m arr_main $flags
    # cbuild list_main.c list.c my_utils.c $files $flags
    cbuild logging.c my_utils.c $files -m log_main $flags
    cbuild logging.c my_utils.c $files -m log_main $flags $log_flag && rm -f $log_file
    cbuild my_io.c my_utils.c $files -m input_main $flags
}

main_fn() {
    local arg="$2"
    local cas="$1"
    case "$cas" in
        "-b") # as basic
            flags="-Xcc $base_flags"
            files=""
            run_files "$flags" "$files" "$arg"
            ;;
        "-un")
            flags="-Xcc $base_flags $undef"
            files=""
            run_files "$flags" "$files" "$arg"
            ;;
        "-mem")
            flags="-Xcc $base_flags $leaky $mem_flags"
            files="$mem_files "
            run_files "$flags" "$files" "$arg"
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
        "-main") # as main
            flags="-Xcc $base_flags $leaky $mem_flags $log_flag "
            files="$mem_files "
            cbuild main.c my_io.c my_utils.c arr_op.c $files $flags -- $arg
            ;;
        *)
            echo "$Usage"
            exit 1
            ;;
    esac
    echo "Done - finished all"
}

main_fn "$inp" "$arg"


