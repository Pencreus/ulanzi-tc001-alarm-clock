#!/bin/zsh
# Verify each 64 KB chunk on the chip against the image; rewrite any that differ.
# Needs .chunks/ from flash-clock.sh. Usage: ./verify-fix.sh firmware/<name>.factory.bin [start_chunk]
setopt nonomatch
cd "$(dirname "$0")"
CH=65536
N=$(ls .chunks | wc -l | tr -d ' ')
IMG="${1:?usage: ./verify-fix.sh firmware/<name>.factory.bin [start_chunk]}"
i=${2:-0}
bad=()
while [ $i -lt $N ]; do
  P=$(ls /dev/cu.usbserial* 2>/dev/null | head -1)
  if [ -z "$P" ] || ! stty -f "$P" 115200 2>/dev/null; then printf "waiting for a fresh USB connection...\r"; sleep 2; continue; fi
  sleep 3
  off=$(printf "0x%x" $((i * CH)))
  f=$(printf ".chunks/c%03d" $i)
  out=$(esptool --port "$P" --baud 115200 --connect-attempts 3 --after no-reset verify-flash "$off" "$f" 2>&1)
  if echo "$out" | grep -aq 'Verification successful'; then
    echo "== chunk $((i + 1))/$N ok"
    i=$((i + 1))
  elif echo "$out" | grep -aq 'digest mismatch'; then
    echo "== chunk $((i + 1))/$N DIFFERS -> rewriting"
    bad+=($((i + 1)))
    out2=$(esptool --port "$P" --baud 115200 --connect-attempts 3 --after no-reset write-flash "$off" "$f" 2>&1)
    if echo "$out2" | grep -aq 'Hash of data verified'; then echo "== chunk $((i + 1)) rewritten"; i=$((i + 1)); else echo "== rewrite failed, will retry"; sleep 5; fi
  else
    echo "== chunk $((i + 1)) link dropped: $(echo "$out" | grep -aE 'rror|Lost|stopped' | tail -n 1 | cut -c1-80)"
    sleep 6
  fi
done
echo "== final whole-image check"
while true; do
  P=$(ls /dev/cu.usbserial* 2>/dev/null | head -1)
  if [ -z "$P" ] || ! stty -f "$P" 115200 2>/dev/null; then printf "waiting for a fresh USB connection...\r"; sleep 2; continue; fi
  sleep 3
  out=$(esptool --port "$P" --baud 115200 verify-flash 0x0 "$IMG" 2>&1)
  if echo "$out" | grep -aq 'Verification successful'; then echo "== ALL DONE: image verified, chip reset"; break; fi
  echo "== whole image not confirmed: $(echo "$out" | grep -aE 'rror|differ|mismatch|Lost|stopped' | tail -n 1 | cut -c1-80)"
  sleep 6
done
echo "bad chunks that were rewritten: ${bad[*]:-none}"
