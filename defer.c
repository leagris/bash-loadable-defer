/* defer -- Run a command when a signal fires, chaining onto any existing
   handler instead of replacing it, so handlers on one signal accumulate.

   Copyright (C) 2026 Léa Gris <lea.gris@noiraude.net>

   The help text and the trap-handling design are derived from the `trap'
   builtin in bash, Copyright (C) Free Software Foundation, Inc. */

/* SPDX-License-Identifier: GPL-3.0-or-later  */

#include <config.h>

#include <signal.h>		/* SIG_DFL / SIG_IGN sentinels stored in trap_list */

#include "loadables.h"

#include "trap_internals.h"	/* every bash-internal borrowing, isolated */

/* End-of-options marker. */
#define OPT_SEP "--"

/* A command bound to a signal: the pairing `defer' installs. */
typedef struct
{
  char *command;
  char *sigspec;
} deferral;

static char *
  __attribute__((returns_nonnull, warn_unused_result, malloc, malloc (xfree),
		 nonnull (1))) prepend_cmdline (const char *newcmd,
						const char *existing)
{
  const size_t nlen = strlen (newcmd);

  if (existing == 0 || *existing == '\0')
    return savestring (newcmd);

  const size_t elen = strlen (existing);
  char *combined = xmalloc (nlen + elen + 2);
  strcpy (combined, newcmd);
  combined[nlen] = '\n';
  strcpy (combined + nlen + 1, existing);
  return combined;
}

static const char *
  __attribute__((returns_nonnull, warn_unused_result, nonnull))
current_trap (const char *sigspec)
{
  const int sig = decode_signal (sigspec, DSIG_NOCASE | DSIG_SIGPREFIX);
  if (sig == NO_SIG)
    return "";
  const char *existing = trap_list[sig];
  if (existing == (char *) SIG_DFL || existing == (char *) SIG_IGN)
    return "";
  return existing;
}

static int
  __attribute__((pure, warn_unused_result))
is_trap_query (const WORD_LIST *args)
{
  if (args == 0)
    return 1;
  for (; args; args = args->next)
    {
      char *arg = args->word->word;
      if (arg[0] != '-' || arg[1] == '\0' || arg[1] == '-')
	return 0;
      if (strpbrk (arg + 1, "lpP"))
	return 1;
    }
  return 0;
}

static int
  __attribute__((nonnull, warn_unused_result))
trap_set (sh_builtin_func_t *trap_fn, const char *sigspec,
	  const char *command)
{
  WORD_LIST *args = add_string_to_list (sigspec, NULL);
  args = add_string_to_list (command, args);
  args = add_string_to_list (OPT_SEP, args);
  const int rc = trap_fn (args);
  dispose_words (args);
  return rc;
}

static int
  __attribute__((nonnull (1), warn_unused_result))
trap_chain (sh_builtin_func_t *trap_fn, const deferral d)
{
  const char *existing = current_trap (d.sigspec);
  char *combined = prepend_cmdline (d.command, existing);
  const int rc = trap_set (trap_fn, d.sigspec, combined);
  xfree (combined);
  return rc;
}

int
defer_builtin (WORD_LIST *args)
{
  sh_builtin_func_t *trap_fn = find_shell_builtin ("trap");
  if (trap_fn == 0)
    {
      builtin_error ("trap builtin is not available");
      return EXECUTION_FAILURE;
    }

  if (is_trap_query (args))
    return trap_fn (args);

  reset_internal_getopt ();
  for (;;)
    {
      const int opt = internal_getopt (args, "");
      if (opt == -1)
	break;
      switch (opt)		/* NOLINT(*-multiway-paths-covered) */
	/* CASE_HELPOPT expands to a case */
	{
	  CASE_HELPOPT;
	default:
	  builtin_usage ();
	  return EX_USAGE;
	}
    }
  const WORD_LIST *operands = loptend;

  if (operands == 0 || operands->next == 0)
    {
      builtin_usage ();
      return EX_USAGE;
    }

  char *command = operands->word->word;
  if (command == 0 || *command == '\0')
    {
      builtin_error ("action argument is empty");
      return EX_USAGE;
    }

  int rc = EXECUTION_SUCCESS;
  for (const WORD_LIST * sigspecs = operands->next;
       sigspecs && rc == EXECUTION_SUCCESS; sigspecs = sigspecs->next)
    rc = trap_chain (trap_fn, (deferral)
		     {
		     command, sigspecs->word->word});
  return rc;
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
  (char *) NULL
};

struct builtin defer_struct = {
  "defer",
  defer_builtin,
  BUILTIN_ENABLED,
  defer_doc,
  "defer [-lpP] [action signal_spec ...]",
  0				/* reserved for internal use */
};
