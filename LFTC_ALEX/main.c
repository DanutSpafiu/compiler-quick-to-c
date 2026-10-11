#include <stdio.h>

int main() 
{
  char *inbuf = loadFile("test/1.q");
  tokenize(inbuf);
  showTokens();

  return 0;
}