#!/usr/bin/env bash

set -e

BIN_DIR="$HOME/.local/bin"
PROG_NAME="calc-prog"
WRAPPER_NAME="calc"

mkdir -p "$BIN_DIR"

# compile
cc -o "$BIN_DIR/$PROG_NAME" main.c -lm

# create wrapper
cat > "$BIN_DIR/$WRAPPER_NAME" <<EOF
#!/bin/sh

BIN="\$HOME/.local/bin/$PROG_NAME"

# only use rlwrap in interactive terminal
if [ -t 0 ] && command -v rlwrap >/dev/null 2>&1; then
  exec rlwrap "\$BIN" "\$@"
else
  exec "\$BIN" "\$@"
fi
EOF

chmod +x "$BIN_DIR/$WRAPPER_NAME"

echo "Installed: $WRAPPER_NAME"
