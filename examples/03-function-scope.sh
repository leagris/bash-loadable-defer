#!/usr/bin/env bash
#
# Copyright (C) 2026 Léa Gris <lea.gris@noiraude.net>
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Function-scoped cleanup with RETURN.
#
# A command deferred on RETURN fires when the enclosing function returns. The
# cleanup is queued before the temp file is created, and each call cleans up its
# own file as it returns, so temporaries never pile up.
# shellcheck disable=SC2016

here=${0%/*}
[ "$here" = "$0" ] && here=.
enable -f "$here/../defer" defer

work=${TMPDIR:-/tmp}/defer-return.$$
defer 'rm -rf -- "$work"' EXIT
mkdir -m 700 -- "$work" || exit 1

process() {
  item=$work/$1
  defer 'echo "  cleanup ${item##*/}"; rm -f -- "$item"' RETURN
  : >"$item"
  echo "working in ${item##*/}"
}

echo "start"
process first
process second
echo "end (both temp files already gone)"
