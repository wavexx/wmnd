#ifndef MISC_H
#define MISC_H

/* Tokenize a shell command */
void
parse_command(const char*, char***, int*);

/* Exec a command */
pid_t
exec_command(const char*);

/* %-style substitution on a string */
typedef struct
{
  char c;
  const char* value;
} perctbl_t;

char*
percsubst(const char* string, const perctbl_t* table, int elements);

#endif
