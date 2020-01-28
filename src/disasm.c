#include <stdio.h>

int main(int argc, char **argv) {
  printf("[REGULAR_ad] disassembler\n");
  if (argc < 2) {
      printf("usage: disasm --opts <in> <out>");
  }

  return 0;
}
