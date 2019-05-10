//
//  main.c
//  diff
//
//  Created by William McCarthy on 4/29/19.
//  Copyright © 2019 William McCarthy. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "diff.h"

typedef struct para para;
struct para {
  char** base;
  int filesize;
  int start;
  int stop;
};

para* para_make(char* base[], int size, int start, int stop);
para* para_first(char* base[], int size);
para* para_next(para* p);
size_t para_filesize(para* p);
size_t para_size(para* p);
char** para_base(para* p);
char* para_info(para* p);
int   para_equal(para* p, para* q);
void para_print(para* p, void (*fp)(const char*));

para* para_make(char* base[], int filesize, int start, int stop) {
  para* p = (para*) malloc(sizeof(para));
  p->base = base;
  p->filesize = filesize;
  p->start = start;
  p->stop = stop;
//  p->firstline = (p == NULL || start < 0) ? NULL : p->base[start];
//  p->secondline = (p == NULL || start < 0 || filesize < 2) ? NULL : p->base[start+1];

  return p;
}

para* para_first(char* base[], int size) {
  para* p = para_make(base, size, 0, -1);
  return para_next(p);
}

void para_destroy(para* p) { free(p); }

para* para_next(para* p) {
  if (p->stop == p->filesize) { return NULL; }

  int i;
  para* pnew = para_make(p->base, p->filesize, p->stop + 1, p->stop + 1);
  for (i = pnew->start; i < p->filesize && strcmp(p->base[i], "\n") != 0; ++i) { }
  pnew->stop = i;

  return pnew;
}
size_t para_filesize(para* p) { return p == NULL ? 0 : p->filesize; }

size_t para_size(para* p) { return p == NULL || p->stop < p->start ? 0 : p->stop - p->start + 1; }

char** para_base(para* p) { return p->base; }

char* para_info(para* p) {
  static char buf[BUFLEN];   // static for a reason
  snprintf(buf, sizeof(buf), "base: %p, filesize: %d, start: %d, stop: %d\n",
                  p->base, p->filesize, p->start, p->stop);
  return buf;  // buf MUST be static
}

int para_equal(para* p, para* q) {
  if (p == NULL || q == NULL) { return 0; }
  if (para_size(p) != para_size(q)) { return 0; }
  int i = p->start, j = q->start, equal = 0;
  while ((equal = strcmp(p->base[i], q->base[i])) == 0) { ++i; ++j; }
  return equal;
}

void para_print(para* p, void (*fp)(const char*)) {
  if (p == NULL) { return; }
  for (int i = p->start; i <= p->stop && i != p->filesize; ++i) { fp(p->base[i]); }
}

char* yesorno(int condition) { return condition == 0 ? "no" : "YES"; }

FILE* openfile(const char* filename, const char* openflags) {
  FILE* f;
  if ((f = fopen(filename, openflags)) == NULL) {  printf("can't open '%s'\n", filename);  exit(1); }
  return f;
}

void printleft(const char* left) {
  char buf[BUFLEN];

  strcpy(buf, left);
  int j = 0, len = (int)strlen(buf) - 1;
  for (j = 0; j <= 48 - len ; ++j) { buf[len + j] = ' '; }
  buf[len + j++] = '<';
  buf[len + j++] = '\0';
  printf("%s\n", buf);
}

void printright(const char* right) {
  if (right == NULL) { return; }
  printf("%50s %s", ">", right);
}

void printboth(const char* left_right) {
  char buf[BUFLEN];
  size_t len = strlen(left_right);
  if (len > 0) { strncpy(buf, left_right, len); }
  buf[len - 1] = '\0';
  printf("%-50s %s", buf, left_right); }

void setoption(const char* argv, const char* s, const char* t, int* value) {
  if(strcmp(argv, s) == 0 || (t != NULL && strcmp(argv, t) == 0)) {
    *value = 1;
  }
}

void init_options_files(int argc, const char* argv[]) {
  int cnt = 0;
  const char* files[2] = {NULL, NULL};

  while (argc-- > 0){
    const char* arg = *argv;
    setoption(arg, "-v", "--version",                &showversion);
    setoption(arg, "-q", "--brief",                  &showbrief);
    setoption(arg, "-i", "--ignore-case",            &ignorecase);
    setoption(arg, "-s", "--report-identical-files", &reportidentical);
    setoption(arg, "-c", "--context",                &showcontext);
    setoption(arg, "-u", "--unified",                &showunified);
    setoption(arg, "--normal", NULL,                 &diffnormal);
    setoption(arg, "-y", "--side-by-side",           &show_sidebyside);
    setoption(arg, "--left-column", NULL,            &show_leftcolumn);
    setoption(arg, "--suppress-common-lines", NULL,  &suppresscommon);

    if(argv[0] != '-') {
      if(cnt == 2) {
        fprintf(stderr, "this version of diff only handles two files\n");
        fprintf(stderr, "Usage: ./diff [options] file1 file2\n");
      } else { files[cnt++] = arg; }
      //files[cnt++] = arg;
    }
    ++argv;
  }

  if(!showcontext && !)
}

int main(int argc, const char * argv[]) {

  init_options_files(--argc, ++argv); // skips over name of program


  char buf[BUFLEN];
  char *strings1[MAXSTRINGS], *strings2[MAXSTRINGS];

  memset(buf, 0, sizeof(buf));
  memset(strings1, 0, sizeof(strings1));
  memset(strings2, 0, sizeof(strings2));

  if (argc < 3) { fprintf(stderr, "Usage: ./diff [options] file1 file2\n");  exit(ARGC_ERROR); }
  else if(argc == 3) {
    FILE *fin1 = openfile(argv[1], "r");
    FILE *fin2 = openfile(argv[2], "r");

    int count1 = 0, count2 = 0;
    while (!feof(fin1) && fgets(buf, BUFLEN, fin1) != NULL) { strings1[count1++] = strdup(buf); }   fclose(fin1);
    while (!feof(fin2) && fgets(buf, BUFLEN, fin2) != NULL) { strings2[count2++] = strdup(buf); }   fclose(fin2);

    para* p = para_first(strings1, count1);
    para* q = para_first(strings2, count2);
//  int equal = 0;

  // while (p != NULL) {
  //   while (q != NULL && !para_equal(p,q)) {
  //     para_print(q, printright);
  //     q = para_next(q);
  //   }
  //   if (q != NULL && para_equal(p,q)) { para_print(p, printboth); }
  //   else { para_print(p, printleft); }
  //   p = para_next(p);
  // }

    if (para_equal(p, q)) { para_print(p, printboth);
    }

    para_print(q, printright);

    while ((q = para_next(q)) != NULL) {
      int equal = para_equal(p, q);
      para_print(q, equal ? printboth : printright);
    }

    while((p = para_next(p)) != NULL) {
      para_print(p, printleft);
    }
  }

  else if(argc >= 4 && argc < 6) {
    // int diffnormal = 0, show_version = 0, show_brief = 0, ignorecase = 0, report_identical = 0, show_sidebyside = 0,
    // show_leftcolumn = 0, suppresscommon = 0, showcontext = 0, show_unified = 0;

    for(int i = 1; i < argc; i++){
      if ((strncmp(argv[i], "-v", sizeof(argv[i]))) == 0) { show_version = 1; }
      if ((strncmp(argv[i], "-q", sizeof(argv[i]))) == 0) { show_brief = 1; }
      if ((strncmp(argv[i], "-i", sizeof(argv[i]))) == 0) { ignorecase = 1; }
      if ((strncmp(argv[i], "-s", sizeof(argv[i]))) == 0) { report_identical = 1; }
      if ((strncmp(argv[i], "-y", sizeof(argv[i]))) == 0) { show_sidebyside = 1; }
      if ((strncmp(argv[i], "-y", sizeof(argv[i]))) == 0 && (strncmp(argv[++i], "--left-column", sizeof(argv[++i])) == 0)) { show_leftcolumn = 1; }
      if ((strncmp(argv[i], "-y", sizeof(argv[i]))) == 0 && (strncmp(argv[++i], "--suppress-common-lines", sizeof(argv[++i])) == 0)) { suppresscommon = 1; }
      if ((strncmp(argv[i], "-c", sizeof(argv[i]))) == 0) { showcontext = 1; }
      if ((strncmp(argv[i], "-u", sizeof(argv[i]))) == 0) { show_unified = 1; }
    }

    // if ((strncmp(argv[1], "-v", sizeof(argv[1]))) == 0) { printf("version flag\n"); }
    // if ((strncmp(argv[1], "-q", sizeof(argv[1]))) == 0) { printf("reports only whether files are different\n"); }
    // if ((strncmp(argv[1], "-i", sizeof(argv[1]))) == 0) { printf("ignores upper/lower case differences\n"); }
    // if ((strncmp(argv[1], "-s", sizeof(argv[1]))) == 0) { printf("reports if files are the same\n"); }
    // if ((strncmp(argv[1], "-y", sizeof(argv[1]))) == 0) { printf("side by side format & prints common lines\n"); }
    // if ((strncmp(argv[1], "-c", sizeof(argv[1]))) == 0) { printf("shows NUM default 3 lines of copied text\n"); }
    // if ((strncmp(argv[1], "-u", sizeof(argv[1]))) == 0) { printf("shows NUM default 3 lines of unified text\n"); }
  }

  else { fprintf(stderr, "Usage: ./diff [options] file1 file2\n");  exit(ARGC_ERROR); }

//  printf("p is: %s", para_info(p));
//  printf("q is: %s", para_info(q));
//  para_print(p, printleft);
//  para_print(q, printright);
//  printf("p and q are equal: %s\n\n", yesorno(para_equal(p, q)));

  //if (showversion) { showversion(); }

  printf("\nTODO: check line by line in a paragraph, using '|' for differences");
  printf("\nTODO: this starter code does not yet handle printing all of fin1's paragraphs.");
  printf("\nTODO: handle the rest of diff's options\n");
  printf("\tAs Tolkien said it, '...and miles to go before I sleep.'\n\n");
  return 0;
}
