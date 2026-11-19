
// used for char, allows stuff that returns true
typedef int (predicate)(int);

/* input result */
typedef enum {
    INPUT_OK = 0,
    INPUT_EOF,
    INPUT_EMPTY,
    INPUT_NOT,
    INPUT_OVERFLOW,
    INPUT_STOPPED,
    INPUT_RANGE,
    INPUT_SEP,
} inp_res;

void consume_crlf_tail(FILE *in) {
    int c = fgetc(in);
    if (c == EOF) return;
    if (c != '\n') ungetc(c, in);
}

bool finish_number(FILE *in, int c) {
    if (c == EOF || c == '\n')
        return false;
    if (c == '\r') {
        consume_crlf_tail(in);
        return false;
    }
    ungetc(c, in);
    return true;
}

void new_line(FILE *out) {
    if (!out) return;
    fputc('\n', out);
    fflush(out);
}

void handle_eof(FILE *in, FILE *out) {
    clearerr(in);
    new_line(out);
}
bool input_rejected(inp_res s) {
    // reject everything that is not Good or Error
    return !(s == INPUT_OK || s == INPUT_EOF);
}
bool input_drain_required(inp_res s) {
    switch (s) {
        default:
        case INPUT_EOF:
        case INPUT_EMPTY:
        case INPUT_OK: return false;
        case INPUT_SEP:
        case INPUT_NOT:
        case INPUT_OVERFLOW:
        case INPUT_RANGE:
        case INPUT_STOPPED: return true;
    }
}
const char *input_status_str(inp_res s) {
    switch (s) {
        case INPUT_OK: return "ok"; // 123
        case INPUT_SEP: return "expected char not found"; // 123;123 instead of 123-123
        case INPUT_NOT: return "not correct type"; // apple
        case INPUT_EOF: return "end of input"; // EOF
        case INPUT_EMPTY: return "empty"; //  \n  -  \r\n
        case INPUT_RANGE: return "out of range"; // [10;40] - 300
        case INPUT_STOPPED: return "stopped (partially usable input)"; // 1234a
        case INPUT_OVERFLOW: return "overflow"; // more than 2^64 ...
        default: return "unknown";
    }
}

/*  separator  */

inp_res input_sep(FILE *in, int expected) {
    int c = fgetc(in);
    if (c == EOF) return INPUT_EOF;
    if (c == expected) return INPUT_OK;
    ungetc(c, in);
    return INPUT_SEP;
}

/*  utility  */
// IMPORTANT : if you call this in the input make sure to NOT clear eof flag from the input before it
// otherwise it will drain more than needed
void drain_line(FILE *in, FILE *out) {
    int c;
    while ((c = fgetc(in)) != '\n' && c != '\r' && c != EOF);

    if (c == '\r' && (c = fgetc(in)) != '\n' && c != EOF)
        ungetc(c, in);

    /* we ran out of input before any line terminator -> the user's
       cursor is still sitting at the end of the current line, so
       emit one ourselves to keep prompts/errors on their own line */
    if (c == EOF) {
        handle_eof(in, out);
    }
}

/* char-by-char read + parse -> no malloc */
// usually parsers work like this :
// go through the buffer and try to parse it
// but in fact we can use the FILE stream as a buffer here, and go by char-s to have the "same" result
// but since we can't look ahead without destroying too much we can't parse special double cases
// - stuff like "nan" and "inf" or "infinity" are just errors now ...

inp_res input_int(FILE *in, FILE *out, int *v, int min, int max, const char *prompt) {
    if (min > max) { int t = min; min = max; max = t; }

    if (prompt && out) { fputs(prompt, out); fflush(out); }

    int c = fgetc(in);
    if (c == EOF)
        return INPUT_EOF;
    if (c == '\n' || c == '\r') {
        if (c == '\r') consume_crlf_tail(in);
        return INPUT_EMPTY;
    }

    int sign = 1;
    if (c == '+' || c == '-') {
        if (c == '-') sign = -1;
        c = fgetc(in);
        if (c == EOF)
            return INPUT_EOF;
        if (c == '\n' || c == '\r') {
            ungetc(c, in);
            return INPUT_NOT;
        }
        if (c < '0' || c > '9') {
            ungetc(c, in);
            return INPUT_NOT;
        }
    } else if (c < '0' || c > '9') {
        ungetc(c, in);
        return INPUT_NOT;
    }

    long long n = 0;
    bool overflow = false;

    do {
        int d = c - '0';
        if (!overflow) {
            long long limit = (sign > 0)
                ? INT_MAX : (long long)INT_MAX + 1;

            if (n > (limit - d) / 10)
                overflow = true;
            else
                n = n * 10 + d;
        }
    } while ((c = fgetc(in)) >= '0' && c <= '9');

    long long signed_n = n * sign;

    if (overflow) {
        if (c != EOF) ungetc(c, in);
        return INPUT_OVERFLOW;
    } else if (signed_n < min || signed_n > max) {
        if (c != EOF) ungetc(c, in);
        return INPUT_RANGE;
    }

    if (c == EOF) {
        clearerr(in);
        new_line(out);
    }
    bool stopped = finish_number(in, c);
    if (v) *v = (int)signed_n;
    return stopped ? INPUT_STOPPED : INPUT_OK;
}

inp_res input_uint(FILE *in, FILE *out, uint *v, uint min, uint max, const char *prompt) {
    if (min > max) { uint t = min; min = max; max = t; }

    if (prompt && out) { fputs(prompt, out); fflush(out); }

    int c = fgetc(in);
    if (c == EOF) return INPUT_EOF;
    if (c == '\n' || c == '\r') {
        if (c == '\r') consume_crlf_tail(in);
        return INPUT_EMPTY;
    }

    if (c == '+') {
        c = fgetc(in);
        if (c == EOF)
            return INPUT_EOF;
        if (c == '\n' || c == '\r') {
            ungetc(c, in);
            return INPUT_NOT;
        }
        if (c < '0' || c > '9') {
            ungetc(c, in);
            return INPUT_NOT;
        }
    } else if (c < '0' || c > '9') {
        ungetc(c, in);
        return INPUT_NOT;
    }

    uint n = 0;
    bool overflow = false;

    do {
        uint d = (uint)(c - '0');
        if (!overflow) {
            if (n > (UINT_MAX - d) / 10u)
                overflow = true;
            else
                n = n * 10u + d;
        }
    } while ((c = fgetc(in)) >= '0' && c <= '9');

    if (overflow) {
        if (c != EOF) ungetc(c, in);
        return INPUT_OVERFLOW;
    } else if (n < min || n > max) {
        if (c != EOF) ungetc(c, in);
        return INPUT_RANGE;
    }

    if (c == EOF) {
        clearerr(in);
        new_line(out);
    }
    bool stopped = finish_number(in, c);
    if (v) *v = n;
    return stopped ? INPUT_STOPPED : INPUT_OK;
}

inp_res input_double(FILE *in, FILE *out, double *v, double min, double max, const char *prompt) {
    if (min > max) { double t = min; min = max; max = t; }

    if (prompt && out) { fputs(prompt, out); fflush(out); }

    int c = fgetc(in);
    if (c == EOF) return INPUT_EOF;
    if (c == '\n' || c == '\r') {
        if (c == '\r') consume_crlf_tail(in);
        return INPUT_EMPTY;
    }

    int sign = 1;
    if (c == '+' || c == '-') {
        if (c == '-') sign = -1;
        c = fgetc(in);
        if (c == EOF) return INPUT_EOF;
        if (c == '\n' || c == '\r') {
            ungetc(c, in);
            return INPUT_NOT;
        }
        if ((c < '0' || c > '9') && c != '.') {
            ungetc(c, in);
            return INPUT_NOT;
        }
    } else if ((c < '0' || c > '9') && c != '.') {
        ungetc(c, in);
        return INPUT_NOT;
    }

    double int_part = 0.0;
    double frac_part = 0.0;
    double frac_scale = 1.0;
    int norm_digit = 0;
    int frac_digits = 0;

    while (c >= '0' && c <= '9') {
        // if (norm_digit < 20) {
        // while double only keeps perfect precission up to digit ~16 i decided to let go of that
        // so this will parse insainly big (~1e300, aka just barely under inf) big doubles too
        int_part = int_part * 10.0 + (c - '0');
        norm_digit++;
        // }
        c = fgetc(in);
    }
    if (c == '.') {
        c = fgetc(in);
        while (c >= '0' && c <= '9') {
            if (frac_digits < 20) {
                /* double keeps precission up to 16 digits, 20 should be fine for decimal places */
                frac_part = frac_part * 10.0 + (c - '0');
                frac_scale *= 10.0;
                frac_digits++;
            }
            c = fgetc(in);
        }
    }

    if (!norm_digit && !frac_digits) {
        if (c != EOF) ungetc(c, in);
        return INPUT_NOT;
    }

    double d = (int_part + frac_part / frac_scale) * sign;

    if (isinf(d)) {
        // in a real double input infinity would be acceptable, but here it is not
        // since we don't parse strings infinity can only occur at overflow
        if (c != EOF) ungetc(c, in);
        return INPUT_OVERFLOW;
    } else if (d < min || d > max) {
        if (c != EOF) ungetc(c, in);
        return INPUT_RANGE;
    }

    if (c == EOF) {
        clearerr(in);
        new_line(out);
    }
    bool stopped = finish_number(in, c);
    if (v) *v = d;
    return stopped ? INPUT_STOPPED : INPUT_OK;
}

inp_res input_char(FILE *in, FILE *out, char *v, predicate *filter, const char *prompt) {
    if (prompt && out) { fputs(prompt, out); fflush(out); }

    int ch = fgetc(in);
    if (ch == EOF) { // EOF
        handle_eof(in, out);
        return INPUT_EOF;
    }
    if (ch == '\n' || ch == '\r') {
        if (ch == '\r') consume_crlf_tail(in);
        return INPUT_EMPTY;
    }
    if (filter && !filter(ch))
        return INPUT_NOT;
    
    int c = fgetc(in);
    if (c == '\r') {
        consume_crlf_tail(in);
    } else if (c != '\n' && c != EOF)
        return INPUT_STOPPED;

    *v = (char)ch;
    return INPUT_OK;
}

