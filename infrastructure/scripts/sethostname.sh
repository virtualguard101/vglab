#!/bin/bash
# ================================================
# Description: Set hostname in container which has no systemd
# Target hostname: NEW_HOSTNAME
# ================================================

set -euo pipefail

NEW_HOSTNAME=$1

echo "=== Start to set hostname to: $NEW_HOSTNAME ==="

# Check if the user is root
if [[ $EUID -ne 0 ]]; then
    echo "The user is not root, trying to use sudo to execute..."
    exec sudo "$0" "$@"
fi

# 1. Backup the original files
echo "Backup the configuration files..."
cp -f /etc/hostname /etc/hostname.bak.$(date +%Y%m%d_%H%M%S) 2>/dev/null || true
cp -f /etc/hosts /etc/hosts.bak.$(date +%Y%m%d_%H%M%S) 2>/dev/null || true

# 2. Immediately set the hostname (UTS namespace)
echo "Immediately set the hostname..."
hostname "$NEW_HOSTNAME"

# 3. Permanent write to /etc/hostname
echo "$NEW_HOSTNAME" > /etc/hostname
echo "Written to /etc/hostname"

# 4. Secure update /etc/hosts (avoid duplicate entries)
echo "Update /etc/hosts..."

# Try to replace the existing 127.0.0.1 or 127.0.1.1 line
if grep -qE '^127\.(0\.0\.1|0\.1\.1)\s' /etc/hosts; then
    sed -i "s/^127\.(0\.0\.1|0\.1\.1)\s.*/127.0.0.1\tlocalhost $NEW_HOSTNAME/" /etc/hosts
else
    # If not, add it to the beginning of the file
    sed -i "1i127.0.0.1\tlocalhost $NEW_HOSTNAME" /etc/hosts
fi

# Extra insurance: ensure localhost mapping exists
if ! grep -q "localhost" /etc/hosts; then
    echo "127.0.0.1 localhost" >> /etc/hosts
fi

echo "Updated /etc/hosts"

# 5. Verify the results
echo ""
echo "=== Modified successfully! Verify the results below ==="
echo "Current hostname: $(hostname)"
echo "/etc/hostname content: $(cat /etc/hostname)"
echo "Related lines in hosts:"
grep -E "(localhost|$NEW_HOSTNAME)" /etc/hosts || echo "No related entries found"

echo ""
echo "Suggestion: After modification, restart the container (docker restart <container>), observe if the hostname is still $NEW_HOSTNAME."
echo "If the hostname is changed back to the original name, recommend using --hostname $NEW_HOSTNAME when running docker or setting hostname: $NEW_HOSTNAME in docker-compose.yml."

echo "Script executed successfully."
