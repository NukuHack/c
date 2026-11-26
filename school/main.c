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
#define Adat TomoritettAdat /* define, not typedef, so no conflict */

TomoritettAdat *tomorit(const char* nyers_szoveg);
bool append_blokk(Adat *adat, size_t *cap, RleBlokk b);
long check_sum(Adat *adat);
void print_adat(Adat *adat);

int main(void) {
    str var = inps();
    if (!var) {
        perror("inp str");
        return 1;
    }
    puts(var);

    Adat *adat = tomorit(var);
    destroy(var);
    if (!adat) {
        fputs("could not compress", stderr);
        return 1;
    }
    print_adat(adat);
    destroy(adat->blokkok);
    destroy(adat);

#if defined(MEMCHECK)
    check_mem_freed();
#endif
    return 0;
}

void print_adat(Adat *adat) {
    if (!adat)
        return;
    if (check_sum(adat) != adat->checksum) {
        puts("checked 'checksum' BUT it does Not match");
        return;
    }
    puts("checked 'checksum' and it matches");
    arr_foreach(RleBlokk, var, adat->blokkok, adat->blokkok_szama) {
        printf("char: '"FMT_char"' bd: '"FMT_int"'\n", isprint(var.karakter) ? var.karakter : '.', var.darabszam);
    }
}
bool append_blokk(Adat *adat, size_t *cap, RleBlokk b) {
    IF_NGROW(RleBlokk, adat->blokkok, cap, (size_t)adat->blokkok_szama) {
        perror("adat, grow");
        destroy(adat->blokkok);
        destroy(adat);
        return false;
    }
    adat->blokkok[adat->blokkok_szama++] = b;
    return true;
}
long check_sum(Adat *adat) {
    if (!adat)
        return -1;
    RleBlokk *arr = adat->blokkok, *end = arr + adat->blokkok_szama, b;
    if (!arr)
        return -1;
    uint cs = 0;
    while (arr != end) {
        b = *arr;
        cs *= 31;
        cs += (unsigned int)b.karakter + (unsigned int)b.darabszam;
        cs %= 1000000007;
        arr++;
    }
    return cs;
}
// might fail -> NULL
// otherwise this needs freeing
Adat *tomorit(const char* txt) {
    size_t len = 0;
    if (!txt || !(len = strlen(txt))) {
        fputs("no input", stderr);
        return NULL;
    }
    Adat *adat = New_c(Adat, 1);
    if(!adat) {
        perror("adat, malloc");
        return NULL;
    }

    size_t cap = 0;
    RleBlokk b = { *txt++, 1 };

    while (*txt) {
        if (*(txt++) == b.karakter)
            b.darabszam++;
        else {
            if (!append_blokk(adat, &cap, b))
                return NULL;
            b.karakter = *txt;
            b.darabszam = 1;
        }
    }

    if (!append_blokk(adat, &cap, b))
        return NULL;

    long cs = check_sum(adat);
    if (cs < 0) {
        fputs("adat, checksum", stderr);
        destroy(adat->blokkok);
        destroy(adat);
        return NULL;
    }

    adat->checksum = (uint)cs; 
    return adat;
}




