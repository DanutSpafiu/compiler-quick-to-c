#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "utils.h"
#include "../LFTC_ASIN/parser.h"

int main(int argc, char **argv)
{
  int show = argc > 1 && strcmp(argv[1], "--tokens") == 0;
  if(argc > (show ? 3 : 2)){
    fprintf(stderr, "Utilizare: %s [--tokens] [fisier.q]\n", argv[0]);
    return EXIT_FAILURE;
  }
  const char *fileName = argc > 1 + show ? argv[1 + show] : "test/1.q";
  char *inbuf = loadFile(fileName);
  tokenize(inbuf);
  free(inbuf);
  if(show)showTokens();
  parse();

  return 0;
}
