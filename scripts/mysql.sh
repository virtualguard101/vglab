#!/usr/bin/env bash

set -euo pipefail


if ! command -v mysql &> /dev/null; then
    echo "mysql not found"
    echo "Please install mysql first"
fi

sudo mysqld --initialize --user=mysql --datadir=/var/lib/mysql

sudo systemctl enable --now mysqld
