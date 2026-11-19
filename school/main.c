#include "my_utils.h"
#include "my_io.h"
#include "macro.h"


int main(void) {
    int n, k;
    {
        char tmp[100];
        err("input 2 num: ");
        if (!read_str(NULL, ARR_L(tmp))) {
            err("input failed");
            return 1;
        }
        int p = parse_i(tmp, &n);
        p = parse_i(tmp+p+1, &k);
    }

    size_t n_ = (size_t)n;
    int *arr = New(int, n_);
    if(!arr) {
        err("malloc fail");
        return 1;
    }

    arr_foreach_p(int, var, arr, n) {
        *var = inpi_s(NULL, 0, 200);
    }

    int i = 0, ind = -1, count = 0;
    arr_foreach(int, var, arr, n) {
        i++;
        if (count == k)
            break;
        if (var >= 10 && var <= 100) {
            if (ind == -1) ind = i;
            count++;
        } else {
            ind = -1;
            count = 0;
        }
    }
    printf("%d\n", ind);
}
