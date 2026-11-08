#include <stdlib.h>
#include <stdio.h>

#define BUFF_LEN 1024

int main(int argc, const char** args) {
    if (1 == argc) {
        fprintf(stdout, "Usage %s <file>\n", args[0]);
        return 1;
    }
/*
    for (int i = 0; i < argc; i++) {
        fprintf(stdout, "arg: %s\n", args[i]);
    }
*/
    FILE *pFile = fopen(args[1], "r");
    if (!pFile) {
        fputs("could not open file\n", stderr);
        return 1;
    }

    char buff[BUFF_LEN] = {0};

    while (fgets(buff, BUFF_LEN, pFile)) {
        fputs(buff, stdout);
    }
    fclose(pFile);
    
    return 0;
} 
int read_main(int _f, const char** argc) {
    char *file = "file_op.c";
    char *arr[2] = {argc[0], file};
    return main(2, &arr);
}

static const char* msg = "Here is a message for a file write\n \t with some extra white spaces\n\n";

int write(const char* name) {
    if (!name) {
        fputs("no file to write\n", stderr);
        return 1;
    }

    FILE *pFile = fopen(name, "w");
    if (!pFile) {
        fputs("could not open file\n", stderr);
        return 1;
    }

    if(!fputs(msg, pFile)) {
        fputs("could not write in file\n", stderr);
        fclose(pFile);
        return 1;
    }
    fflush(pFile);
    fclose(pFile);
    
    return 0;
}
int write_main(void) {
    char *file = "temp.txt";
    return write(file);
}


