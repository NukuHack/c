#include <stdio.h>
#include <stdbool.h>
#include "small_lib.h"
#include <unistd.h>

#define SIZE 64
static bool screen[SIZE][SIZE] = {0};

// small, basic abs
int abs(int a) {
    return a < 0 ? -a : a;
}
int test(int x, int y, int r) {
    return x*x+y*y-r*r;
}
bool magnitude(int x, int y, int r) {
    //
    //return abs(test(x-1,y,r)) < abs(test(x,y,r));
    // this would return :
    // | (x-1)^ + y^ - r^ | < | x^ + y^ - r^ |
    // -( (x-1)^ +y^ -r^ ) < x^ +y^ -r^
    // -( x^ -2x +1 +y^ -r^ ) < -||-
    // -x^ +2x -1 -y^ +r^ < -||-
    // -2x^ +2x -2y^ +2r^ -1 < 0
    return 2*x -2*(x*x) -2*(y*y) +2*(r*r) -1 < 0;
    // Digital Differential Analyzer - [C = 2r^ -1]
    // D(x,y) = -2x^ +2x -2y^ +C
    // y side :
    // D(x,y+1) = -2x^ +2x -2(y+1)^ +C
    //  -||- = -2x^ +2x -2y^ -4y -2 +C
    // D(x,y+1) - D(x,y) = -4y -2 = E(x,y)
    // E(x,y+1) = -4(y-1) -2 = -4y -4 -2
    // E(x,y+1) - E(x,y) = -4
    // y side : [-2x^ +2x -2y^ +C], step : -4y -2, = -4
    // x side :
    // D(x-1,y) = -2(x-1)^ +2(x-1) -2y^ +C
    //  -||- = -2x^ +4x -2 +2x -2 -2y^ +C
    //  D(x-1,y) - D(x,y) = +4x -4 = F(x,y)
    //  F(x-1,y) = 4(x-1) -4 = 4x-4-4
    //  F(x-1,y) - F(x,y) = -4
    //  x side : [-2x^ +2x -2y^ +C], step : 4x -4, = -4
}


void printCircle() {
    for (int i = 0; i<SIZE; i++) {
        for (int j = 0; j<SIZE; j++) {
            printf("%c",screen[i][j]?'X':' ');
        }
        printf("\n");
    }
}

bool is_circle_outline(int x, int y, int r) {
    return x * x + y * y == r * r;
    // make it to 0
    return x * x + y * y - r * r == 0;
    // non negative : pos -> out, neg -> in
}

void set_all(int x, int y, int cx, int cy) {
    screen[cy+y][cx+x] = 1;
    screen[cy+y][cx-x] = 1;
    screen[cy-y][cx+x] = 1;
    screen[cy-y][cx-x] = 1;

    screen[cx+x][cy+y] = 1;
    screen[cx+x][cy-y] = 1;
    screen[cx-x][cy+y] = 1;
    screen[cx-x][cy-y] = 1;
}

void calcAndPrintCircle(int r, int cx, int cy) {
    // Digital Differential Analyzer - [C = 2r^ -1]
    // y side : [-2x^ +2x -2y^ +C], step : -4y -2, = -4
    //  x side : [-2x^ +2x -2y^ +C], step : 4x -4, = -4
    int x = r;
    int y = 0;
    int r2 = r<<1;
    //int d = -2*x*x +x*x -2*y*y +2*r*r -1;
    //int d = 2*x - 2*x*x -2*y*y + 2*r*r -1; // since x = r, and y = 0
    int d = r2 - 1; // 2*x - 1
    int xe = (r2<<1) -2; // 4*x -2
    int ye = -4; // 4*y -4
    /* // old
       while (x > y) {
       if (magnitude(x, y, r)) {
       x--;
       }
       set_all(x, y);
       ++y;
       }
       */
    while (x >= y) {
        set_all(x, y, cx, cy);
        // y side
        d+=ye;
        ye-=4;
        y++;
        // x side
        if (d < 0) {
            d+=xe;
            xe-=4;
            x--;
        }


    }
    printCircle();
}

int circle_main(void) {

    int r = inp_int_s("input a circle radius: ", 3, 30);
    int y_off = inp_int_s("y offset: ", -10, 10) + SIZE / 2;
    int x_off = inp_int_s("x offset: ", -10, 10) + SIZE / 2;

    calcAndPrintCircle(r, x_off, y_off);

    return 0;
}

int bouncing(void) {
    for (int i = SIZE/2 -8; i<SIZE/2 +8; i++) {
        clear();
        calcAndPrintCircle(10, i, i);

        sleep(1);
    }

    return 0;
}

