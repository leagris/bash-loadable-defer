/* trap_internals.h -- The single point where defer mirrors bash internals.

   trap.h is not among the installed bash headers, and reading a signal's
   current trap command to chain onto it has no public API.  Every such
   borrowing lives here so the coupling stays one visible, isolated unit. */

/* SPDX-License-Identifier: GPL-3.0-or-later  */

#ifndef DEFER_TRAP_INTERNALS_H
#define DEFER_TRAP_INTERNALS_H

extern char *trap_list[];
extern int decode_signal (const char *, int);

#define NO_SIG (-1)
#define DSIG_SIGPREFIX 0x01
#define DSIG_NOCASE 0x02

#endif /* DEFER_TRAP_INTERNALS_H */