#!/usr/bin/env bash
#
# Copyright (C) 2026 Léa Gris <lea.gris@noiraude.net>
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Inspecting deferred handlers, and the help text.
#
# defer shares trap's inspection switches, so you can see what is queued. These
# modes delegate to trap, so what prints is the familiar restorable form.

here=${0%/*}
[ "$here" = "$0" ] && here=.
enable -f "$here/../defer" defer

defer 'echo "EXIT: flush logs"' EXIT
defer 'echo "EXIT: remove pid file"' EXIT
defer 'echo "INT: cancel download"' INT

echo "== defer -p EXIT : chained handler in restorable form, newest first =="
defer -p EXIT

echo
echo "== defer -P EXIT : the handler commands only =="
defer -P EXIT

echo
echo "== defer (no arguments) : every trapped disposition =="
defer

echo
echo "== defer -l : signal names and numbers =="
defer -l

echo
echo "== help defer : usage and option summary =="
help defer

echo
echo "== the queued EXIT handlers now run as the script exits, newest first =="
