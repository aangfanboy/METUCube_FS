#!/bin/bash
# =============================================================================
# firstrun.sh — SD-card-only provisioner (NO network, NO USB, NO SSH needed)
#
# Drop this file + can-bringup.sh onto the FAT boot partition, then append the
# systemd.run token to cmdline.txt (see README). On the next boot the kernel
# runs this script ONCE as root, which:
#   1. installs can-bringup.sh to /usr/local/sbin
#   2. installs + enables can-bringup.service
#   3. removes itself from cmdline.txt and reboots into the normal system
#
# Everything below uses only tools present in Raspberry Pi OS Lite — no apt,
# no internet. Progress is logged to /boot/firmware/firstrun.log so you can
# read it by pulling the SD card.
# =============================================================================
set +e

BOOT=/boot/firmware
[ -d "$BOOT" ] || BOOT=/boot
LOG="$BOOT/firstrun.log"

log() { echo "[$(date '+%F %T')] $*" | tee -a "$LOG"; }
: > "$LOG"
log "=== firstrun provisioner start ==="

# 1. Install the bring-up script
if [ -f "$BOOT/can-bringup.sh" ]; then
    install -m 0755 "$BOOT/can-bringup.sh" /usr/local/sbin/can-bringup.sh
    # Strip any CR line endings picked up from a Windows editor
    sed -i 's/\r$//' /usr/local/sbin/can-bringup.sh
    log "installed /usr/local/sbin/can-bringup.sh"
else
    log "ERROR: $BOOT/can-bringup.sh not found — cannot install"
fi

# 2. Write the systemd unit
cat > /etc/systemd/system/can-bringup.service <<'UNIT'
[Unit]
Description=Bring up MCP2518FD CAN interface (can0) at 500k, independent of cFS
After=systemd-udev-settle.service
Wants=systemd-udev-settle.service

[Service]
Type=oneshot
RemainAfterExit=yes
ExecStart=/usr/local/sbin/can-bringup.sh

[Install]
WantedBy=multi-user.target
UNIT
log "wrote /etc/systemd/system/can-bringup.service"

systemctl enable can-bringup.service >>"$LOG" 2>&1
log "enabled can-bringup.service"

# 3. Self-cleanup: remove the systemd.run tokens from cmdline.txt so the normal
#    system boots from now on.
sed -i 's| systemd.run=[^ ]*||g; s| systemd.run_success_action=[^ ]*||g; s| systemd.unit=[^ ]*||g' "$BOOT/cmdline.txt"
log "cleaned cmdline.txt"

log "=== firstrun provisioner done — rebooting ==="
exit 0
