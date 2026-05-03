#!/bin/bash

set -uoe

# check and install infrastructure
if ! command -v uv &> /dev/null; then
    curl -LsSf https://astral.sh/uv/install.sh | sh
    echo 'uv installed successfully'
fi

if ! command -v cargo &> /dev/null; then
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
    echo 'cargo installed successfully'
fi

if ! command -v just &> /dev/null; then
    cargo install just
    echo 'just installed successfully'
fi

# install dependencies
uv sync
echo 'dependencies installed successfully'

# reload the shell
exec $SHELL
