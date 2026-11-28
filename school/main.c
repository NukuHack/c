#include "my_utils.h"
#include "my_io.h"

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
char* kitomorit(const TomoritettAdat* adat);
Adat *parse(str raw);
size_t length(const Adat* adat);
bool append_blokk(Adat *adat, size_t *cap, RleBlokk b);
long check_sum(const Adat *adat);
bool print_adat(const Adat *adat, FILE *out);

void print_header(const Adat *adat) {
    long ch = check_sum(adat);
    size_t len = length(adat);
    bool edited = adat->checksum != ch;
    fprintf(stdout, "Parsed Adat : \n"
            "check:'"FMT_uint"' calculated check:'"FMT_long"'\n"
            "it "FMT_str" manually edited\n"
            "block-count:'"FMT_int"' char-len:'"FMT_size_t"'\n",
            adat->checksum, ch,
            edited?"was":"wasn't",
            adat->blokkok_szama, len
        );
}

str read_from(const char *arg, bool decomp) {
    if (!arg) return NULL;
    str var = NULL;
    size_t len = 0;
    if (Strcmp(arg, ==, "")) {
        var = inps();
        if (!var) {
            perror("inp str");
            return NULL;
        }
        len = strlen(var);
    } else {
        FILE *fp = fopen(arg, "r");
        if (!fp) {
            perror("file open");
            return NULL;
        }
        size_t cap = 0;
        // here we assume that the user does not want to read in a file bigger than their ram
        ptrdiff_t red = fgetbatch(&var, &cap, fp, false);
        fclose(fp);
        if (red == 0) {
            fputs("no file data\n", stderr);
            return NULL;
        } else if (red < 0) {
            perror("file read");
            return NULL;
        }
        len = (size_t)red;
    }
    if (decomp) {
        Adat *adat = parse(var);
        destroy(var);
        if (!adat) {
            return NULL;
        }
        str raw_txt = kitomorit(adat);
        destroy(adat->blokkok);
        destroy(adat);
        if (!raw_txt) {
            return NULL;
        }
        var = raw_txt;
        len = strlen(var);
    }
    str txt = New(char, len + 1);
    if (!txt) {
        destroy(var);
        perror("malloc");
        return NULL;
    }
    memcpy(txt, var, len+1); // including 0 term
    destroy(var);
    return txt;
}

bool print_wrap(const Adat *adat, FILE *out, bool comp) {
    if (comp)
        return print_adat(adat, out);

    str txt = kitomorit(adat);
    if (!txt)
        return false;
    size_t len = strlen(txt);
    ptrdiff_t wrote = fputbatch(&txt, &len, out, false);
    destroy(txt);
    if (wrote <= 0 || len != (size_t)wrote) {
        perror("write decomp");
        return false;
    }
    return true;
}
bool write_to(const char *arg, const Adat *adat, bool comp) {
    if (!arg) return false;
    bool ret = false;
    errno = 0;

    if (Strcmp(arg, ==, "")) {
        ret = print_wrap(adat, stdout, comp);
        goto ending;
    }

    if (file_exist(arg)) {
        fputs("out file already exist, overwrite?\n", stdout);
        if (!confirm("confirm [y/n]: "))
            return false;
    }
    FILE *fp = fopen(arg, "w");
    if (!fp) {
        perror("output file open");
        return false;
    }
    ret = print_wrap(adat, fp, comp);
    fclose(fp);

ending:;;
    if (!ret) {
        if (!errno) errno = EINVAL;
        perror("write failed");
    }
    return ret;
}

int main(void) {
    bool ret = 1;
    goto body;
memch:;
#if defined(MEMCHECK)
    check_mem_freed();
#endif
    return ret;
body:;
    str inp = inps_s("Input file (default stdin): ");
    if (!inp) inp = "";
    bool decomp = confirm("encoded? [y/n]: ");
    str txt = read_from(inp, decomp);
    destroy(inp);
    if (!txt) {
        goto memch;
    }

    Adat *adat = tomorit(txt);
    destroy(txt);
    if (!adat) {
        fputs("could not compress", stderr);
        goto memch;
    }
    print_header(adat);

    str out = inps_s("Output file: ");
    if (!out) out = "";
    bool comp = confirm("compress? [y/n]: ");
    ret = write_to(out, adat, comp);
    destroy(out);
    destroy(adat->blokkok);
    destroy(adat);

    goto memch;
}

cstr next_line(str *text) {
    size_t len = strlen(*text);
    str nil = memchr(*text, '\n', len);
    if (!nil) return NULL;
    *nil = '\0';
    str f = *text;
    *text = nil + 1;
    return (cstr)f;
}
#define advance(line, p) \
    do { \
        if (!p) { \
            perror("no number data"); \
            goto ending; \
        } else if (p < 0) { \
            perror("bad number data"); \
            goto ending; \
        } else { \
            line += p; \
        } \
    } while (0)

Adat *parse(str raw) {
    size_t len = 0;
    if (!raw || !(len = strlen(raw))) {
        fputs("no input", stderr);
        return NULL;
    }
    Adat *adat = New_c(Adat, 1);
    if(!adat) {
        perror("adat, malloc");
        return NULL;
    }
    ptrdiff_t p = 0;
    cstr line = next_line(&raw);
    line += strlen("HEADER: ");
    int count = 0;
    p = parse_i(line, &count);
    advance(line, p);
    line += strlen(";CS:");
    uint t_check = 0;
    p = parse_u(line, &t_check);      // i don't have a long parser ... yet 
    advance(line, p);
    long check = (long)t_check;   // TODO : parse long
    
    RleBlokk b = {0}, *arr = New_c(RleBlokk, (size_t)count);
    if (!arr) {
        destroy(adat);
        perror("malloc");
        return NULL;
    }
    adat->blokkok = arr;
    size_t cap = (size_t)count;

    while ((line = next_line(&raw))) {
        if (*line == '\0') continue;
        int tmpc = 0; uint tmpu = 0;
        p = parse_i(line, &tmpc);
        advance(line, p);
        b.karakter = (char)tmpc;
        line++;
        p = parse_u(line, &tmpu);
        advance(line, p);
        b.darabszam = (int)tmpu;
        if (!append_blokk(adat, &cap, b))
            goto ending;
    }

    long cs = check_sum(adat);
    if (cs < 0) {
        fputs("adat, checksum", stderr);
        goto ending;
    } else if (cs != check) {
        fprintf(stderr, "adat, checksum edited (og: "FMT_long", new: "FMT_long"), aborting!!\n", cs, check);
        goto ending;
    }

    adat->checksum = (uint)cs; 
    return adat;
ending:;;
    if (adat->blokkok)
        destroy(adat->blokkok);
    destroy(adat);
    return NULL;
}

bool print_adat(const Adat *adat, FILE *out) {
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
long check_sum(const Adat *adat) {
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

size_t length(const Adat* adat) {
    if (!adat) return 0;
    RleBlokk *arr = adat->blokkok, *end = arr + adat->blokkok_szama, b;
    if (!arr) return 0;
    size_t len = 0;
    while (arr != end) {
        b = *arr;
        len += (size_t)b.darabszam;
        arr++;
    }
    return len;
}
char *kitomorit(const Adat* adat) {
    if (!adat) return NULL;
    size_t len = length(adat);
    if (!len) return NULL;
    char *txt = New(char, len + 1);
    if (!txt) return NULL;

    RleBlokk *arr = adat->blokkok, *end = arr + adat->blokkok_szama, b;
    if (!arr) return NULL;
    size_t cur = 0;
    while (arr != end) {
        b = *arr;
        memset(txt + cur, b.karakter, (size_t)b.darabszam);
        cur += (size_t)b.darabszam;
        arr++;
    }
    return txt;
}

bool append_blokk(Adat *adat, size_t *cap, RleBlokk b) {
    IF_NGROW(RleBlokk, adat->blokkok, cap, (size_t)adat->blokkok_szama) {
        perror("adat, grow");
        return false;
    }
    adat->blokkok[adat->blokkok_szama++] = b;
    return true;
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
                goto ending;
            b.karakter = *txt;
            b.darabszam = 1;
        }
        txt++;
    }

    if (!append_blokk(adat, &cap, b))
        goto ending;

    long cs = check_sum(adat);
    if (cs < 0) {
        fputs("adat, checksum", stderr);
        goto ending;
    }

    adat->checksum = (uint)cs; 
    return adat;
ending:;;
    destroy(adat->blokkok);
    destroy(adat);
    return NULL;
}

