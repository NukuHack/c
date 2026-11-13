#include <stdio.h> // basic io
#include <stdlib.h> // "NULL" + mem
#include <string.h> // memory op
#include <ctype.h> // char conversion
#include <stdbool.h> // bool
#include "my_utils.h" // better io (renamed from small_lib.h)
#include "input.h" // input

#if defined(MY_MEMCHECK)
#include "memcheck.h"
#endif

int main(int _argc, char **_argv) {
    int argc = _argc;
    char **argv = _argv;

    if (argc == 1) {
        printf("Usage : %s <num>\n", argv[0]);
        return 1;
    }

    switch(atoi(argv[1])) {
        case(1) : {
                      char buf[1024];
                      if(!read_str("input a line: ", ARR_L(buf)))
                          return 1;

                      int wc = count_words(buf, " "); // uses string.h underneath
                      int cc = strlen(buf);
                      printf("word count: %i, char count: %i \n", wc, cc);
                  } break;
        case (2) : {
                       char buf[1024];
                       if(!read_str("input a line: ", ARR_L(buf)))
                           return 1;
                       char buf2[1024];
                       if(!read_str("input another line: ", ARR_L(buf2)))
                           return 1;

                       int f = strcmp(buf, buf2);
                       if (f == 0) {
                           print("equal");
                       } else if (f > 0) {
                           print("second");
                       } else {
                           print("first");
                       }
                   } break;
        case(3) : {
                      char buf[1024];
                      if(!read_str("a line to copy: ", ARR_L(buf)))
                          return 1;
                      char bufcpy[1024];

                      memcpy(bufcpy, buf, strlen(buf));

                      printf("copied string \n%s\n", bufcpy);
                  } break;
        case(4) : { 
                      char arrbuf[1024] = "apple";
                      char *ptrbuf = "apple";

                      printf("s1, arr) %s\ns2, prt) %s\n", arrbuf, ptrbuf);

                      arrbuf[2] = 'x';
                      // ptrbuf[2] = 'x';
                      // can not change the pointer string since it is a segfault, read-only-mem

                      print("after change: ");
                      printf("s1, arr) %s\ns2, prt) %s\n", arrbuf, ptrbuf);

                  } break;
        case(5) : {
                      char file_name[1024];
                      if(!read_str("input the filename: ", ARR_L(file_name)))
                          return 1;
                      FILE *file = fopen(file_name, "r");
                      if (!file) {
                          fprintf(stderr, "could not open file %s\n", file_name);
                          return 1;
                      }
                      char buf[1024];
                      if (!fgets(ARR_L(buf), file)) {
                          err("could not read from file");
                          fclose(file);
                          return 1;
                      }
                      fclose(file);
                      int len = strlen(buf);
                      if (len <= 0) {
                          err("no data in file");
                          return 1;
                      }
                      printf("file data : \n%s\n", buf);

                      for(int i = 0; i<len; i++)
                          buf[i] = toupper(buf[i]);

                      printf("file data uppercase : \n%s\n", buf);
                  } break;
        case(6) : {
                      char file_name[1024];
                      if(!read_str("input the filename: ", ARR_L(file_name)))
                          return 1;
                      FILE *file = fopen(file_name, "r");
                      if (!file) {
                          err("could not open file");
                          return 1;
                      }
                      char buf[1024];
                      if (!fgets(ARR_L(buf), file)) {
                          err("could not read from file");
                          fclose(file);
                          return 1;
                      }
                      fclose(file); file = NULL;
                      int len = strlen(buf);
                      if (len <= 0) {
                          err("no data in file");
                          return 1;
                      }
                      // printf("file data : \n%s\n", buf);

                      for(int i = 0; i<len; i++)
                          buf[i] = toupper(buf[i]);

                      // printf("file data uppercase : \n%s\n", buf);
                      if(!read_str("input destination file: ", ARR_L(file_name)))
                          return 1;
                      if (check_file_exist(file_name)) {
                          err("file already exists");
                          return 1;
                      }
                      file = fopen(file_name, "w");
                      if (!file) {
                          err("could not open file");
                          return 1;
                      }
                      int ch_printed = 0;
                      int pr = 0;
                      while (1) { // used like a label + goto
                          pr = fprintf(file, "%s", buf + ch_printed);
                          if (pr > 0 && ch_printed < len) {
                              ch_printed += pr;
                              continue;
                          }
                          else if (ch_printed == len)
                              print("wrote everything sucessfully");
                          else if (pr <= 0)
                              err("error, probably ...");

                          break;
                      }

                      fclose(file);
                  } break;

        default : {
                      print("not done yet / NAN");
                  }
                  break;
    }


    return 0;
}

