#include <stdio.h>
#include <stdlib.h>

#include "display_cow.h"

#define MAX_LINE 1024   // Max chars to read in one go

static void read_from_stdin(void)
{
   /*char line[MAX_LINE];
   while (EOF != fgets(line, MAX_LINE, stdin)) {
      display_cow(line);
      }*/
}

int main(int argc, char **argv)
{
   gtk_init(&argc, &argv);

   cowsay_init();
   
   if (argc == 1) {
      read_from_stdin();
   }
   else if (argc == 2) {
      display_cow(argv[1]);
   }
   else {
      fprintf(stderr, "Error: Too many arguments\n");
   }
   
   return EXIT_SUCCESS;
}
