#!/usr/bin/env bash
#
# Copyright (C) 2026 Léa Gris <lea.gris@noiraude.net>
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Deferred cleanup in LIFO order.
#
# Each release is queued with defer BEFORE the resource exists, so a resource is
# never created without its cleanup already registered. On exit the releases run
# in reverse order, the way a stack unwinds.
# shellcheck disable=SC2016

here=${0%/*}
[ "$here" = "$0" ] && here=.
enable -f "$here/../defer" defer

work=${TMPDIR:-/tmp}/defer-lifo.$$
defer 'echo "  release workdir"; rm -rf -- "$work"' EXIT
mkdir -m 700 -- "$work" || exit 1
echo "acquired workdir: $work"

lock=$work/lock
defer 'echo "  release lock"; rm -f -- "$lock"' EXIT
: >"$lock"
echo "acquired lock"

data=$work/data
defer 'echo "  release data file"; rm -f -- "$data"' EXIT
: >"$data"
echo "acquired data file: $data"

echo "doing work, then unwinding..."
