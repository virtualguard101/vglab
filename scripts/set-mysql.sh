#!/usr/bin/env bash
# Reset or set MySQL root password on Arch (mysql 9.x).
#
# Background:
#   - First `systemctl start mysqld` runs mysqld --initialize and assigns a random root password.
#   - `sudo mysql -u root` does NOT work (unlike MariaDB); root uses password auth.
#   - Temp password is written to /var/log/mysql.log (not /var/log/mysql/*.log).
#
# Usage:
#   ./scripts/set-mysql.sh show-temp-password   # try to read init password from log
#   ./scripts/set-mysql.sh reset [NEW_PASSWORD] # reset root password (prompts if omitted)

set -euo pipefail

MYSQL_LOG="/var/log/mysql.log"
MYSQL_ERR_GLOB="/var/lib/mysql/*.err"

usage() {
    sed -n '2,11p' "$0" | sed 's/^# \{0,1\}//'
    echo
    echo "Commands: show-temp-password | reset [NEW_PASSWORD]"
}

require_root() {
    if [[ "${EUID:-$(id -u)}" -ne 0 ]]; then
        echo "error: run with sudo" >&2
        exit 1
    fi
}

show_temp_password() {
    require_root
    local found=0

    if [[ -f "$MYSQL_LOG" ]]; then
        echo "==> searching $MYSQL_LOG"
        if grep -F 'temporary password' "$MYSQL_LOG"; then
            found=1
        fi
    else
        echo "==> $MYSQL_LOG not found"
    fi

    # zsh rejects unmatched globs; bash is fine here.
    shopt -s nullglob
    for f in $MYSQL_ERR_GLOB; do
        echo "==> searching $f"
        if grep -F 'temporary password' "$f"; then
            found=1
        fi
    done
    shopt -u nullglob

    if [[ "$found" -eq 0 ]]; then
        echo "no temporary password found (already changed, or log rotated)."
        echo "use: sudo $0 reset"
        return 1
    fi
}

reset_root_password() {
    require_root

    local new_password="${1:-}"
    if [[ -z "$new_password" ]]; then
        read -r -s -p "new root password: " new_password
        echo
        read -r -s -p "confirm password: " new_password_confirm
        echo
        if [[ "$new_password" != "$new_password_confirm" ]]; then
            echo "error: passwords do not match" >&2
            exit 1
        fi
        if [[ -z "$new_password" ]]; then
            echo "error: password cannot be empty" >&2
            exit 1
        fi
    fi

    echo "==> stopping mysqld"
    systemctl stop mysqld

    echo "==> starting mysqld with --skip-grant-tables"
    sudo -u mysql /usr/bin/mysqld --skip-grant-tables --skip-networking &
    local mysqld_pid=$!

    cleanup() {
        if kill -0 "$mysqld_pid" 2>/dev/null; then
            kill "$mysqld_pid" 2>/dev/null || true
            wait "$mysqld_pid" 2>/dev/null || true
        fi
    }
    trap cleanup EXIT

    for _ in $(seq 1 30); do
        if mysql -u root --protocol=socket -e "SELECT 1" >/dev/null 2>&1; then
            break
        fi
        sleep 1
    done

    if ! mysql -u root --protocol=socket -e "SELECT 1" >/dev/null 2>&1; then
        echo "error: mysqld did not become ready" >&2
        exit 1
    fi

    echo "==> setting root password"
    local escaped_password="${new_password//\'/\'\'}"
    mysql -u root --protocol=socket <<SQL
FLUSH PRIVILEGES;
ALTER USER 'root'@'localhost' IDENTIFIED BY '${escaped_password}';
FLUSH PRIVILEGES;
SQL

    cleanup
    trap - EXIT

    echo "==> starting mysqld normally"
    systemctl start mysqld

    echo "==> done. test with: mysql -u root -p"
}

main() {
    local cmd="${1:-}"
    shift || true

    case "$cmd" in
        show-temp-password) show_temp_password ;;
        reset) reset_root_password "${1:-}" ;;
        -h|--help|help|"") usage ;;
        *)
            echo "error: unknown command: $cmd" >&2
            usage
            exit 1
            ;;
    esac
}

main "$@"
