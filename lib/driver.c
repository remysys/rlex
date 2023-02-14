#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <compiler.h>

static FILE *Input_file = NULL;
static int Input_line;              /* line number of most-recently read line */
static char File_name[128];         /* template-file name */


FILE *driver_1(FILE *output, int lines, char *file_name)
{
  char install_path[128] = "/usr/local/lib64/";

  if (!(Input_file = fopen(file_name, "r"))) {
    file_name = strcat(install_path, file_name);

    if (!(Input_file = fopen(file_name, "r"))) {
      return NULL;
    }
  }

  strncpy(File_name, file_name, sizeof(File_name) );
  Input_line = 0;
  driver_2(output, lines);
  return Input_file;
}

int driver_2(FILE *output, int lines)
{
  static char buf[256];
  char *p;
  int processing_comment = 0;
  if (!Input_file) {
    ferr("internal error [driver_2], template file lex.par not open\n");
  }

  if (lines) {
    fprintf(output, "\n#line %d \"%s\"\n", Input_line + 1, File_name);
  }

  while(fgets(buf, sizeof(buf), Input_file)) {
    ++Input_line;
    if (*buf == '?') {
      break;
    }

    for (p = buf; isspace(*p); ++p) {
      ;
    }

    if (*p == '@') {
      processing_comment = 1;
      continue;
    } else if (processing_comment) {  /* previous line was a comment */
      /* but current line is not */
      processing_comment = 0;
      if (lines) {
        fprintf(output, "\n#line %d \"%s\"\n", Input_line, File_name);
      }
    }

    fputs(buf, output);
  }

  return feof(Input_file);
}