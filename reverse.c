#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

static int seek_prev_line(FILE *fp) {
  if (!fp)
    return -1;

  long pos = ftell(fp);
  if (pos <= 0)
    return 0;

  if (fseek(fp, pos - 1, SEEK_SET) != 0)
    return -1;
  int c = fgetc(fp);
  if (c == '\n') {
    pos -= 2;
  } else {
    pos -= 1;
  }

  while (pos >= 0) {
    if (fseek(fp, pos, SEEK_SET) != 0)
      return -1;
    if (fgetc(fp) == '\n') {
      return fseek(fp, pos + 1, SEEK_SET);
    }
    if (pos == 0)
      return fseek(fp, 0, SEEK_SET);
    pos--;
  }

  return fseek(fp, 0, SEEK_SET);
}

static int same_file_path(const char *a, const char *b) {
  struct stat sa, sb;
  if (stat(a, &sa) != 0)
    return 0;
  if (stat(b, &sb) != 0)
    return 0;
  return sa.st_dev == sb.st_dev && sa.st_ino == sb.st_ino;
}

int main(int argc, char *argv[]) {
  FILE *in_file_ptr = NULL;
  FILE *out_file_ptr = stdout;

  // no args specificed
  if (argc < 2 || argc > 3) {
    printf("usage: reverse <input> <output>\n");
    return 1;
  }

  // check if files are the same
  if (same_file_path(argv[1], argv[2])) {
    fprintf(stderr, "error: input and output file must differ\n");
    return 1;
  }

  // open file to read
  in_file_ptr = fopen(argv[1], "rb");
  if (in_file_ptr == NULL) {
    fprintf(stderr, "error: cannot open file '%s'\n", argv[1]);
    return 1;
  }

  // move to end of file
  fseek(in_file_ptr, 0L, SEEK_END);
  // file_size = ftell(in_file_ptr);
  // move back one more byte so we don't re-read the last \n
  fseek(in_file_ptr, -1, SEEK_CUR);

  while (seek_prev_line(in_file_ptr) == 0) {
    char *line = NULL;
    size_t cap = 0;

    // getline advances the file pointer so store it
    long prev_pos = ftell(in_file_ptr);
    if (getline(&line, &cap, in_file_ptr) != -1) {
      fprintf(out_file_ptr, "%s", line);
    }

    // rewind so we dont read the same line forever
    fseek(in_file_ptr, prev_pos, SEEK_SET);

    free(line);
    if (ftell(in_file_ptr) == 0) {
      break;
    }
  }

  fclose(in_file_ptr);

  return 0;
}
