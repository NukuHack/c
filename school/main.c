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
bool print_adat_file(Adat *adat, FILE *out);


int main(int argc, char **argv) {
    str *arg = shift_args(&argc, &argv);
    str var = NULL;
    size_t len = 0;
    if (!argc) {
        var = inps();
        if (!var) {
            perror("inp str");
            return 1;
        }
        len = strlen(var);
    } else {
        arg = shift_args(&argc, &argv);
        FILE *fp = fopen(*arg, "r");
        if (!fp) {
            perror("file open");
            return 1;
        }
        size_t cap = 0;
        // here we assume that the user does not want to read in a file bigger than their ram
        ptrdiff_t red = fgetbatch(&var, &cap, fp, false);
        fclose(fp);
        if (red == 0) {
            fputs("no file data\n", stderr);
            return 1;
        } else if (red < 0) {
            perror("file read");
            return 1;
        }
        len = (size_t)red;
    }
    str txt = New(char, len + 1);
    if (!txt) {
        destroy(var);
        perror("malloc");
        return 1;
    }
    memcpy(txt, var, len+1); // including 0 term
    destroy(var);
    puts(txt);

    Adat *adat = tomorit(txt);
    destroy(txt);
    if (!adat) {
        fputs("could not compress", stderr);
        return 1;
    }
    //print_adat(adat);
    bool ret = 0;
    errno = 0;
    if (!argc) {
        ret = print_adat_file(adat, stdout);
    } else {
        arg = shift_args(&argc, &argv);
        if (file_exist(*arg)) {
            puts("out file already exist, overwrite?");
            if (!confirm())
                goto drop;
        }
        FILE *fp = fopen(*arg, "w");
        if (!fp) {
            perror("output file open");
            goto drop;
        }
        ret = print_adat_file(adat, fp);
        fclose(fp);
    }
    if (!ret) {
        if (!errno) errno = EINVAL;
        perror("write failed");
        goto drop;
    }
    destroy(adat->blokkok);
    destroy(adat);

#if defined(MEMCHECK)
    check_mem_freed();
#endif
    return 0;
drop:;;
    destroy(adat->blokkok);
    destroy(adat);
    return 1;
}

bool print_adat_file(Adat *adat, FILE *out) {
    if (!adat)
        return false;
    if (check_sum(adat) != adat->checksum) { // only for debug
        puts("checked 'checksum' BUT it does Not match");
        return false;
    }
    // HEADER: <blokkok_szama>;CS:<kiszámított_checksum>
    fprintf(out, "HEADER: "FMT_int";CS:"FMT_uint"\n", adat->blokkok_szama, adat->checksum);
    arr_foreach(RleBlokk, var, adat->blokkok, adat->blokkok_szama) {
        // <karakter_ascii_kodja>;<darabszam>
        fprintf(out, "%3d;"FMT_int"\n", (uchar)var.karakter, var.darabszam); // uchar so parsing is easier
    }
    return true;
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
        printf("char: '"FMT_char"' bd: '"FMT_int"'\n",
                isprint(var.karakter) ? var.karakter : '.', var.darabszam);
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
    if (!adat) return -1;
    RleBlokk *arr = adat->blokkok, *end = arr + adat->blokkok_szama, b;
    if (!arr) return -1;
    long cs = 0;
    while (arr != end) {
        b = *arr;
        cs *= 31;
        cs += (unsigned int)b.karakter + (unsigned int)b.darabszam;
        cs %= 1000000007;
        arr++;
    }
    return (uint)cs;
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
        if (*txt == b.karakter)
            b.darabszam++;
        else {
            if (!append_blokk(adat, &cap, b))
                return NULL;
            b.karakter = *txt;
            b.darabszam = 1;
        }
        txt++;
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




