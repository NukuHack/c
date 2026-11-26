#include "my_utils.h"
#include "my_io.h"
#include "my_vec.h"

typedef struct RleBlokk {
    char karakter;          // Az ismétlődő karakter
    int darabszam;          // Hányszor ismétlődik egymás után
} RleBlokk;
typedef struct TomoritettAdat {
    int blokkok_szama;      // A dinamikus tömb aktuális mérete
    unsigned int checksum;  // Az adatintegritást védő ellenőrző összeg
    RleBlokk* blokkok;      // Dinamikusan foglalt tömb a blokkoknak
} TomoritettAdat;

TomoritettAdat *tomorit(const char* nyers_szoveg);

int main(void) {
    str var = inps();
    if (!var) {
        perror("inp str");
        return 1;
    }
    puts(var);

    TomoritettAdat *adat = tomorit(var);
    destroy(var);
    if (!adat) {
        fputs("could not compress", stderr);
        return 1;
    }
    arr_foreach(RleBlokk, var, adat->blokkok, adat->checksum) {
        printf("char: '"FMT_char"' bd: '"FMT_int"'\n", var.karakter, var.darabszam);
    }
    destroy(adat->blokkok);
    destroy(adat);

#if defined(MEMCHECK)
    check_mem_freed();
#endif
    return 0;
}

typedef struct {
    size_t capacity;
    size_t count;
    RleBlokk *data;
} RleVec;
// might fail -> NULL
// otherwise this needs freeing
TomoritettAdat *tomorit(const char* nyers_szoveg) {
    cstr txt = nyers_szoveg; // too long var, name
    size_t len = 0;
    if (!txt || !(len = strlen(txt))) {
        fputs("no input", stderr);
        return NULL;
    }
    TomoritettAdat *adat = New_c(TomoritettAdat, 1);
    if(!adat) {
        perror("adat, malloc");
        return NULL;
    }
    char cur = *(txt++);
    RleBlokk b = {cur, 1};
    size_t cap = 0;
    forx(len) {
        if (cur == *txt) {
            b.darabszam++;
        } else {
            IF_NGROW(RleBlokk, adat->blokkok, &cap, (size_t)adat->blokkok_szama) {
                perror("vec, grow");
                destroy_arr(adat->blokkok, adat->blokkok_szama);
                return NULL;
            }
            adat->blokkok[adat->blokkok_szama++] = b;
            b.karakter = *txt;
            b.darabszam = 1;
        }
        cur = *txt; txt++;
        if (i == len && cur != b.karakter) {
            IF_NGROW(RleBlokk, adat->blokkok, &cap, (size_t)adat->blokkok_szama) {
                perror("vec, grow");
                destroy_arr(adat->blokkok, adat->blokkok_szama);
                return NULL;
            }
            adat->blokkok[adat->blokkok_szama++] = b;
        }
    }
    adat->checksum = (uint)len;
    return adat;
}




