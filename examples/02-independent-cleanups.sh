#!/usr/bin/env bash
#
# Copyright (C) 2026 Léa Gris <lea.gris@noiraude.net>
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Independent handlers that share one signal.
#
# Each subsystem queues its own teardown on EXIT before it starts up, so the
# cleanup is registered no matter where a later startup step fails. With plain
# `trap` the last registration would replace the earlier ones; defer chains them.

here=${0%/*}
[ "$here" = "$0" ] && here=.
enable -f "$here/../defer" defer

start_db() {
  defer 'echo "db: disconnect"' EXIT
  echo "db: connect"
}

start_cache() {
  defer 'echo "cache: flush"' EXIT
  echo "cache: warm"
}

start_server() {
  defer 'echo "server: stop"' EXIT
  echo "server: listen"
}

start_db
start_cache
start_server
echo "all subsystems up; main work here..."
