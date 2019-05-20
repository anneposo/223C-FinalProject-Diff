#ifndef diff_h
#define diff_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARGC_ERROR 1
#define TOOMANYFILES_ERROR 2
#define CONFLICTING_OUTPUT_OPTIONS 3
#define MAXSTRINGS 1024
#define MAXPARAS 4096
#define HASHLEN 200
#define LINEBUFLEN 100
#define BUFLEN 256
#define HASHLEN 200
#define LINEBUFLEN 100

const char* files[2] = { NULL, NULL }; //holds filenames of file 1 and file 2
char buf[BUFLEN];
char *strings1[MAXSTRINGS], *strings2[MAXSTRINGS];
int showversion = 0, showbrief = 0, ignorecase = 0, report_identical = 0, showsidebyside = 0;
int showleftcolumn = 0, showunified = 0, showcontext = 0, suppresscommon = 0, diffnormal = 0;
int count1 = 0, count2 = 0;

char match_buf[BUFLEN];
char diff_buf[BUFLEN];
char *filepath1;
char *filepath2;
char pathbuf1[BUFLEN]; //stores absolute file path for files[0] (1st file)
char pathbuf2[BUFLEN]; //stores absolute file path for files[1] (2nd file)
char mod_date1[BUFLEN]; //stores modified date of file 1
char mod_date2[BUFLEN]; //stores modified date of file 2

typedef struct para para;
struct para {
  char** base;
  int filesize;
  int start;
  int stop;
  char* firstline;   // DEBUG only
  char* secondline;
};

char* yesorno(int condition);
FILE* openfile(const char* filename, const char* openflags);

void printleft(const char* left);
void printleft_nospace(const char* left);
void printright(const char* right);
void printright_nospace(const char* right);
void printboth(const char* left_right);

void printline(void);

para* para_make(char* base[], int size, int start, int stop);
para* para_first(char* base[], int size);
para* para_next(para* p);
size_t para_filesize(para* p);
size_t para_size(para* p);
char** para_base(para* p);
char* para_info(para* p);
int   para_equal(para* p, para* q);
void para_print(para* p, void (*fp)(const char*));
void para_printfile(char* base[], int count, void (*fp)(const char*));

void normal(para* p, para* q);

#endif
