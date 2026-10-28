#!/bin/zsh
# Resumable flash for a Ulanzi TC001: writes the image in 64 KB chunks so a USB
# drop only costs one chunk, not the whole image. Progress is kept in .flash_next.
# Usage: ./flash-clock.sh firmware/<name>.factory.bin [start_chunk]
setopt nonomatch
cd "$(dirname "$0")"
IMG="${1:?usage: ./flash-clock.sh firmware/<name>.factory.bin [start_chunk]}"
CH=65536
SIZE=$(stat -f %z "$IMG")
N=$(( (SIZE + CH - 1) / CH ))
WORK=".chunks"
rm -rf "$WORK"; mkdir -p "$WORK"
split -b $CH -a 3 -d "$IMG" "$WORK/c"
i=${2:-0}
echo "== $IMG: $SIZE bytes, $N chunks, starting at chunk $i"
while [ $i -lt $N ]; do
  P=$(ls /dev/cu.usbserial* 2>/dev/null | head -1)
  # only attempt on a fresh, settled port: a failed attempt wedges it until replug
  if [ -z "$P" ] || ! stty -f "$P" 115200 2>/dev/null; then printf "waiting for a fresh USB connection...\r"; sleep 2; continue; fi
  sleep 4
  off=$(printf "0x%x" $((i * CH)))
  f=$(printf "%s/c%03d" "$WORK" $i)
  after=no-reset
  out=$(esptool --port "$P" --baud 115200 --connect-attempts 3 --after $after write-flash "$off" "$f" 2>&1)
  if [ $? -eq 0 ]; then
    i=$((i + 1))
    echo "== chunk $((i))/$N done ($((i * 100 / N))%)"
  else
    echo "== chunk $((i + 1))/$N FAILED: $(echo "$out" | grep -aE 'rror|Lost|timeout' | tail -n 1 | cut -c1-90)"
    echo "== link dropped; replug the clock and this chunk retries"
    sleep 6
  fi
done
echo "== verifying whole image on the chip"
while true; do
  out=$(esptool --port "$(ls /dev/cu.usbserial* | head -1)" --baud 115200 verify-flash 0x0 "$IMG" 2>&1)
  echo "$out" | grep -aq 'Verification successful' && break
  echo "== verify not confirmed: $(echo "$out" | grep -aE 'rror|differ|Lost|mismatch' | tail -n 1 | cut -c1-90); retrying"
  sleep 5
done
echo "== ALL DONE"
