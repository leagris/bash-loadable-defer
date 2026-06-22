#!/usr/bin/env bash
#
# Copyright (C) 2026 Léa Gris <lea.gris@noiraude.net>
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Chained signal handlers.
#
# Several components register an INT handler. On Ctrl-C they all run, newest
# first, instead of the last registration silently replacing the others. Run with
# --auto to deliver the signal to ourselves for a non-interactive demo.

here=${0%/*}
[ "$here" = "$0" ] && here=.
enable -f "$here/../defer" defer

defer 'echo "handler: save state"' INT
defer 'echo "handler: close connections"' INT
defer 'echo "handler: stop accepting new work"' INT

if [ "${1:-}" = --auto ]; then
  echo "delivering SIGINT to self (PID $$)"
  kill -INT $$
  echo "resumed after the interrupt"
else
  echo "running as PID $$ (interrupt with Ctrl-C, or run: kill -INT $$)"
  sleep 3
  echo "done"
fi
