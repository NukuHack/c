#!/usr/bin/env bash

Usage="Usage is :
    $0 <flag> <arg>
  flags :
     -main = run main.c with everything (similar flags like -mem)
     -mem = memory checking
     -un = undefined checks
     -b = basic, no extra
     -f = list how they are run
"

if [ "$#" -eq 0 ]; then
    echo "$Usage"
    exit 1
fi

base_flags="-W -Wall -Wextra -Wconversion -Warray-bounds -Wunused -Wwrite-strings \
-Wswitch -Wswitch-bool -Wmaybe-uninitialized -O -Waddress -Waggregate-return \
-Waggressive-loop-optimizations -pedantic -Wpedantic -std=c99 -g "

mem_flags=" -DMY_MEMCHECK -DMY_LOGGING=5 "
mem_files="logging.c memcheck.c "

leak=" -fsanitize=leak "
undef=" -fsanitize=undefined "

log_file="latest.log"
log_flag="-DLOG_FILE=\"$log_file\" "

run_files() {
    local bui="$1"
    local flags="$2"
    local files="$3"
    local inp="$4"
    #not works
    local rm_log=" && rm -f $log_file"
    local out=" && ./a.out $inp"

    if [[ "$bui" == "cbuild"  ]]; then
        out=" -- $inp"
    fi
    $bui macro.c my_utils.c my_io.c $files $flags -Dmacro_main='main' $out
    $bui my_utils.c $files $flags -Dmy_main='main' $out
    $bui my_io.c my_utils.c $files $flags -Darr_main='main' $out
    $bui logging.c my_utils.c $files $flags -Dlog_main='main' $out
    $bui logging.c my_utils.c $files $flags $log_flag -Dlog_main='main' $out $rm_log
    $bui my_io.c my_utils.c $files $flags -Dinput_main='main' $out
}

main_fn() {
    local arg="${@:2}"
    case "$1" in
        "-gcc") # gcc !
            flags=" $base_flags"
            files=""
            run_files "gcc" "$flags" "$files" "$arg"
            ;;
        "-b") # as basic
            flags="-Xcc $base_flags"
            files=""
            run_files "cbuild" "$flags" "$files" "$arg"
            ;;
        "-un")
            flags="-Xcc $base_flags $undef"
            files=""
            run_files "cbuild" "$flags" "$files" "$arg"
            ;;
        "-mem")
            flags="-Xcc $base_flags $leak $mem_flags"
            files="$mem_files "
            run_files "cbuild" "$flags" "$files" "$arg"
            ;;
        "-f")
            echo "gcc <input-file> -o <output-exe>"
            echo "cbuild <input-file> -o <output-exe> -c <combined-code>"
            echo "flags:
            base : $base_flags
            mem : $mem_flags -- and link with: $mem_files
            undefined test : $undef
            leak test : $leak"
            ;;
        "-main") # as main
            flags="-Xcc $base_flags $leak $mem_flags $log_flag "
            files="$mem_files "
            cbuild main.c my_io.c my_utils.c $files $flags -- $arg
            ;;
        *)
            echo "$Usage"
            exit 1
            ;;
    esac
    echo "Done - finished all"
}

main_fn "$@"


