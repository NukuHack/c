#include <ctype.h> // char conversion
#include "my_utils.h" // better io (renamed from small_lib.h)
#include "my_io.h" // input

int main(int argc, char **argv) {
    if (argc == 1) {
        printf("Usage : %s <num>\n", argv[0]);
        return 1;
    }
    char *buf = NULL;
    FILE *file = NULL;
    goto body;
err:;;
    if (file) {
        fclose(file);
    }
    if (buf) {
        free(buf);
    }
    return 1;
body:;;
     switch(atoi(argv[1])) {
         case(1) : {
                       char buf[BUF_LEN];
                       if(!read_str("input a line: ", ARR_L(buf)))
                           return 1;

                       int wc = count_words(buf, " "); // uses string.h underneath
                       size_t cc = strlen(buf);
                       printf("word count: %d, char count: %ld \n", wc, cc);
                   } break;
         case (2) : {
                        char buf[BUF_LEN];
                        if(!read_str("input a line: ", ARR_L(buf)))
                            return 1;
                        char buf2[BUF_LEN];
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
                       char buf[BUF_LEN];
                       if(!read_str("a line to copy: ", ARR_L(buf)))
                           return 1;
                       char bufcpy[BUF_LEN];

                       size_t len = strlen(buf);
                       memcpy(bufcpy, buf, len + 1);

                       printf("copied string \n%s\n", bufcpy);
                   } break;
         case(4) : { 
                       char arrbuf[BUF_LEN] = "apple";
                       const char *ptrbuf = "apple";

                       printf("s1, arr) %s\ns2, prt) %s\n", arrbuf, ptrbuf);

                       arrbuf[2] = 'x';
                       // ptrbuf[2] = 'x';
                       // can not change the pointer string since it is a segfault, read-only-mem

                       print("after change: ");
                       printf("s1, arr) %s\ns2, prt) %s\n", arrbuf, ptrbuf);

                   } break;
         case(5) : {
                       char file_name[BUF_LEN];
                       if(!read_str("input the filename: ", ARR_L(file_name)))
                           return 1;
                       size_t n = BUF_LEN;
                       buf = (char*)malloc(n * sizeof(char));
                       if (!buf) {
                           fprintf(stderr, "could not malloc %lu bytes\n", n);
                           return 1;
                       }
                       file = fopen(file_name, "r");
                       if (!file) {
                           fprintf(stderr, "could not open file %s\n", file_name);
                           free(buf);
                           return 1;
                       }

                       ptrdiff_t red = fgetbatch(&buf, &n, file, false);
                       if (red < 0) {
                           err("could not read from file");
                           fclose(file);
                           free(buf);
                           return 1;
                       }
                       fclose(file); file = NULL;

                       if (n == 0) {
                           err("no data in file");
                           free(buf);
                           return 1;
                       }
                       printf("file data : \n%s\n", buf);

                       for(size_t i = 0; i<n; i++)
                           buf[i] = (char)toupper(buf[i]);

                       printf("file data uppercase : \n%s\n", buf);
                       free(buf);
                   } break;
         case(6) : {
                       char file_name[BUF_LEN];
                       if(!read_str("input the filename: ", ARR_L(file_name)))
                           goto err;
                       size_t n = BUF_LEN;
                       buf = (char*)malloc(n * sizeof(char));
                       if (!buf) {
                           fprintf(stderr, "could not malloc %lu bytes\n", n);
                           goto err;
                       }
                       file = fopen(file_name, "r");
                       if (!file) {
                           err("could not open file");
                           goto err;
                       }
                       ptrdiff_t red = fgetbatch(&buf, &n, file, false);
                       if (red < 0) {
                           err("could not read from file");
                           goto err;
                       }
                       fclose(file); file = NULL;

                       if (n == 0) {
                           err("no data in file");
                           goto err;
                       }
                       // printf("file data : \n%s\n", buf);

                       for(size_t i = 0; i<n; i++)
                           buf[i] = (char)toupper(buf[i]);

                       // printf("file data uppercase : \n%s\n", buf);
                       if(!read_str("input destination file: ", ARR_L(file_name)))
                           goto err;
                       if (check_file_exist(file_name)) {
                           err("file already exists");
                           goto err;
                       }
                       file = fopen(file_name, "w");
                       if (!file) {
                           err("could not open file");
                           goto err;
                       }

                       int pr = fputbatch(&buf, &n, file, false);
                       if (pr == n)
                           print("wrote everything sucessfully");
                       else if (pr <= 0)
                           err("error, probably ...");

                       fclose(file);
                       free(buf);
                   } break;

         default : {
                       print("not done yet / NAN");
                   }
                   break;
     }


    return 0;
}

