# defer

A Bash loadable builtin that defers commands to run on shell events, **chaining
handlers instead of replacing them**.

Plain `trap` keeps exactly one handler per signal, so a second `trap` silently
discards the first:

```sh
trap 'echo close db'   EXIT
trap 'echo flush logs' EXIT   # the db handler is now gone
# on exit: flush logs
```

`defer` adds to the handler instead, and runs the deferred commands
last-in-first-out, like Go's `defer`:

```sh
defer 'echo close db'   EXIT
defer 'echo flush logs' EXIT
# on exit:
#   flush logs
#   close db
```

This makes cleanup composable: independent pieces of code can each register
their own teardown on `EXIT` (or `RETURN`, `INT`, `ERR`, ...) without knowing or
clobbering one another.

## Usage

```
defer [-lpP] [action signal_spec ...]
```

- `defer ACTION SIGNAL_SPEC ...` queues `ACTION` to run when the shell receives
  each `SIGNAL_SPEC`. The most recently deferred action runs first.
- `SIGNAL_SPEC` is a signal name (case-insensitive, `SIG` prefix optional) or
  number, or one of the pseudo-signals `EXIT`, `RETURN`, `DEBUG`, `ERR`.
- With no action arguments, or with `-l`, `-p`, or `-P`, `defer` behaves like
  `trap`: it lists registered handlers or signal names. So `defer -p EXIT`
  prints the accumulated handler in a form you can reuse as input.

Common scopes:

| `SIGNAL_SPEC` | Action runs |
| --- | --- |
| `EXIT` | when the shell exits |
| `RETURN` | when the enclosing function (or sourced script) returns |
| `INT` | on `Ctrl-C` |
| `ERR` | when a command's failure would exit a `set -e` shell |

```sh
process() {
  local tmp=
  defer 'rm -f -- "$tmp"; printf "Cleanup of: %s\\n" "$tmp"' RETURN
  tmp=$(mktemp)
  ls -l "$tmp"
}
```

See `help defer` once loaded for the full description.

## Build and load

Requires bash built with loadable-builtin support, its development headers, and
`pkg-config` (the bash package ships `bash.pc`). Nothing is installed
system-wide.

```sh
./configure      # locates the loadables dir via pkg-config
make             # builds ./defer
```

Load it into a running shell, use it, and unload when done:

```sh
enable -f "$PWD/defer" defer
defer 'echo bye' EXIT
enable -d defer                  # unload (required before reloading a rebuild)
```

Other targets: `make test` runs the suite, `make format` runs GNU `indent`,
`make clean` / `make distclean` remove build output.

## Examples

Runnable scripts live in [`examples/`](examples/), each self-loading the
freshly built `./defer`:

| Script | Shows |
| --- | --- |
| `01-lifo-cleanup.sh` | Resources released in reverse order on `EXIT`. |
| `02-independent-cleanups.sh` | Separate subsystems sharing `EXIT` without overwriting. |
| `03-function-scope.sh` | `RETURN`-scoped cleanup firing per function call. |
| `04-signal-handlers.sh` | Chained `INT` handlers all running on `Ctrl-C`. |
| `05-inspect-and-help.sh` | The `-l`, `-p`, `-P` switches and `help defer`. |

```sh
bash examples/01-lifo-cleanup.sh
```

## How it works

`defer` is a thin layer over bash's own trap machinery rather than a separate
command queue:

- For each `SIGNAL_SPEC`, it reads the current trap string, **prepends** the new
  `ACTION` (separated by a newline), and installs the combined string with
  `set_signal`. Prepending is what gives the last-in-first-out order.
- Because the handler is an ordinary trap string, the inspection flags and
  no-argument listing simply delegate to the built-in `trap`, so `defer -p` and
  `trap -p` show the same thing.

A consequence worth knowing: deferred commands and any plain `trap` you set
share the same per-signal handler, and there is no separate "undefer". Clearing
a signal with `trap - SIGNAL_SPEC` drops every command deferred on it.

## License

GPL-3.0-or-later. The help text and trap-handling design derive from the `trap`
builtin in bash, Copyright (C) Free Software Foundation, Inc.
