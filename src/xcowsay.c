/*  xcowsay.c -- Cute talking cow for GTK+.
 *  Copyright (C) 2008  Nick Gasson
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>
#include <assert.h>
#include <string.h>

#include "display_cow.h"
#include "settings.h"
#include "xcowsayd.h"
#include "config_file.h"
#include "i18n.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// Default settings
#define DEF_LEAD_IN_TIME  250
#define DEF_DISPLAY_TIME  CALCULATE_DISPLAY_TIME
#define DEF_LEAD_OUT_TIME LEAD_IN_TIME
#define DEF_MIN_TIME      2000
#define DEF_MAX_TIME      30000
#define DEF_FONT          "Bitstream Vera Sans 14"
#define DEF_READING_SPEED 250   // Human average is apparently 200-250 WPM (=5 WPS)
#define DEF_COW_SIZE      "med"
#define DEF_IMAGE_BASE    "cow"

#define MAX_STDIN 4096   // Maximum chars to read from stdin

static int daemon_flag = 0;
static int debug = 0;

static struct option long_options[] = {
   {"help", no_argument, 0, 'h'},
   {"version", no_argument, 0, 'v'},
   {"time", required_argument, 0, 't'},
   {"font", required_argument, 0, 'f'},
   {"dream", required_argument, 0, 'd'},
   {"cow-size", required_argument, 0, 'c'},
   {"reading-speed", required_argument, 0, 'r'},
   {"daemon", no_argument, &daemon_flag, 1},
   {"debug", no_argument, &debug, 1},
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

   display_cow_or_invoke_daemon(debug, data);
   free(data);
}

static void usage()
{
   printf
      ("%s: xcowsay [OPTION]... [MESSAGE]...\n"
       "%s\n\n"
       "%s:\n"
       " -h, --help\t\t%s\n"
       " -v, --version\t\t%s\n"
       " -t, --time=SECONDS\t%s\n"
       " -r, --reading-speed=N\t%s\n"
       " -f, --font=FONT\t%s\n"
       " -d, --dream=FILE\t%s\n"
       "     --daemon\t\t%s\n"
       "     --cow-size=SIZE\t%s\n"
       "     --debug\t\t%s\n\n"
       "%s\n\n"
       "%s\n\n"
       "%s\n",
       i18n("Usage"),
       i18n("Display a cow on your desktop with MESSAGE or standard input."),
       i18n("Options"),
       i18n("Display this message and exit."),
       i18n("Print version information."),
       i18n("Number of seconds to display message for"),
       i18n("Number of milliseconds to delay per word."),
       i18n("Set message font (Pango format)."),
       i18n("Display an image instead of text."),
       i18n("Run xcowsay in daemon mode."),
       i18n("Size of the cow (small, med, large)."),
       i18n("Keep daemon attached to terminal."),
       i18n("Default values for these options can be specified in the "
            "xcowsay config\nfile. See the man page for more information."),
       i18n("If the display_time option is not set the display time will "
            "be calcuated\nfrom the reading_speed parameter multiplied by "
            "the word count."),
       i18n("Report bugs to nick@cakesniffer.co.uk")
       );
}

static void version()
{
   static const char *copy =
      "Copyright (C) 2008 Nick Gasson\n"
      "This program comes with ABSOLUTELY NO WARRANTY. This is free software, and\n"
      "you are welcome to redistribute it under certain conditions. See the GNU\n"
      "General Public Licence for details.";
   
#ifdef HAVE_CONFIG_H
   puts(PACKAGE_STRING);
#endif

   puts(copy);   
}

static int parse_int_option(const char *optarg)
{
   char *endptr;
   int r = strtol(optarg, &endptr, 10);
   if ('\0' == *endptr)
      return r;
   else {
      fprintf(stderr, i18n("Error: %s is not a valid integer\n"), optarg);
      exit(EXIT_FAILURE);
   }
}

/*
 * Join all the strings in argv from ind to argc-1 into one big
 * string with spaces between the words.
 */
static char *cat_from_index(int ind, int argc, char **argv)
{
   size_t len = 0, i;
   for (i = ind; i < argc; i++)
      len += strlen(argv[i]) + (i < argc - 1 ? 1 : 0);

   char *buf = malloc(len+1);
   assert(buf);

   char *p = buf;
   for (i = ind; i < argc; i++) {
      strcpy(p, argv[i]);
      p += strlen(argv[i]);
      if (i < argc - 1)  // No space at the end
         *p++ = ' ';
   }

   return buf;
}

int main(int argc, char **argv)
{
   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALEDIR);
   textdomain(PACKAGE);
   
   add_int_option("lead_in_time", DEF_LEAD_IN_TIME);
   add_int_option("display_time", DEF_DISPLAY_TIME);
   add_int_option("lead_out_time", get_int_option("lead_in_time"));
   add_int_option("min_display_time", DEF_MIN_TIME);
   add_int_option("max_display_time", DEF_MAX_TIME);
   add_int_option("reading_speed", DEF_READING_SPEED);
   add_string_option("font", DEF_FONT);
   add_string_option("cow_size", DEF_COW_SIZE);
   add_string_option("image_base", DEF_IMAGE_BASE);
   
   parse_config_file();
   
   int c, index = 0, failure = 0;
   const char *spec = "hvd:rt:f:";
   const char *dream_file = NULL;
   while ((c = getopt_long(argc, argv, spec, long_options, &index)) != -1) {
      switch (c) {
      case 0:
         // Set a flag
         break;
      case 'd':
         dream_file = optarg;
         break;
      case 'c':
         set_string_option("cow_size", optarg);
         break;
      case 'h':
         usage();
         exit(EXIT_SUCCESS);
      case 'v':
         version();
         exit(EXIT_SUCCESS);
      case 't':
         set_int_option("display_time", parse_int_option(optarg)*1000);
         break;
      case 'r':
         set_int_option("reading_speed", parse_int_option(optarg));
         break;
      case 'f':
         set_string_option("font", optarg);
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

   if (daemon_flag) {
      run_cowsay_daemon(debug, argc, argv);
   }
   else {
      cowsay_init(&argc, &argv);

      if (dream_file != NULL) {
         printf("Dream %s\n", dream_file);
      }
      else if (optind == argc) {
         read_from_stdin();
      }
      else {
         char *str = cat_from_index(optind, argc, argv);
         display_cow_or_invoke_daemon(debug, str);
         free(str);
      }
   }
   
   return EXIT_SUCCESS;
}
