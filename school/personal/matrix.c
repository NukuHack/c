#include <stdio.h>
#include <stdlib.h>

double det(const double *a, int n) {
    if (n == 1)
        return *a;

    int m = n - 1;
    double d = 0.0;
    double *minor = (double*)malloc(m * m * sizeof *minor);
    if (!minor)
        goto done;

    for (int col = 0; col < n; col++) {
        int idx = 0;
        /* Build minor: remove row 0 and column col */
        for (int i = 1; i<n; i++) {
            for (int j = 0; j<n; j++) {
                if (j == col) continue;
                minor[idx++] = a[i * n + j];
            }
        }

        double sign = col >> 1 ? -1.0 : 1.0;
        d += sign * a[col] * det(minor, m);
    }

done:
    free(minor);
    return d;
}

int main(void) {
    double arr[] = {
        2, 4, 5,
        6, 2, 1,
        2, 3, 2
    };

    double d = det(arr, 3);
    printf("d : %f\n", d);
    return 0;
}

