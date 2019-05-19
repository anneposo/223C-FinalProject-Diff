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

#include "diff.h"
#include "para.h"
#include "util.h"
#include "para.c"
#include "util.c"

const char* files[2] = { NULL, NULL };
char q_linebuf[LINEBUFLEN];
char p_linebuf[LINEBUFLEN];
char linematch[LINEBUFLEN];
char match_buf[BUFLEN];
char diff_buf[BUFLEN];
int foundline = 0;

void normal(para* p, para* q);

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
      //para_print(q, printboth);
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
  printf("\n\n\ndiff (CSUF diffutils) 1.0.0\n");
  printf("Copyright (C) 2014 CSUF\n");
  printf("This program comes with NO WARRANTY, to the extent permitted by law.\n");
  printf("You may redistribute copies of this program\n");
  printf("under the terms of the GNU General Public License.\n");
  printf("For more information about these matters, see the file named COPYING.\n");
  printf("Written by William McCarthy, Tony Stark, and Dr. Steven Strange\n");
}

void todo_list(void) {
  printf("\n\n\nTODO: check line by line in a paragraph, using '|' for differences");
  printf("\nTODO: this starter code does not yet handle printing all of fin1's paragraphs.");
  printf("\nTODO: handle the rest of diff's options\n");
  printf("\nTODO: diff -q aka --brief (reports only whether files are different. shows nothing if files are identical)\n");
  printf("TODO: diff -y aka --side-by-side (side by side format && prints common lines)\n");
  printf("TODO: diff -y --left-column (prints only left column of common lines)\n");
  printf("TODO: diff -y --suppress-common-lines (side-by-side but doesn't show common lines)\n");
  printf("TODO: diff -c NUM (shows NUM default 3 lines of copied context)\n");
  printf("TODO: diff -u NUM (shows NUM default 3 lines of unified context)\n");
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
  //const char* files[2] = { NULL, NULL };

  while (argc-- > 0) {
    const char* arg = *argv;
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
    ++argv;   // DEBUG only;  move increment up to top of switch at release
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

  //todo_list();
  return 0;
}
