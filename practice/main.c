#include "terminal_draw.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

int w =0, h =0;
typedef struct {
    Color c;
    char k;
    int x;
    int y;
    int dx;
    int dy;
} Kar;
Kar *table = NULL;

int pos_x(int mid, int count, int spread) {
    if (count < 2*spread)
        return mid - count / spread;
    else if (count > 3*spread)
        return mid + count / spread - 2;
    else
        return mid;
}
int pos_y(int base, int count, int spread) {
    if (base == 0)
        return base - count % spread;
    else
        return base + count % spread;
}
#define COUNT 100 // for good measure
void init_kar(int *count, Kar *arr) {
    int mid = h / 2; // around where on each side
    int spread = COUNT / 2 / 5; // how much on eahc side, in each row (5 row)
    for (int c=0;c < COUNT;c++) {
        Kar k;
        if (c > COUNT/2)
            k = (Kar) { .c = 31, .k = 'O',
                .x = pos_x(0, c, spread),
                .y = pos_y(mid, c, spread),
                .dx = 1, .dy = 0 };
        else
            k = (Kar) { .c = 34, .k = 'X',
                .x = pos_x(w, c, spread),
                .y = pos_y(mid, c, spread),
                .dx = -1, .dy = 0 };
        arr[c] = k;
    }
    count[0]=COUNT;
}

bool rem_if(Kar k) {
    return (
            (k.x < 0 && k.dx < 0) || 
            (k.x > w && k.dx > 0) || 
            (k.y < 0 && k.dy < 0) ||
            (k.y > h && k.dy > 0)
       );
}

int main(int argc, char** args) {
    goto ma;
fail:
    printf("fail, quitting");
    free(table); // fine, since freeing NULL is NOP
    return 1;

ma:;;
   bool game = true;

   clear_screen();
   int karc = 0;
   Kar arr[COUNT] = {0};

   get_screen_size(&w, &h);

   table = (Kar*)malloc(w * h * sizeof(Kar));
   if (!table) {
       goto fail;
   }
   init_kar(&karc, arr);

   while (game) {
       {
           int cw = w, ch = h;
           get_screen_size(&w, &h);
           if (w != cw || ch != h) {
               // error terminal resized
               goto fail;
           }
       }

       for (int i = 0; i<w; i++)
           for (int j = 0;j<h;j++) {
               Kar k = table[i*h+j];
           }
       for (int i = 0; i<karc; i++) {
           if (rem_if(arr[i])) {
               arr[i--] = arr[--karc]; // remove unordered
               continue; // to next iteration, same 'i' - so check runs nicely
           }

           Kar k = arr[i];
           arr[i].x += arr[i].dx;
           arr[i].y += arr[i].dy;
           draw_char(k.x, k.y, k.k, k.c);

       }

       if (karc <= 0)
           game = false;

       sleep_ms(100);
       clear_screen();
   }

   clear_screen();

   free(table);
   return 0;
}
