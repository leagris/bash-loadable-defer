/* defer - run a command when a signal fires, chaining onto any existing
   handler instead of replacing it, so handlers on one signal accumulate.

   Copyright (C) 2026 Léa Gris <lea.gris@noiraude.net>

   The help text and the trap-handling design are derived from the `trap'
   builtin in bash, Copyright (C) Free Software Foundation, Inc.

   SPDX-License-Identifier: GPL-3.0-or-later  */

#include <config.h>

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#include "loadables.h"

/* trap.h is not among the installed bash headers, so declare the trap
   primitives we use. The running bash exports these symbols. */
#define NO_SIG (-1)
#define DSIG_SIGPREFIX 0x01
#define DSIG_NOCASE 0x02
extern int decode_signal (const char *, int);
extern void set_signal (int, const char *);
extern char *trap_list[];
extern int signal_is_trapped (int);
extern int signal_is_ignored (int);
#define TRAP_STRING(s)                                                        \
  ((signal_is_trapped (s) && signal_is_ignored (s) == 0) ? trap_list[(s)]     \
                                                         : (char *)NULL)

/* Prepend so the most recently deferred command runs first (LIFO). The
   returned buffer is the caller's to free with xfree. */
static char *
chain_command (const char *newcmd, const char *existing)
{
  char *combined;

  size_t nlen = strlen (newcmd);

  if (existing == 0 || *existing == '\0')
    {
      combined = xmalloc (nlen + 1);
      strcpy (combined, newcmd);
      return (combined);
    }

  size_t elen = strlen (existing);
  combined = xmalloc (nlen + elen + 2);
  strcpy (combined, newcmd);
  combined[nlen] = '\n';
  strcpy (combined + nlen + 1, existing);
  return (combined);
}

extern int trap_builtin (WORD_LIST *);

static int
inspection_request (const WORD_LIST *list)
{
  for (; list; list = list->next)
    {
      char *w = list->word->word;
      if (w[0] != '-' || w[1] == '\0' || w[1] == '-')
        return (0);
      if (strpbrk (w + 1, "lpP"))
        return (1);
    }
  return (0);
}

int
defer_builtin (WORD_LIST *list)
{
  if (list == 0 || inspection_request (list))
    return (trap_builtin (list));

  reset_internal_getopt ();
  for (;;)
    {
      int opt = internal_getopt (list, "");
      if (opt == -1)
        break;
      switch (opt)
        {
          CASE_HELPOPT;
        default:
          builtin_usage ();
          return (EX_USAGE);
        }
    }
  list = loptend;

  if (list == 0 || list->next == 0)
    {
      builtin_usage ();
      return (EX_USAGE);
    }

  const char *command = list->word->word;
  if (command == 0 || *command == '\0')
    {
      builtin_error ("action argument is empty");
      return (EX_USAGE);
    }

  for (WORD_LIST *l = list->next; l; l = l->next)
    {
      const int sig
          = decode_signal (l->word->word, DSIG_NOCASE | DSIG_SIGPREFIX);
      if (sig == NO_SIG)
        {
          builtin_error ("%s: invalid signal specification", l->word->word);
          return (EXECUTION_FAILURE);
        }

      char *combined = chain_command (command, TRAP_STRING (sig));
      set_signal (sig, combined);
      xfree (combined);
    }

  return (EXECUTION_SUCCESS);
}

/* Blank lines use " " rather than "" because gettext maps "" to the catalog
   header, which would print as PO metadata under a non-C locale. */
char *defer_doc[] = {
  "Defer commands to run when the shell receives signals or other events.",
  " ",
  "Like `trap', but chains handlers instead of replacing them: each `defer' on",
  "a SIGNAL_SPEC adds ACTION to the existing handler rather than overwriting it.",
  "Deferred commands run last-in-first-out, so the most recently deferred ACTION",
  "runs first.",
  " ",
  "ACTION is a command read and executed when the shell receives SIGNAL_SPEC.",
  " ",
  "If a SIGNAL_SPEC is EXIT (0) ACTION is executed on exit from the shell.  If a",
  "SIGNAL_SPEC is DEBUG, ACTION is executed before every simple command and",
  "selected other commands.  If a SIGNAL_SPEC is RETURN, ACTION is executed each",
  "time a shell function or a script run by the . or source builtins finishes",
  "executing.  A SIGNAL_SPEC of ERR means to execute ACTION each time a",
  "command's failure would cause the shell to exit when the -e option is enabled.",
  " ",
  "If no arguments are supplied, defer prints the list of commands associated",
  "with each trapped signal in a form that may be reused as shell input to",
  "restore the same signal dispositions.",
  " ",
  "Options:",
  "  -l        print a list of signal names and their corresponding numbers",
  "  -p        display the commands associated with each SIGNAL_SPEC in a form",
  "            that may be reused as shell input; or for all trapped signals if",
  "            no arguments are supplied",
  "  -P        display the commands associated with each SIGNAL_SPEC.  At least",
  "            one SIGNAL_SPEC must be supplied.  -P and -p cannot be used",
  "            together.",
  " ",
  "Each SIGNAL_SPEC is either a signal name in <signal.h> or a signal number.",
  "Signal names are case insensitive and the SIG prefix is optional.  A signal",
  "may be sent to the shell with \"kill -signal $$\".",
  " ",
  "Exit Status:",
  "Returns success unless a SIGNAL_SPEC or option is invalid, or ACTION is empty.",
  (char *)NULL
};

struct builtin defer_struct = {
  "defer",
  defer_builtin,
  BUILTIN_ENABLED,
  defer_doc,
  "defer [-lpP] [action signal_spec ...]",
  0 /* reserved for internal use */
};
