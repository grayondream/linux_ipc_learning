#include <unistd.h>
#include <stdio.h>

#define MAX_LEN 1024

void err_exit(const char *buff, int err_ret);
#define ERROR_CHECK(ret, op, failed, val, desc)  if((ret) op (failed))\
                                              {\
                                              char (buff)[MAX_LEN] = {0};\
                                              snprintf((buff), MAX_LEN, (desc), (val));\
                                              err_exit((buff), (ret));\
                                              }
/*
 * @brief   针对api的包装函数  
 */
void safe_exit(const char *buff);
int lread(int readfd, char *buff, int len);
int lwrite(int writefd, char *buff, int len);
char* lfgets(char *buff, int len, FILE *stream);
int lopen(char *filename, int mode);
void lclose(int fd);
void lpipe(int *fd);
int lfork();
void lwaitpid(int pid, int *status, int option);
FILE* lpopen(char *command, char* type);
void lpclose(FILE *fd);
void lmkfifo(char *file, int mode);
void lunlink(char *file);