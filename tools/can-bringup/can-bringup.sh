#!/bin/bash
# =============================================================================
# can-bringup.sh
#
# Brings up the MCP2518FD CAN interface (can0) at 500 kbit/s, independently of
# cFS. Designed for a HEADLESS IO board (no HDMI, no SSH): all diagnostics are
# written to a log file on the FAT boot partition (/boot/firmware) so the log
# can be read by simply pulling the SD card and opening it on any PC.
#
# Install as a systemd service (see can-bringup.service) so it runs at boot.
# =============================================================================

set -u

BITRATE="${CAN_BITRATE:-500000}"
IFACE="${CAN_IFACE:-can0}"

# Log to the FAT boot partition so it is readable off a pulled SD card.
# Fall back to /var/log if the boot partition path differs.
if [ -d /boot/firmware ]; then
    LOG="/boot/firmware/can-status.log"
elif [ -d /boot ]; then
    LOG="/boot/can-status.log"
else
    LOG="/var/log/can-status.log"
fi

log() { echo "[$(date '+%Y-%m-%d %H:%M:%S')] $*" | tee -a "$LOG"; }

# Start a fresh log each boot
: > "$LOG"
log "=== CAN bring-up start (iface=$IFACE bitrate=$BITRATE) ==="

# --- 1. Kernel / driver diagnostics -----------------------------------------
log "--- uname ---"
uname -a | tee -a "$LOG"

log "--- dmesg (mcp/spi/can) ---"
dmesg | grep -iE 'mcp251|spi|can' | tail -n 40 | tee -a "$LOG"

log "--- SPI devices present ---"
ls -l /sys/bus/spi/devices 2>&1 | tee -a "$LOG"

log "--- Loaded CAN modules ---"
lsmod | grep -iE 'mcp251|can' | tee -a "$LOG"

# --- 2. Wait for the interface to appear (driver may bind a bit late) -------
log "--- Waiting up to 10s for $IFACE to appear ---"
for i in $(seq 1 20); do
    if [ -d "/sys/class/net/$IFACE" ]; then
        log "$IFACE appeared after ~$((i * 500))ms"
        break
    fi
    sleep 0.5
done

if [ ! -d "/sys/class/net/$IFACE" ]; then
    log "ERROR: $IFACE does NOT exist. The mcp251xfd driver did not bind."
    log "  -> Check config.txt overlay (spi bus / cs / interrupt / oscillator)."
    log "  -> See dmesg lines above; 'no such device' in cFS is expected until this is fixed."
    log "--- Full 'ip link' for reference ---"
    ip link show 2>&1 | tee -a "$LOG"
    log "=== CAN bring-up FAILED ==="
    exit 1
fi

# --- 3. Bring the interface up ----------------------------------------------
# Take it down first in case a previous attempt left it half-configured.
ip link set "$IFACE" down 2>/dev/null

if ip link set "$IFACE" up type can bitrate "$BITRATE" restart-ms 100 2>>"$LOG"; then
    log "$IFACE brought up at $BITRATE bit/s"
else
    log "ERROR: failed to bring up $IFACE at $BITRATE (see errors above)"
    log "=== CAN bring-up FAILED ==="
    exit 1
fi

# --- 4. Final state ----------------------------------------------------------
log "--- ip -details -statistics link show $IFACE ---"
ip -details -statistics link show "$IFACE" 2>&1 | tee -a "$LOG"

log "=== CAN bring-up OK ==="
exit 0
