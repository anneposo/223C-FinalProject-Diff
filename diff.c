#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h> //for getting a file's modified date

#include "diff.h"

char* yesorno(int condition) { return condition == 0 ? "no" : "YES"; }

FILE* openfile(const char* filename, const char* openflags) {
  FILE* f;
  if ((f = fopen(filename, openflags)) == NULL) {  printf("can't open '%s'\n", filename);  exit(1); }
  return f;
}

void printline(void) {
  for (int i = 0; i < 10; ++i) { printf("=========="); }
  printf("\n");
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

void printleft_nospace(const char* left) {
  char buf[BUFLEN];

  strcpy(buf, left);
  int j = 0, len = (int)strlen(buf) - 1;
  for (j = 0; j <= 48 - len ; ++j) { buf[len + j] = ' '; }
  //buf[len + j++] = '<';
  buf[len + j++] = '\0';
  printf("< %s\n", buf);
}

void printleft_leftcolumn(const char* left) {
  char buf[BUFLEN];

  strcpy(buf, left);
  int j = 0, len = (int)strlen(buf) - 1;
  for (j = 0; j <= 48 - len ; ++j) { buf[len + j] = ' '; }
  //buf[len + j++] = '<';
  buf[len + j++] = '(';
  buf[len + j++] = '\0';
  printf("%s\n", buf);
  //printf("%50s", "(",)

}

void print_conext_add(const char* right) {
  if (right == NULL) { return; }
  printf("+ %s", right);
}

void print_context_delete(const char* right) {
  if (right == NULL) { return; }
  printf("- %s", right);
}

void printright_blank(const char* right) {
  if (right == NULL) { return; }
  printf("  %s", right);
}

void printright(const char* right) {
  if (right == NULL) { return; }
  printf("%50s %s", ">", right);
}

void printright_nospace(const char* right) {
  if (right == NULL) { return; }
  printf("> %s", right);
}

void printboth(const char* left_right) {
  char buf[BUFLEN];
  size_t len = strlen(left_right);
  if (len > 0) { strncpy(buf, left_right, len); }
  buf[len - 1] = '\0';
  printf("%-50s %s", buf, left_right);
}

para* para_make(char* base[], int filesize, int start, int stop) {
  para* p = (para*) malloc(sizeof(para));
  p->base = base;
  p->filesize = filesize;
  p->start = start;
  p->stop = stop;
  p->firstline = (p == NULL || start < 0) ? NULL : p->base[start];
  p->secondline = (p == NULL || start < 0 || filesize < 2) ? NULL : p->base[start + 1];

  return p;
}

para* para_first(char* base[], int size) {
  para* p = para_make(base, size, 0, -1);
  return para_next(p);
}

void para_destroy(para* p) { free(p); }

para* para_next(para* p) {
  if (p == NULL || p->stop == p->filesize) { return NULL; }

  int i;
  para* pnew = para_make(p->base, p->filesize, p->stop + 1, p->stop + 1);
  for (i = pnew->start; i < p->filesize && strcmp(p->base[i], "\n") != 0; ++i) { }
  pnew->stop = i;

  if (pnew->start >= p->filesize) {
    free(pnew);
    pnew = NULL;
  }
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

void para_print(para* p, void (*fp)(const char*)) {
  if (p == NULL) { return; }
  for (int i = p->start; i <= p->stop && i != p->filesize; ++i) { fp(p->base[i]); }
}

void para_print_context(para* p, void (*fp)(const char*)) {
  if (p == NULL) { return; }
  for (int i = p->start; i < 3 && i != p->filesize; ++i) { fp(p->base[i]); }
}

void para_print_context_end(para* p, void (*fp)(const char*)) {
  if (p == NULL) { return; }
  int temp = p->stop - 2;
  for (int i = temp; i <= p->stop && i != p->filesize; ++i) { fp(p->base[i]); }
}

// void line_print(para* p, char* q) {
//   if (p == NULL) { return; }
//
//   char buf[BUFLEN];
//   size_t len = strlen(q);
//   if (len > 0) { strncpy(buf, q, len); }
//   buf[len - 1] = '\0';
//
//   //for (int i = p->start; i <= p->stop && i != p->filesize; ++i) {
//     printf("%-50s %s", buf, q);
//   //}
// }
//
// void line_diff_print(char* p, char* q) {
//   if (p == NULL) { return; }
//   char buf1[BUFLEN];
//   size_t len1 = strlen(p);
//   if (len1 > 0) { strncpy(buf1, p, len1); }
//   buf1[len1 - 1] = '\0';
//   printf("%-50s | %s", buf1, q);
//   //printf("%-50s | %s", buf2, q);
// }

void para_printfile(char* base[], int count, void (*fp)(const char*)) {
  para* p = para_first(base, count);
  while (p != NULL) {
    para_print(p, fp);
    p = para_next(p);
  }
  printline();
}

void get_modified_date(char* filepath, char* date_buf) {
  struct stat buf; //stat obtains info about the named file and writes it into buf
  stat(filepath, &buf);
  strftime(date_buf, BUFLEN, "%Y-%m-%d %H:%M:%S", localtime(&buf.st_mtime));
}

void get_filepaths(const char* filename1, const char* filename2) {
  filepath1 = realpath(filename1, pathbuf1); //realpath finds absolute path of input filename - for stat()
  filepath2 = realpath(filename2, pathbuf2);
}

void unified(para* p, para* q) {
  printf("***%s\t%s\n", files[0], mod_date1);
  printf("---%s\t%s\n", files[1], mod_date2);
  printf("***************\n");

  int foundmatch;
  para* qlast = q;
  while (p != NULL) {
    qlast = q;
    foundmatch = 0;
    while (q != NULL && (foundmatch = para_equal(p, q)) == 0) {
      q = para_next(q);
    }
    q = qlast;

    if (foundmatch) {
      while ((foundmatch = para_equal(p, q)) == 0) { // if para_equal == 0, there is no match
        printf("@@ %d,%d +%d,%d @@\n", q->start+1*-1, q->start+3, p->start+1, p->stop);
        para_print(q, print_conext_add);
        q = para_next(q);
        qlast = q;
      }
      para_print_context(p, printright_blank);
      printf("@@ -%d,%d +%d,%d @@\n", p->stop-1, p->filesize-11, p->stop+5, p->stop-1);
      para_print_context_end(p, printright_blank);
      p = para_next(p);
      q = para_next(q);
    } else {
      para_print(p, print_context_delete);
      p = para_next(p);
    }
  }
  while (q != NULL) {
    para_print(q, print_conext_add);
    q = para_next(q);
  }
}


void context(para* p, para* q) {
  printf("***%s\t%s\n", files[0], mod_date1);
  printf("---%s\t%s\n", files[1], mod_date2);
  printf("***************\n");

  int foundmatch;
  para* qlast = q;
  while (p != NULL) {
    qlast = q;
    foundmatch = 0;
    while (q != NULL && (foundmatch = para_equal(p, q)) == 0) {
      q = para_next(q);
    }
    q = qlast;

    if (foundmatch) {
      while ((foundmatch = para_equal(p, q)) == 0) { // if para_equal == 0, there is no match
        printf("*** %d,%d ****\n", q->start+1, q->start+3);
        printf("--- %d,%d ----\n", p->start+1, p->stop);
        para_print(q, print_conext_add);
        q = para_next(q);
        qlast = q;
      }
      para_print_context(p, printright_blank);
      printf("***************\n");
      printf("*** %d,%d ****\n", p->stop-1, p->filesize-4);
      para_print_context_end(p, printright_blank);
      p = para_next(p);
      q = para_next(q);
    } else {
      para_print(p, print_context_delete);
      p = para_next(p);
    }
  }
  while (q != NULL) {
    para_print(q, print_conext_add);
    q = para_next(q);
  }
}

void left_column(para* p, para* q) {
  int foundmatch;
  para* qlast = q;
  while (p != NULL) {
    qlast = q;
    foundmatch = 0;
    while (q != NULL && (foundmatch = para_equal(p, q)) == 0) {
      q = para_next(q);
    }
    q = qlast;

    if (foundmatch) {
      while ((foundmatch = para_equal(p, q)) == 0) { // if para_equal == 0, there is no match
        para_print(q, printright);
        q = para_next(q);
        qlast = q;
      }
      para_print(p, printleft_leftcolumn);
      p = para_next(p);
      q = para_next(q);
    } else {
      para_print(p, printleft);
      p = para_next(p);
    }
  }
  while (q != NULL) {
    para_print(q, printright);
    q = para_next(q);
  }
  exit(0);
}

int para_equal(para* p, para* q) {
  if (p == NULL || q == NULL) { return 0; }
  if (para_size(p) != para_size(q)) { return 0; }
  if (p->start >= p->filesize || q->start >= q->filesize) { return 0; }
  int equal = 0, different_lines = 0;
  for (int i = p->start, j = q->start; i <= p->stop && j <= q->stop && (equal = strcmp(p->base[i], q->base[j])) == 0; ++i, ++j) {
    different_lines += equal == 0 ? 0 : 1; //if different_lines is 0, the two paragraphs are equal
  }
  //while ((equal = strcmp(p->base[i], q->base[i])) == 0) { ++i; ++j; }
  return different_lines == 0;
}

int file_equal(para* p, para* q) { // returns 0 if files differ, 1 if files are identical
  int foundmatch;
  para* qlast = q;
  while (p != NULL) {
    qlast = q;
    foundmatch = 0;
    while (q != NULL && (foundmatch = para_equal(p, q)) == 0) {
      q = para_next(q); // if p and q are not equal, go to next paragraph in q file
    }
    q = qlast;
    if (foundmatch) { // if paragraph p and q are equal, go to next paragraph in p file.
      p = para_next(p);
    } else { return 0; } // else, return 0 because files differ
  }
  return 1; // if function exits above while loop, then the two files are identical
}

void brief(para* p, para* q) {
    if (file_equal(p, q) == 0) {
      printf("Files %s and %s differ\n", files[0], files[1]);
    }
    return;
}

void report_identical_files(para* p, para* q, int brief) {
  if (file_equal(p,q) == 1) {
    printf("Files %s and %s are identical\n", files[0], files[1]);
  } else {
    if(!brief) {
      normal(p, q);
    } else { return; }
  }
  return;
}

// void line_diff (para* p, para* q) { //for implementing | to show diff lines
//   int j = q->start, i = p->start, equal = 0, different_lines = 0;
//   int lastj = q->start, diff_index;
//
//   while (i <= p->stop) { // searches through the current paragraph in p
//     //while (q != NULL) { //searches through all the remaining paragraphs in file q
//     while(j <= q->stop && (equal = strcmp(p->base[i], q->base[j])) == 0) {
//       //for (j = q->start; j <= q->stop && (equal = strcmp(p->base[i], q->base[j])) == 0; ++j) {
//         different_lines += equal == 0 ? 0 : 1; //record if the lines are equal in different_lines
//         //printf ("%s %s", p->base[i], q->base[j]);
//         line_print(q, q->base[j]);
//         //strcat(match_buf, p->base[i]); //strcat matching line to buf1
//         ++i;
//         ++j;
//     }
//       if (different_lines) { //if lines are different
//         line_diff_print(p->base[i], q->base[j]);
//         //printf ("%s| %s", p->base[i], q->base[j]);
//         strcat(diff_buf, p->base[i]); //strcat diff line to buf2
//         diff_index = i; //record index of where diff line was found in file p
//         //printboth_line_diff(p->base[i], q->base[j]);
//         //lastj = j;
//         //++j;
//       } else { line_print(q, q->base[j]); }
//       //q = para_next(q); //check next paragraph in file q for matching/diff lines
//     //}
//     ++j;
//     ++i;
//   }
//   return;
// }

void normal(para* p, para* q) {
  int foundmatch;
  para* qlast = q;
  while (p != NULL) {
    qlast = q;
    foundmatch = 0;
    while (q != NULL && (foundmatch = para_equal(p, q)) == 0) {
      q = para_next(q);
    }
    q = qlast;

    if (foundmatch) {
      while ((foundmatch = para_equal(p, q)) == 0) { // if para_equal == 0, there is no match
        para_print(q, printright_nospace);
        //printf("\n");
        q = para_next(q);
        qlast = q;
      }
      p = para_next(p);
      q = para_next(q);
    } else {
      para_print(p, printleft_nospace);
      //printf("\n");
      p = para_next(p);
    }
  }
  while (q != NULL) {
    para_print(q, printright_nospace);
    //printf("\n");
    q = para_next(q);
  }

}

void side_by_side(para* p, para* q, int suppresscommon){
  int foundmatch;
  para* qlast = q;
  while (p != NULL) {
    qlast = q;
    foundmatch = 0;
    while (q != NULL && (foundmatch = para_equal(p, q)) == 0) {
      q = para_next(q);
    }
    q = qlast;

    if (foundmatch) {
      while ((foundmatch = para_equal(p, q)) == 0) { // if para_equal == 0, there is no match
        para_print(q, printright);
        q = para_next(q);
        qlast = q;
      }
      if (!suppresscommon) {
        para_print(q, printboth);
        //line_diff(p,q);
      }
      p = para_next(p);
      q = para_next(q);
    } else { // if no match is found
      //call line_diff that searches for different lines and also calls a modified para_print(both)
      //line_diff(p, q);
      para_print(p, printleft);
      p = para_next(p);
    }
  }
  while (q != NULL) {
    para_print(q, printright);
    q = para_next(q);
  }
}

void version(void) {
  printf("diff (CSUF diffutils) 1.0.0\n");
  printf("Copyright (C) 2014 CSUF\n");
  printf("This program comes with NO WARRANTY, to the extent permitted by law.\n");
  printf("You may redistribute copies of this program\n");
  printf("under the terms of the GNU General Public License.\n");
  printf("For more information about these matters, see the file named COPYING.\n");
  printf("\nWritten by Anne Poso.\n");
}


void loadfiles(const char* filename1, const char* filename2) {
  memset(buf, 0, sizeof(buf));
  memset(strings1, 0, sizeof(strings1));
  memset(strings2, 0, sizeof(strings2));

  FILE *fin1 = openfile(filename1, "r");
  FILE *fin2 = openfile(filename2, "r");

  while (!feof(fin1) && fgets(buf, BUFLEN, fin1) != NULL) { strings1[count1++] = strdup(buf); }  fclose(fin1);
  while (!feof(fin2) && fgets(buf, BUFLEN, fin2) != NULL) { strings2[count2++] = strdup(buf); }  fclose(fin2);
}

void print_option(const char* name, int value) { printf("%17s: %s\n", name, yesorno(value)); }

void diff_output_conflict_error(void) {
  fprintf(stderr, "diff: conflicting output style options\n");
  fprintf(stderr, "diff: Try `diff --help' for more information.)\n");
  exit(CONFLICTING_OUTPUT_OPTIONS);
}

void setoption(const char* arg, const char* s, const char* t, int* value) {
  if ((strcmp(arg, s) == 0) || ((t != NULL && strcmp(arg, t) == 0))) {
    *value = 1;
  }
}

void showoptions(const char* file1, const char* file2) {
  printf("diff options...\n");
  print_option("diffnormal", diffnormal);
  print_option("show_version", showversion);
  print_option("show_brief", showbrief);
  print_option("ignorecase", ignorecase);
  print_option("report_identical", report_identical);
  print_option("show_sidebyside", showsidebyside);
  print_option("show_leftcolumn", showleftcolumn);
  print_option("suppresscommon", suppresscommon);
  print_option("showcontext", showcontext);
  print_option("show_unified", showunified);

  printf("file1: %s,  file2: %s\n\n\n", file1, file2);

  printline();
}


void init_options_files(int argc, const char* argv[]) {
  int cnt = 0;

  while (argc-- > 0) {
    const char* arg = *argv;
    ++argv;
    setoption(arg, "-v",       "--version",                  &showversion);
    setoption(arg, "-q",       "--brief",                    &showbrief);
    setoption(arg, "-i",       "--ignore-case",              &ignorecase);
    setoption(arg, "-s",       "--report-identical-files",   &report_identical);
    setoption(arg, "--normal", NULL,                         &diffnormal);
    setoption(arg, "-y",       "--side-by-side",             &showsidebyside);
    setoption(arg, "--left-column", NULL,                    &showleftcolumn);
    setoption(arg, "--suppress-common-lines", NULL,          &suppresscommon);
    setoption(arg, "-c",       "--context",                  &showcontext);
    setoption(arg, "-u",       "showunified",                &showunified);
    if (arg[0] != '-') {
      if (cnt == 2) {
        fprintf(stderr, "apologies, this version of diff only handles two files\n");
        fprintf(stderr, "Usage: ./diff [options] file1 file2\n");
        exit(TOOMANYFILES_ERROR);
      } else { files[cnt++] = arg; }
    }
  }

  if (!showcontext && !showunified && !showsidebyside && !showleftcolumn) {
    diffnormal = 1;
  }

  if (showversion) { version();  exit(0); }

  if (((showsidebyside || showleftcolumn) && (diffnormal || showcontext || showunified)) ||
      (showcontext && showunified) || (diffnormal && (showcontext || showunified))) {

    diff_output_conflict_error();
  }

  //showoptions(files[0], files[1]);
  loadfiles(files[0], files[1]);
  get_filepaths(files[0], files[1]); //gets absolute filepaths for file1 and file2 and stores it in pathbuf1, pathbuf2
  get_modified_date(pathbuf1, mod_date1); //gets modified date of file 1 and stores it in mod_date1
  get_modified_date(pathbuf2, mod_date2); //gets modified date of file 2 and stores it in mod_date2
}


int main(int argc, const char * argv[]) {
  init_options_files(--argc, ++argv); //records input command line arguments

//  para_printfile(strings1, count1, printleft);
//  para_printfile(strings2, count2, printright);

  para* p = para_first(strings1, count1);
  para* q = para_first(strings2, count2);

  if(showsidebyside && showleftcolumn) { left_column(p, q); }
  if(showsidebyside)                   { side_by_side(p, q, suppresscommon); }
  if(report_identical && showbrief)    { brief(p, q); report_identical_files(p, q, showbrief); exit(0); }
  if(showbrief)                        { brief(p, q); exit(0); }
  if(report_identical)                 { report_identical_files(p, q, showbrief); exit(0); }
  if(diffnormal)                       { normal(p, q); }
  if(showcontext)                      { context(p, q); exit(0); }
  if(showunified)                     { unified(p, q); exit(0); }


  return 0;
}
