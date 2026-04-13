#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void compress(const char *fileName) {
  printf("compressing file\n");

  FILE *f = fopen(fileName, "rb");
  if (!f) {
    perror("fopen failed");
    exit(1);
  }

  fseek(f, 0, SEEK_END);
  long fileLength = ftell(f);
  rewind(f);

  if (fileLength <= 0) {
    printf("empty file\n");
    fclose(f);
    exit(1);
  }

  unsigned char *buf = malloc(fileLength);
  if (!buf) {
    printf("malloc failed\n");
    fclose(f);
    exit(1);
  }

  size_t read = fread(buf, 1, fileLength, f);
  fclose(f);

  if (read != fileLength) {
    printf("read error\n");
    free(buf);
    exit(1);
  }

  // safer output filename handling
  char outName[512];
  snprintf(outName, sizeof(outName), "%s.rle", fileName);

  FILE *outF = fopen(outName, "wb");
  if (!outF) {
    perror("out file failed");
    free(buf);
    exit(1);
  }

  unsigned char current = buf[0];
  int count = 1;

  for (long i = 1; i < fileLength; i++) {
    if (buf[i] == current && count < 255) {
      count++;
    } else {
      fputc((unsigned char)count, outF);
      fputc(current, outF);

      current = buf[i];
      count = 1;
    }
  }

  // flush last run
  fputc((unsigned char)count, outF);
  fputc(current, outF);

  fclose(outF);
  free(buf);

  printf("compressed to %s\n", outName);
}

void decompress(const char *fileName) {
  printf("decompressing file\n");

  FILE *f = fopen(fileName, "rb");
  if (!f) {
    perror("fopen failed");
    exit(1);
  }

  fseek(f, 0, SEEK_END);
  long fileLength = ftell(f);
  rewind(f);

  if (fileLength <= 0) {
    printf("empty file\n");
    fclose(f);
    exit(1);
  }

  unsigned char *buf = malloc(fileLength);
  if (!buf) {
    printf("malloc failed\n");
    fclose(f);
    exit(1);
  }

  size_t read = fread(buf, 1, fileLength, f);
  fclose(f);

  if (read != fileLength) {
    printf("read error\n");
    free(buf);
    exit(1);
  }

  char outName[512];
  snprintf(outName, sizeof(outName), "%s.decompressed", fileName);

  FILE *outF = fopen(outName, "wb");
  if (!outF) {
    perror("out file failed");
    free(buf);
    exit(1);
  }

  for (long i = 0; i < fileLength; i += 2) {
    unsigned char count = buf[i];
    unsigned char value = buf[i + 1];

    for (int j = 0; j < count; j++) {
      fputc(value, outF);
    }
  }

  fclose(outF);
  free(buf);

  printf("decompressed to %s\n", outName);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("too few arguments specified\n");

    return 1;
  }

  printf("%s, %s\n", argv[1], argv[2]);

  if (strcmp(argv[1], "c") == 0) {
    compress(argv[2]);
  } else if (strcmp(argv[1], "d") == 0) {
    decompress(argv[2]);
  } else {
    printf("invalid command\n");
    return 1;
  }

  return 0;
}
