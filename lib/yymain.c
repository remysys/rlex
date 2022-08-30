/* a default main module to test the lexical analyzer */

int main(int argc, char *argv[])
{
  if (argc == 2) {
    ii_newfile(argv[1]);
  }

  while(yylex());
  
  return 0;
}