## Basic terminals typically use a set of 8 standard colors (plus bright variants in many terminals) with predefined escape codes.  
## These are based on ANSI escape sequences. The codes are:  

## Standard colors (low intensity, often normal/dark):  
- Black: \033[30m
- Red: \033[31m
- Green: \033[32m
- Yellow: \033[33m
- Blue: \033[34m
- Magenta: \033[35m
- Cyan: \033[36m
- White: \033[37m

## Bight colors (high intensity, often bold/bright):  
- Bright Black (Gray): \033[90m
- Bright Red: \033[91m
- Bright Green: \033[92m
- Bright Yellow: \033[93m
- Bright Blue: \033[94m
- Bright Magenta: \033[95m
- Bright Cyan: \033[96m
- Bright White: \033[97m

## Background colors (standard):  
- Black background: \033[40m
- Red background: \033[41m
- Green background: \033[42m
- Yellow background: \033[43m
- Blue background: \033[44m
- Magenta background: \033[45m
- Cyan background: \033[46m
- White background: \033[47m

## Bright background colors (not supported in all terminals, but common):  
- Bright Black background: \033[100m
- Bright Red background: \033[101m
- Bright Green background: \033[102m
- Bright Yellow background: \033[103m
- Bright Blue background: \033[104m
- Bright Magenta background: \033[105m
- Bright Cyan background: \033[106m
- Bright White background: \033[107m

## Reset code to revert to default terminal colors: \033[0m  
 - Note: In many contexts, \033 can be replaced with \e or \x1b.
 - These are equivalent representations of the ASCII escape character.

## Escape stuff: - other thing with custom behaviour in terminals  

 - \033[A -	up 1 line
 - \033[B -	down 1 line
 - \033[C -	right 1 col
 - \033[D -	left 1 col
 - \033[H -	move to top-left of screen
 - \033[<row>;<col>H -	move to specific position
 - \033[K -	clear from cursor to end of line
 - \033[2K -	clear entire line
 - \033[J -	clear from cursor to end of screen
 - \033[2J -	clear entire screen
 - \033[s / \033[u -	save / restore cursor position
 - \033 - is ESC (octal 033 = 0x1B).

# Char stuff:  
 - \n - new line
 - \r - return to start of the line
 - \t - simple tab

# C\_strings : the non ascii version
In C wide strings are the first try to make "non ascii strings" while it works flawlessly for utf8, 90%-time for utf16, but for utf32 it is not so much
Sadly in c99 (what we use) there is no answer for nice-easy non-ascii strings, that are reliable and non-platform dependent
the wchar\_t mostly works on posix, but if we are on posix there is a lib `iconv` that solves the problems even better
-- ai
Good news is that your exact request—a fixed-width character type like int32\_t—was answered by the C standard itself, starting with C11.

🎯 The Solution: char32\_t (and char16\_t)

The C11 standard introduced two new character types specifically to solve the portability problem of wchar\_t:

char32\_t: This is exactly what you are looking for. It is an unsigned integer type that is guaranteed to be at least 32 bits wide.

char16\_t: Similarly, this is an unsigned integer type guaranteed to be at least 16 bits wide, intended for UTF-16.

These types are defined in the <uchar.h> header.


