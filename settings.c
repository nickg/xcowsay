#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "settings.h"

typedef enum {
   optInt, optBool, optString
} option_type_t;

typedef union {
   int ival;
   bool bval;
   const char *sval;
} option_value_t;

typedef struct {
   option_type_t type;
   option_value_t u;
   const char *name;
} option_t;

typedef struct _option_list_t {
   struct _option_list_t *next;
   option_t opt;
} option_list_t;

static option_list_t *options = NULL;

static option_list_t *alloc_node()
{
   option_list_t *node = (option_list_t*)malloc(sizeof(option_list_t));
   assert(node);
   node->next = NULL;
   return node;
}

static option_t *get_option(const char *name)
{
   option_list_t *it;
   for (it = options; it != NULL; it = it->next) {
      if (strcmp(name, it->opt.name) == 0)
         return &it->opt;
   }
   fprintf(stderr, "Internal Error: Invalid option %s\n", name);
   exit(EXIT_FAILURE);
}


static void assert_string(const option_t *opt)
{
   if (optString != opt->type) {
      fprintf(stderr, "Error: Option %s is not of type string\n", opt->name);
      exit(EXIT_FAILURE);
   }
}

static void assert_int(const option_t *opt)
{
   if (optInt != opt->type) {
      fprintf(stderr, "Error: Option %s is not of type integer\n", opt->name);
      exit(EXIT_FAILURE);
   }
}

static void assert_bool(const option_t *opt)
{
   if (optBool != opt->type) {
      fprintf(stderr, "Error: Option %s is not of type Boolean\n", opt->name);
      exit(EXIT_FAILURE);
   }
}

int get_int_option(const char *name)
{
   option_t *opt = get_option(name);
   assert_int(opt);
   return opt->u.ival;
}

bool get_bool_option(const char *name)
{
   option_t *opt = get_option(name);
   assert_bool(opt);
   return opt->u.bval;
}

const char *get_string_option(const char *name)
{
   option_t *opt = get_option(name);
   assert_string(opt);
   return opt->u.sval;
}

static void add_option(const char *name, option_type_t type, option_value_t def)
{
   option_t opt = { type, def, name };
   option_list_t *node = alloc_node();
   node->opt = opt;
   node->next = options;
   options = node;
}

void add_int_option(const char *name, int ival)
{
   option_value_t u;
   u.ival = ival;
   add_option(name, optInt, u);
}

void add_bool_option(const char *name, bool bval)
{
   option_value_t u;
   u.bval = bval;
   add_option(name, optBool, u);
}

static const char *copy_string(const char *s)
{
   char *copy = malloc(strlen(s)+1);
   strcpy(copy, s);
   return copy;
}

void add_string_option(const char *name, const char *sval)
{
   option_value_t u;
   u.sval = copy_string(sval);
   add_option(name, optString, u);
}

void set_int_option(const char *name, int ival)
{
   option_t *opt = get_option(name);
   assert_int(opt);
   opt->u.ival = ival;
}

void set_bool_option(const char *name, bool bval)
{
   option_t *opt = get_option(name);
   assert_bool(opt);
   opt->u.bval = bval;
}

void set_string_option(const char *name, const char *sval)
{
   option_t *opt = get_option(name);
   assert_string(opt);
   free(opt->u.sval);
   opt->u.sval = sval;
}
