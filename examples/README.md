# `defer` examples

Runnable demonstrations of the `defer` loadable. Build the loadable first:

```sh
make
```

Then run any example from the repository root, for instance:

```sh
bash examples/01-lifo-cleanup.sh
bash examples/04-signal-handlers.sh --auto
```

Each script enables the loadable from `../defer` relative to its own location.

| Script | Shows |
| --- | --- |
| `01-lifo-cleanup.sh` | Resources released in reverse acquisition order on EXIT. |
| `02-independent-cleanups.sh` | Multiple subsystems sharing one EXIT signal without overwriting each other. |
| `03-function-scope.sh` | RETURN-scoped cleanup that fires per function call. |
| `04-signal-handlers.sh` | Chained INT handlers that all run on Ctrl-C. |
| `05-inspect-and-help.sh` | The `-p`, `-P`, `-l` switches, no-arg listing, and `help defer`. |
