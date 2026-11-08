
if [ "$#" -eq 0 ]; then
    echo "Usage $0 <flag>"
    exit 1
fi

inp="$1"

case "$inp" in
    "-d")
        cbuild arr_op.c small_lib.c -m arr_main -Xcc -Wall -Wextra -Wpedantic -std=c99

        cbuild list_main.c list.c small_lib.c -Xcc -Wall -Wextra -Wpedantic -std=c99

        cbuild input.c string.c small_lib.c -m input_main -Xcc -Wall -Wextra -Wpedantic -std=c99

        cbuild input.c string.c small_lib.c -m string_main -Xcc -Wall -Wextra -Wpedantic -std=c99

        cbuild input.c string.c small_lib.c -m input_main -Xcc -DDYN_STR -Wall -Wextra -Wpedantic -std=c99
        ;;
    *)
        echo "bruh use -d fr debug"
        ;;
esac
