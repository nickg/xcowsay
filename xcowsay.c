#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>

#include "display_cow.h"
#include "settings.h"


// Default settings
#define DEF_LEAD_IN_TIME  250
#define DEF_DISPLAY_TIME  4000
#define DEF_LEAD_OUT_TIME LEAD_IN_TIME
#define DEF_FONT          "Bitstream Vera Sans 14" 

#define MAX_STDIN 4096   // Maximum chars to read from stdin

static struct option long_options[] = {
   {"help", no_argument, 0, 'h'},
   {"time", required_argument, 0, 't'},
   {0, 0, 0, 0}
};

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

static void usage()
{
   static const char *usage_message =
      "Usage: xcowsay [OPTION] [MESSAGE]\n"
      "Display a cow on your desktop with [MESSAGE] or standard input.\n\n"
      "Options:\n"
      " -h, --help\t\tDisplay this message and exit.\n"
      " -t, --time SECONDS\tDisplay message for SECONDS seconds.\n";
   puts(usage_message);
}

int main(int argc, char **argv)
{   
   add_int_option("lead_in_time", DEF_LEAD_IN_TIME);
   add_int_option("display_time", DEF_DISPLAY_TIME);
   add_int_option("lead_out_time", get_int_option("lead_in_time"));
   add_string_option("font", DEF_FONT);

   cowsay_init(&argc, &argv);
   
   int c, option_index = 0, failure = 0;
   while ((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1) {
      switch (c) {
      case 'h':
         usage();
         exit(EXIT_SUCCESS);
      case 't':
         break;
      case '?':
         // getopt_long already printed an error message
         failure = 1;
         break;
      default:
         abort();
      }
   }
   if (failure)
      exit(EXIT_FAILURE);

   srand((unsigned)time(NULL));
   
   if (optind == argc) {
      read_from_stdin();
   }
   else if (optind == argc - 1) {
      display_cow(argv[optind]);
   }
   else {
      fprintf(stderr, "Error: Too many arguments\n");
   }
   
   return EXIT_SUCCESS;
}
