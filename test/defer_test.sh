#!/usr/bin/env bash
#
# Copyright (C) 2026 Léa Gris <lea.gris@noiraude.net>
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Behaviour tests for the defer loadable. Run via `make test`.

here=${0%/*}
[ "$here" = "$0" ] && here=.
obj="$here/../defer"
: "${THIS_SH:=bash}"
fail=0

check() {
  local name=$1 expected=$2 actual=$3
  if [ "$actual" = "$expected" ]; then
    printf 'ok   - %s\n' "$name"
  else
    printf 'FAIL - %s\n       expected: [%s]\n       actual:   [%s]\n' \
      "$name" "$expected" "$actual"
    fail=1
  fi
}

[ -f "$obj" ] || { echo "build first: $obj missing"; exit 2; }

out=$("$THIS_SH" -c '
  enable -f "'"$obj"'" defer
  defer "printf one" EXIT
  defer "printf two" EXIT
')
check "LIFO EXIT order" "twoone" "$out"

out=$("$THIS_SH" -c '
  enable -f "'"$obj"'" defer
  trap "printf base" EXIT
  defer "printf added" EXIT
')
check "chains over existing trap" "addedbase" "$out"

out=$("$THIS_SH" -c '
  enable -f "'"$obj"'" defer
  f() { defer "printf ret" RETURN; }
  f
')
check "RETURN trap fires" "ret" "$out"

out=$("$THIS_SH" -c '
  enable -f "'"$obj"'" defer
  f() { defer "printf x" EXIT RETURN; }
  f
  printf "|"
')
check "registers on each signal spec" "x|x" "$out"

if "$THIS_SH" -c 'enable -f "'"$obj"'" defer; defer "true" NOSUCHSIG' 2>/dev/null; then
  check "invalid signal rejected" "nonzero" "zero"
else
  check "invalid signal rejected" "nonzero" "nonzero"
fi

if "$THIS_SH" -c 'enable -f "'"$obj"'" defer; defer "" EXIT' 2>/dev/null; then
  check "empty command rejected" "nonzero" "zero"
else
  check "empty command rejected" "nonzero" "nonzero"
fi

out=$("$THIS_SH" -c '
  enable -f "'"$obj"'" defer
  defer "echo one" EXIT
  defer "echo two" EXIT
  defer -p EXIT
  trap - EXIT
')
check "-p shows chained handler" "trap -- 'echo two
echo one' EXIT" "$out"

out=$("$THIS_SH" -c '
  enable -f "'"$obj"'" defer
  defer "echo hi" INT
  defer
  trap - INT
')
check "no-args lists traps" "trap -- 'echo hi' SIGINT" "$out"

if "$THIS_SH" -c 'enable -f "'"$obj"'" defer; defer -l' | grep -q SIGINT; then
  check "-l lists signal names" "yes" "yes"
else
  check "-l lists signal names" "yes" "no"
fi

if "$THIS_SH" -c 'enable -f "'"$obj"'" defer; defer -pP EXIT' 2>/dev/null; then
  check "-p and -P mutually exclusive" "nonzero" "zero"
else
  check "-p and -P mutually exclusive" "nonzero" "nonzero"
fi

exit $fail
