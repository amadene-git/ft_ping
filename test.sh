#!/usr/bin/env bash
set -u

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

TMP_DIR="$(mktemp -d)"
FAILURES=0
PING_CMD=()
PYTHON_CMD=()

if [ -t 1 ] && [ -z "${NO_COLOR:-}" ]; then
  GREEN=$'\033[32m'
  RED=$'\033[31m'
  YELLOW=$'\033[33m'
  BOLD=$'\033[1m'
  RESET=$'\033[0m'
else
  GREEN=''
  RED=''
  YELLOW=''
  BOLD=''
  RESET=''
fi

cleanup() {
  rm -rf "$TMP_DIR"
}
trap cleanup EXIT

pass() {
  printf '%s[PASS]%s %s\n' "$GREEN" "$RESET" "$1"
}

fail() {
  printf '%s[FAIL]%s %s\n' "$RED" "$RESET" "$1"
  FAILURES=$((FAILURES + 1))
}

skip() {
  printf '%s[SKIP]%s %s\n' "$YELLOW" "$RESET" "$1"
}

need_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    fail "missing command: $1"
    exit 1
  fi
}

can_sudo_without_password() {
  command -v sudo >/dev/null 2>&1 && sudo -n true >/dev/null 2>&1
}

can_run_directly() {
  local output

  output="$(timeout -s INT -k 1s 1s stdbuf -oL ./ft_ping 127.0.0.1 2>&1)"
  ! printf '%s\n' "$output" | grep -q 'Operation not permitted'
}

select_ping_command() {
  if can_run_directly; then
    PING_CMD=(stdbuf -oL ./ft_ping)
  elif can_sudo_without_password; then
    PING_CMD=(sudo -n stdbuf -oL ./ft_ping)
  else
    fail 'ft_ping needs CAP_NET_RAW. Run sudo ./test.sh or sudo setcap cap_net_raw+ep ./ft_ping.'
    exit 1
  fi
}

select_python_command() {
  if ! command -v python3 >/dev/null 2>&1; then
    return
  fi
  if [ "$(id -u)" -eq 0 ]; then
    PYTHON_CMD=(python3)
  elif can_sudo_without_password; then
    PYTHON_CMD=(sudo -n python3)
  fi
}

show_file_on_failure() {
  local file="$1"

  printf '%s\n' '--- output ---'
  sed -n '1,120p' "$file"
  printf '%s\n' '--------------'
}

test_localhost_ping() {
  local output_file="$TMP_DIR/localhost.out"

  timeout -s INT -k 1s 3s "${PING_CMD[@]}" 127.0.0.1 >"$output_file" 2>&1
  if grep -Eq '^FT_PING 127\.0\.0\.1 \(127\.0\.0\.1\) 56\(84\) bytes of data\.$' "$output_file"; then
    pass 'localhost header reports 56 data bytes and 84 total bytes'
  else
    fail 'localhost header does not report 56(84) bytes'
    show_file_on_failure "$output_file"
  fi

  if grep -Eq '^64 bytes from 127\.0\.0\.1: icmp_seq=1 ' "$output_file"; then
    pass 'localhost echo reply reports 64 ICMP bytes'
  else
    fail 'localhost echo reply does not report 64 ICMP bytes'
    show_file_on_failure "$output_file"
  fi

  if grep -Eq 'bytes from .*127\.0\.0\.1.*icmp_seq=' "$output_file"; then
    pass 'localhost echo reply is accepted'
  else
    fail 'localhost echo reply was not printed'
    show_file_on_failure "$output_file"
  fi
}

inject_rogue_echo_reply() {
  local dst="$1"
  local output_file="$2"

  "${PYTHON_CMD[@]}" - "$dst" >"$output_file" 2>&1 <<'PY'
import socket
import struct
import sys

dst = sys.argv[1]
ident = 0x4242
seq = 0x1337
payload = b"rogue-ft-ping"

def checksum(data):
    if len(data) % 2:
        data += b"\x00"
    total = 0
    for i in range(0, len(data), 2):
        total += (data[i] << 8) + data[i + 1]
    total = (total & 0xffff) + (total >> 16)
    total = (total & 0xffff) + (total >> 16)
    return (~total) & 0xffff

packet = struct.pack("!BBHHH", 0, 0, 0, ident, seq) + payload
packet = struct.pack("!BBHHH", 0, 0, checksum(packet), ident, seq) + payload

sock = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_ICMP)
sock.sendto(packet, (dst, 0))
PY
}

test_rogue_echo_reply_is_ignored() {
  local target="${ROGUE_TARGET:-192.0.2.123}"
  local injection_dst="${ROGUE_DST:-127.0.0.1}"
  local output_file="$TMP_DIR/rogue.out"
  local injector_output="$TMP_DIR/inject.out"
  local ping_pid

  if [ "${#PYTHON_CMD[@]}" -eq 0 ]; then
    skip 'rogue ICMP test needs privileged python3'
    return
  fi

  timeout -s INT -k 1s 2s "${PING_CMD[@]}" "$target" >"$output_file" 2>&1 &
  ping_pid=$!
  sleep 0.3

  if ! inject_rogue_echo_reply "$injection_dst" "$injector_output"; then
    fail 'could not inject rogue ICMP echo reply'
    show_file_on_failure "$injector_output"
    wait "$ping_pid" >/dev/null 2>&1
    return
  fi

  wait "$ping_pid" >/dev/null 2>&1

  if grep -q 'sendTo() failed' "$output_file"; then
    skip "rogue ICMP test target is not sendable: $target"
  elif grep -q 'bytes from' "$output_file"; then
    fail 'rogue ICMP echo reply was accepted'
    show_file_on_failure "$output_file"
  else
    pass 'rogue ICMP echo reply is ignored'
  fi
}

need_cmd make
need_cmd timeout
need_cmd stdbuf
need_cmd grep
need_cmd sed
need_cmd mktemp

if ! make; then
  fail 'build failed'
  exit 1
fi
select_ping_command
select_python_command

printf '%sUsing ft_ping command:%s %s\n' "$BOLD" "$RESET" "${PING_CMD[*]}"
test_localhost_ping
test_rogue_echo_reply_is_ignored

if [ "$FAILURES" -ne 0 ]; then
  printf '%s%d test(s) failed.%s\n' "$RED" "$FAILURES" "$RESET"
  exit 1
fi

printf '%sAll runnable tests passed.%s\n' "$GREEN" "$RESET"
