/*  config_file.c -- Config file parser.
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
#include <ctype.h>
#include <assert.h>
#include <setjmp.h>
#include <string.h>

#include "config_file.h"
#include "settings.h"

typedef enum { tEOF, tTOKEN, tNEWLINE, tEQUALS } token_t;
static const char* tok_names[] = { "EOF", "token", "newline", "'='" };

typedef struct {
   char *buf;
   int len, off;
} strbuf_t;

static strbuf_t *make_str_buf(int len)
{
   strbuf_t *p = (strbuf_t*)malloc(sizeof(strbuf_t));
   p->len = len;
   p->off = 0;
   p->buf = malloc(len);
   assert(p->buf);
   return p;
}

static void free_str_buf(strbuf_t *sbuf)
{
   free(sbuf->buf);
   free(sbuf);
}

static void push_char(strbuf_t *sbuf, char ch)
{
   if (sbuf->off + 1 == sbuf->len)
      return;  // Silently truncate too long tokens :S
   else {
      sbuf->buf[sbuf->off++] = ch;
      sbuf->buf[sbuf->off] = '\0';
   }
}

#define clear_buf(sbuf) sbuf->off = 0
#define has_chars(sbuf) (sbuf->off > 0)
#define string(sbuf) (sbuf->buf)

static token_t next_token(FILE *f, strbuf_t* sbuf, int *lineno, jmp_buf *escape)
{
   clear_buf(sbuf);
   bool skip_to_eol = false;
   for (;;) {
      char next = fgetc(f);
      if (EOF == next)
         return (has_chars(sbuf) && !skip_to_eol) ? tTOKEN : tEOF;
      else if ('\n' == next) {
         skip_to_eol = false;
         if (has_chars(sbuf)) {
            ungetc('\n', f);
            return tTOKEN;
         }
         else {
            (*lineno)++;
            return tNEWLINE;
         }
      }
      else if (!skip_to_eol) {
         if (isspace(next)) {
            if (has_chars(sbuf))
               return tTOKEN;
         }
         else if ('=' == next)
            return tEQUALS;
         else if ('#' == next)
            skip_to_eol = true;
         else if (isalpha(next) || isdigit(next) || '_' == next)
            push_char(sbuf, next);
         else {
            fprintf(stderr, "Illegal character in xcowsayrc: %c\n", next);
            longjmp(*escape, 3);
         }
      }
   }
}

static void expect(token_t want, token_t got, bool allow_eof,
                   int lineno, jmp_buf *escape)
{
   if (tEOF == got && allow_eof)
      longjmp(*escape, 1);
   else if (want != got) {
      fprintf(stderr, "xcowsayrc: line %d: Expected %s but found %s\n",
              lineno, tok_names[want], tok_names[got]);
      longjmp(*escape, 2);
   }
}

static bool is_int_option(const char *s, int *ival)
{
   const char *p = s;
   while (*p) {
      if (isdigit(*p))
         ++p;
      else
         return false;
   }
   *ival = atoi(s);
   return true;
}

static bool is_bool_option(const char *s, bool *bval)
{
   if (strcasecmp(s, "true") == 0) {
      *bval = true;
      return true;
   }
   else if (strcasecmp(s, "false") == 0) {
      *bval = false;
      return true;
   }
   else
      return false;
}

void parse_config_file(void)
{
   char fname[FILENAME_MAX];
   const char *home = getenv("HOME");
   if (NULL == home)
      return;

   snprintf(fname, FILENAME_MAX, "%s/.xcowsayrc", home);

   FILE *frc = fopen(fname, "r");
   if (NULL == frc)
      return;
   
   const int MAX_TOKEN = 256;
   strbuf_t *opt_buf = make_str_buf(MAX_TOKEN);
   strbuf_t *val_buf = make_str_buf(MAX_TOKEN);
   strbuf_t *dummy_buf = make_str_buf(0);

   jmp_buf escape;
   if (setjmp(escape) == 0) {   
      token_t tok;
      int lineno = 1;
      for (;;) {
         tok = next_token(frc, opt_buf, &lineno, &escape);
         expect(tTOKEN, tok, true, lineno, &escape);

         tok = next_token(frc, dummy_buf, &lineno, &escape);
         expect(tEQUALS, tok, false, lineno, &escape);
         
         tok = next_token(frc, val_buf, &lineno, &escape);
         expect(tTOKEN, tok, false, lineno, &escape);

         int ival;
         bool bval;
         if (is_int_option(string(val_buf), &ival))
            set_int_option(string(opt_buf), ival);
         else if (is_bool_option(string(val_buf), &bval))
            set_bool_option(string(opt_buf), bval);
         else
            set_string_option(string(opt_buf), string(val_buf));

         tok = next_token(frc, dummy_buf, &lineno, &escape);
         expect(tNEWLINE, tok, true, lineno, &escape);
      }
   }

   free_str_buf(opt_buf);
   free_str_buf(val_buf);
   free_str_buf(dummy_buf);
   
   fclose(frc);   
}
