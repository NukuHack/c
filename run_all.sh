if [ "$#" -eq 0 ]; then
    echo "Usage $0 <flag>"
    exit 1
fi
flags="-Xcc -W -Wall -Wextra -pedantic -Wpedantic -std=c99"
inp="$1"

case "$inp" in
    "-d")
        cbuild arr_op.c my_utils.c -m arr_main $flags
        # cbuild list_main.c list.c my_utils.c $flags
        cbuild logging.c my_utils.c -m log_main $flags
        cbuild logging.c my_utils.c -m log_main $flags -DLOG_FILE=\"latest.log\"
        cbuild input.c string.c my_utils.c -m input_main $flags
        cbuild input.c string.c my_utils.c -m string_main $flags
        cbuild input.c string.c my_utils.c -m input_main -Xcc -DDYN_STR $flags
        ;;
    *)
        echo "bruh use -d fr debug"
        ;;
esac
