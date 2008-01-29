/*
 * misc: miscellaneus dockapp routines - implementation
 * Copyright(c) 1997 by Alfredo K. Kojima
 * Copyright(c) 2004 by wave++ "Yuri D'Elia" <wavexx@users.sf.net>
 * Distributed under GNU GPL (v2 or above) WITHOUT ANY WARRANTY.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "list.h"
#include "misc.h"


/*
 *----------------------------------------------------------------------
 * parse_command--
 *      Divides a command line into a argv/argc pair.
 *----------------------------------------------------------------------
 */
#define PRC_ALPHA	0
#define PRC_BLANK	1
#define PRC_ESCAPE	2
#define PRC_DQUOTE	3
#define PRC_EOS		4
#define PRC_SQUOTE	5


typedef struct
{
  short nstate;
  short output;
} DFA;


static DFA mtable[9][6] =
{
  {{ 3,  1}, { 0,  0}, { 4,  0}, { 1,  0}, { 8,  0}, { 6,  0}},
  {{ 1,  1}, { 1,  1}, { 2,  0}, { 3,  0}, { 5,  0}, { 1,  1}},
  {{ 1,  1}, { 1,  1}, { 1,  1}, { 1,  1}, { 5,  0}, { 1,  1}},
  {{ 3,  1}, { 5,  0}, { 4,  0}, { 1,  0}, { 5,  0}, { 6,  0}},
  {{ 3,  1}, { 3,  1}, { 3,  1}, { 3,  1}, { 5,  0}, { 3,  1}},
  {{-1, -1}, { 0,  0}, { 0,  0}, { 0,  0}, { 0,  0}, { 0,  0}},
  {{ 6,  1}, { 6,  1}, { 7,  0}, { 6,  1}, { 5,  0}, { 3,  0}},
  {{ 6,  1}, { 6,  1}, { 6,  1}, { 6,  1}, { 5,  0}, { 6,  1}},
  {{-1, -1}, { 0,  0}, { 0,  0}, { 0,  0}, { 0,  0}, { 0,  0}}
};


char*
next_token(const char *word, const char** next)
{
  const char* ptr;
  char* ret;
  char* t;
  int state, ctype;

  t = ret = (char*)malloc(strlen(word) + 1);
  ptr = word;

  state = 0;
  *t = 0;
  for(;;)
  {
    if(*ptr == 0)
      ctype = PRC_EOS;
    else if(*ptr == '\\')
      ctype = PRC_ESCAPE;
    else if(*ptr == '"')
      ctype = PRC_DQUOTE;
    else if(*ptr == '\'')
      ctype = PRC_SQUOTE;
    else if(*ptr == ' ' || *ptr == '\t')
      ctype = PRC_BLANK;
    else
      ctype = PRC_ALPHA;

    if(mtable[state][ctype].output)
    {
      *t = *ptr;
      t++;
      *t = 0;
    }
    state = mtable[state][ctype].nstate;
    ptr++;
    if(mtable[state][0].output < 0)
      break;
  }

  if(*ret == 0)
    t = NULL;
  else
    t = strdup(ret);
  free(ret);

  if(ctype == PRC_EOS)
    *next = NULL;
  else
    *next = ptr;

  return t;
}


void
parse_command(const char* command, char*** argv, int* argc)
{
  LinkedList* list = NULL;
  char* token;
  const char* line;
  int count, i;

  line = command;
  do
  {
    token = next_token(line, &line);
    if(token)
      list = list_cons(token, list);
  }
  while(token && line);

  count = list_length(list);
  *argv = (char**)malloc(sizeof(char*) * count);
  i = count;

  while(list)
  {
    (*argv)[--i] = (char*)list->head;
    list_remove_head(&list);
  }
  *argc = count;
}


pid_t
exec_command(const char* command)
{
  pid_t pid;
  char** argv;
  int argc;

  parse_command(command, &argv, &argc);
  if(!argv)
    return 0;

  if((pid = fork()) == 0)
  {
    char** args;
    int i;

    args = (char**)malloc(sizeof(char*) * (argc + 1));
    if(!args)
      exit(10);
    for(i = 0; i < argc; i++)
      args[i] = argv[i];
    args[argc] = NULL;
    execvp(argv[0], args);
    exit(10);
  }
  return pid;
}


char*
percsubst(const char* string, const perctbl_t* table, int elements)
{
  size_t inlen = strlen(string);
  size_t outlen = inlen * 2;
  char* buf = (char*)malloc(outlen);
  const char* rptr;
  char* wptr;
  const perctbl_t* it;
  size_t len;
  size_t dist;

  for(wptr = buf, rptr = string; *rptr;)
  {
    /* normal characters */
    if(*rptr != '%')
    {
      *(wptr++) = *(rptr++);
      continue;
    }

    /* peek the next one */
    if(*(++rptr) == '%')
    {
      *(wptr++) = *(rptr++);
      continue;
    }

    /* search for an element in the table */
    for(it = table; it != table + elements; ++it)
      if(*rptr == it->c)
      {
	len = strlen(it->value);
	dist = wptr - buf;
	if((outlen - dist) >= len)
	{
	  outlen += len * 2;
	  buf = (char*)realloc(buf, outlen);
	  wptr = buf + dist;
	}

	memcpy(wptr, it->value, len);
	wptr += len;
	break;
      }

    /* go on */
    ++rptr;
  }
  *wptr = 0;

  return buf;
}
