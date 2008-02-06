#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "display_cow.h"
#include "settings.h"


// Default settings
#define DEF_LEAD_IN_TIME   250
#define DEF_DISPLAY_TIME   4000
#define DEF_LEAD_OUT_TIME  LEAD_IN_TIME

#define MAX_STDIN 4096   // Maximum chars to read from stdin

static void read_from_stdin(void)
{
   char *data = malloc(MAX_STDIN);
   size_t n = fread(data, 1, MAX_STDIN, stdin);
   if (n == MAX_STDIN) {
      fprintf(stderr, "Warning: Excess input truncated\n");
      n--;
   }
   data[n] = '\0';

   display_cow(data);
   free(data);
}

int main(int argc, char **argv)
{
   add_int_option("lead_in_time", DEF_LEAD_IN_TIME);
   add_int_option("display_time", DEF_DISPLAY_TIME);
   add_int_option("lead_out_time", get_int_option("lead_in_time"));

   srand((unsigned)time(NULL));
   
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
