Here is a complete reference for every component of the `sscanf` format string, covering all directives, quirks, and edge cases.

---

## The Format String: The Big Picture

A `scanf` format string is a sequence of three kinds of things:

1.  **Whitespace characters** (`' '`, `'\t'`, `'\n'`, etc.)
2.  **Ordinary (non‑whitespace) characters** (everything except `%`)
3.  **Conversion specifications** (starting with `%`)

Understanding how each is treated—especially the differences—is the key to using `sscanf` correctly.

---

## 1. Whitespace Directives

A single whitespace character in the format string is a **directive** that matches **any amount** of whitespace in the input—including zero characters.

| Format char | Behavior |
|---|---|
| `' '` | Skips all consecutive whitespace (`isspace`) in the input |
| `'\t'` | **Identical** to `' '` |
| `'\n'` | **Identical** to `' '` |

> **Crucial quirk:** `\n` in the format string is **not** a literal newline. It is just another whitespace directive, exactly equivalent to a space. To match a **literal newline**, you must use a scanset: `%*1[\n]` or `%[\n]`.

**Examples:**

```c
// All three lines behave identically:
sscanf("1   2", "%d %d", &a, &b); // a=1, b=2
sscanf("1   2", "%d\t%d", &a, &b); // a=1, b=2
sscanf("1   2", "%d\n%d", &a, &b); // a=1, b=2
```

---

## 2. Ordinary (Non‑Whitespace) Characters

Any non‑whitespace character in the format string that is not `%` must **exactly match** the next character in the input. If it does not match, `sscanf` stops immediately and returns the number of successful conversions so far.

**Example:**

```c
// The literal 'x' must be present in the input:
sscanf("3.14x2.71", "%lfx%lf", &d1, &d2); // Matches "3.14x2.71"
sscanf("3.14y2.71", "%lfx%lf", &d1, &d2); // Fails (matching failure at 'x')
```

This is how you enforce separators like commas, semicolons, or arbitrary two‑character separators.

---

## 3. Conversion Specifications: The Full Syntax

Every conversion specification follows this structure:

```
%[*][width][length_modifier]conversion_char
```

| Component | Optional? | Description |
|---|---|---|
| `%` | Required | Introduces the specification |
| `*` | Optional | **Assignment suppression** — parse but do not store |
| `width` | Optional | Maximum number of characters to consume (**decimal integer > 0**) |
| `length_modifier` | Optional | `hh`, `h`, `l`, `ll`, `j`, `z`, `t`, `L` |
| `conversion_char` | Required | The type of conversion (see table below) |

---

## 4. The Complete Table of Conversion Specifiers

| Specifier | Matches | Skips leading whitespace? | Stores NUL? | Argument type (with length modifiers) |
|---|---|---|---|---|
| `%` | A literal `%` character | No | — | (none) |
| `c` | Exactly `width` characters (or 1 if width omitted) | **No** | **No** | `char*` (or `wchar_t*` with `l`) |
| `s` | Non‑whitespace character sequence | Yes | **Yes** | `char*` |
| `[set]` | Non‑empty sequence from a character set | **No** | **Yes** | `char*` |
| `d` | Decimal integer | Yes | — | `int*`, `short*` (`h`), `long*` (`l`), etc. |
| `i` | Integer (auto‑detects base) | Yes | — | Same as `d` |
| `u` | Unsigned decimal integer | Yes | — | `unsigned int*`, etc. |
| `o` | Octal integer | Yes | — | `unsigned int*`, etc. |
| `x`, `X` | Hexadecimal integer | Yes | — | `unsigned int*`, etc. |
| `a`, `A`, `e`, `E`, `f`, `F`, `g`, `G` | Floating‑point number | Yes | — | `float*`, `double*` (`l`), `long double*` (`L`) |
| `n` | **Number of characters read so far** | **No** | — | `int*` (does **not** count toward return value) |
| `p` | Implementation‑defined pointer format | Yes | — | `void**` |

**Key observations from the table:**

- **`%c` and `%[` do *not* skip leading whitespace.** If you need to skip whitespace before a `%c`, add a space in the format string: `" %c"`.
- **`%s` and `%[` always append a null terminator.** Therefore, the destination buffer must be at least `width + 1` characters long.
- **`%c` does *not* append a null terminator.** It reads exactly `width` characters (default 1) and stores them raw.

---

## 5. The Scanset `%[...]`: Advanced Character Matching

The scanset is the most powerful (and most frequently misused) `scanf` feature. It matches a **non‑empty** sequence of characters from a specified set.

### Basic forms

| Syntax | Meaning |
|---|---|
| `%[abc]` | Match one or more characters that are **a**, **b**, or **c** |
| `%[^abc]` | Match one or more characters that are **not** a, b, or c (negation with `^`) |
| `%[a-z]` | Match any lowercase letter (hyphen defines a range) |
| `%[^,\n]` | Match anything except a comma or newline |

### How to include special characters in the set

| Character | How to include it | Example |
|---|---|---|
| `]` (closing bracket) | Make it the **first** character in the set (optionally after `^`) | `%[]abc]` matches `]`, `a`, `b`, `c` |
| `-` (hyphen) | Make it the **last** character before the closing `]` | `%[a-z-]` matches `a`–`z` and `-` |
| `^` (caret) | Make it **not** the first character (otherwise it negates) | `%[a^]` matches `a` and `^` |

### Scanset quirks

- **Empty matches fail.** `%[abc]` requires **at least one** matching character. If the input starts with `d`, the conversion fails (returns 0).
- **No whitespace skipping.** Unlike `%s`, `%[` does **not** skip leading whitespace. If you need to skip whitespace before a scanset, prefix it with a space: `" %[^\n]"`.
- **Width applies to the scanset.** `%20[^\n]` reads at most 20 characters, then stops (useful for bounded line reading).

**Example: Parsing CSV-like data with optional whitespace**

```c
char name[32];
int age;
// %[^,] reads everything up to a comma; width prevents overflow.
// The space before %d skips whitespace after the comma.
sscanf("Alice, 30", "%31[^,], %d", name, &age);
// name = "Alice", age = 30
```

---

## 6. Assignment Suppression (`*`)

The `*` after `%` tells `sscanf` to **parse the field but discard it** (no argument is consumed).

**Examples:**

```c
// Skip a two-character separator, parse two doubles:
sscanf("1.5|:2.5", "%lf%*2c%lf", &a, &b); // a=1.5, b=2.5

// Skip a header word, then read an integer:
sscanf("age: 42", "%*s %d", &age); // age=42
```

**Important quirks:**

- The `*` does **not** count toward the return value.
- `%*n` is **undefined behavior** — never suppress a `%n` conversion.

---

## 7. Field Width

A decimal integer after `%` (and optional `*`) specifies the **maximum number of characters** to consume for that conversion.

- For `%s` and `%[`, the width **does not include** the terminating null byte. Therefore, `%20s` writes at most 21 bytes (20 chars + NUL).
- For `%c`, the width specifies the **exact** number of characters to read (not a maximum).
- Width is **ignored** for `%n`.

**Security note:** **Never use `%s` or `%[` without a width** unless you are absolutely certain the input is bounded. Unbounded `%s` is a buffer‑overflow vulnerability.

---

## 8. Length Modifiers

Length modifiers tell `sscanf` the size of the destination type. They are essential when you are not parsing into the default `int` or `double`.

| Modifier | Applies to | Destination type |
|---|---|---|
| `hh` | `d`, `i`, `o`, `u`, `x`, `X`, `n` | `signed char*` / `unsigned char*` |
| `h` | `d`, `i`, `o`, `u`, `x`, `X`, `n` | `short*` / `unsigned short*` |
| (none) | `d`, `i`, `o`, `u`, `x`, `X`, `n` | `int*` / `unsigned int*` |
| `l` | `d`, `i`, `o`, `u`, `x`, `X`, `n` | `long*` / `unsigned long*` |
| `l` | `f`, `e`, `g`, `a` | `double*` (**required** for `double`) |
| `ll` | `d`, `i`, `o`, `u`, `x`, `X`, `n` | `long long*` / `unsigned long long*` |
| `j` | Integer types | `intmax_t*` / `uintmax_t*` |
| `z` | Integer types | `size_t*` (signed counterpart `ssize_t` is not standard) |
| `t` | Integer types | `ptrdiff_t*` |
| `L` | `f`, `e`, `g`, `a` | `long double*` |

**Important quirk:** For `scanf`, `%f` means `float*`; `%lf` means `double*`. This is the **opposite** of `printf` (where `%f` means `double`). Always use `%lf` when reading into a `double`.

---

## 9. Escaping the Percent Sign (`%%`)

To match a literal `%` in the input, use `%%` in the format string. This performs **no conversion** and consumes **no argument**.

**Example:**

```c
int n;
// Input must contain "50%" followed by an integer:
sscanf("50% off 20", "50%% off %d", &n); // n = 20
```

**Quirk:** `%%` cannot be combined with `*`, width, or length modifiers. It is a standalone directive.

---

## 10. The `%n` Directive

`%n` stores the **number of characters consumed so far** (up to that point) into an `int*` argument. It does **not** read any input and does **not** count toward the return value of `sscanf`.

**Primary use case:** Detecting trailing garbage.

```c
double d;
int n = -1;
if (sscanf(s, "%lf%n", &d, &n) != 1) {
    // Failed before %n, n may be unchanged — always initialize n.
}
if (s[n] != '\0') {
    // Trailing characters exist after the number.
}
```

**Quirks:**

- `%n` **does not** skip leading whitespace.
- `%n` **does not** increment the return value.
- `%*n` is **undefined behavior** — never suppress it.
- If a previous conversion fails, `%n` is **not executed**, and the variable may be left untouched. Always gate on the return value first.

---

## 11. Return Value and EOF

The return value of `sscanf` is the **number of receiving arguments successfully assigned**, or `EOF` if an **input failure** occurs before the first conversion completes.

| Situation | Return value |
|---|---|
| All conversions succeed | Number of conversions |
| Some conversions succeed, then one fails | Number of successful conversions so far |
| First conversion fails (matching failure, e.g., `"abc"` for `%lf`) | `0` |
| Input exhausted before any conversion (e.g., `""` for `%lf`) | `EOF` (negative) |

**Critical rule:** **Never compare the return value to `0` or `EOF` alone.** Always compare to the **exact number of conversions you expect**, e.g., `if (sscanf(...) != 2) { /* error */ }`. This single check correctly handles EOF, partial matches, and matching failures.

---

## 12. Summary of Common Pitfalls

| Pitfall | Correct approach |
|---|---|
| Using `%s` without a width | Always bound: `%20s`, `%255[^\n]` |
| Using `%f` for a `double` | Use `%lf` (the `l` is mandatory for `double`) |
| Expecting `\n` to match a literal newline | Use `%*1[\n]` or `%[\n]`; `\n` in format is just whitespace |
| Forgetting `%c` / `%[` do not skip whitespace | Add a space: `" %c"`, `" %[^\n]"` |
| Ignoring the return value | Check `== N` (exact count), never `== 0` or `== EOF` |
| Using `%n` without `%` | `%n` is a conversion; `n` alone is an ordinary character |
| Suppressing `%n` (`%*n`) | Undefined behavior — never do it |
| Assuming `%n` counts toward return value | It does not; it is a side‑effect only |
| Using an unbounded `%[` | Always specify a width: `%20[^\n]` |

---

## 13. Quick Reference Card

```
Format = [whitespace] [ordinary char] [%[*][width][length]conv] ...

conv:
  %%    literal %          (no arg)
  %c    chars (no NUL)     char*        (no ws skip)
  %s    word (adds NUL)    char*        (skips ws)
  %[]   set  (adds NUL)    char*        (no ws skip)
  %d    decimal int        int*         (skips ws)
  %i    auto-base int      int*         (skips ws)
  %u    unsigned decimal   unsigned*    (skips ws)
  %o    octal              unsigned*    (skips ws)
  %x    hex                unsigned*    (skips ws)
  %a %e %f %g  floating    float*/double*/long double*  (skips ws)
  %n    chars consumed     int*         (no input, no count)
  %p    pointer            void**       (skips ws)

Whitespace in format: skips any amount of whitespace (including none).
Ordinary char in format: must match exactly.
```
