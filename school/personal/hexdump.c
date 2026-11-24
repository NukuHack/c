#include "my_utils.h"
#include "my_io.h"

#define BUF 16
#define HLF (BUF / 2)
int print_a_file(FILE *fp, FILE *out) {
    char buf[BUF];
    int cnt = 0, addr = 0;

    while ((cnt = fread(buf, 1, BUF, fp))) {
        if (cnt < 0)
            return 1;

        fprintf(out, "%08x  ", addr); // addr + sep

        forx(MIN(cnt, HLF)) {
            fprintf(out, "%02x", buf[i]&0xff); // 2 char in hex
            fputc(' ', out); // sep
        }
        forx(MAX(0, HLF - cnt)) {
            fputs("   ", out); // if not long enough pad
        }
        fputc(' ', out); // separator
        if (cnt > HLF) {
            forx(cnt - HLF) {
                fprintf(out, "%02x", buf[i + HLF]&0xff); // 2 char in hex
                fputc(' ', out); // sep
            }
        }
        forx(MAX(0, HLF - MAX(0, cnt - HLF))) {
            fputs("   ", out); // if not long enough pad
        }

        fputs(" |", out); // separator
        forx(cnt) {
            fputc(isprint(buf[i]) ? buf[i] : '.', out); // print all printable
        }
        forx(BUF - cnt) {
            fputc(' ', out); // padding if short
        }
        fputs("|\n", out); // end + NewLine

        addr+=cnt;
    }
    return 0;
}

int main(int argc, char **argv) {
    char **arg = shift_args(&argc, &argv);
    if (!argc)
        return print_a_file(stdin, stdout);

    int ret = 0;
    while ((arg = shift_args(&argc, &argv)) && *arg) {
        FILE *fp = fopen(*arg, "rb");
        if (!fp) {
            fprintf(stderr, "File (%s) open: %s\n", *arg, strerror(errno));
            ret++;
            continue;
        }
        fprintf(stdout, "File: %s\n", *arg);
        ret += print_a_file(fp, stdout);
        fclose(fp);
    }
    return ret;
}
