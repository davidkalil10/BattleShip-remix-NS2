#!/usr/bin/env bash
# Usage: gdb_count.sh <name> <fkinds|-> <input_script> <frames> <spec>...
#   Linux counterpart of lldb_count.sh: the VS shortcut (scene 22, or GDB_START_SCENE) under the system gdb; every
#   <spec> becomes an auto-continue breakpoint and the hit count of each is printed on the single "vs=" line (so
#   parallel.sh keeps it).
#   spec:  <function>                      breakpoint by function name (static ones work too: the build has debug info)
#          "<function> if <condition>"     with a gdb condition in C, e.g. "ftRemixAiRecoveryClampWalkTargetX if
#                                          ((FTStruct*)\$rdi)->ga == 0" (x86-64: arguments in $rdi $rsi $rdx $rcx, $xmm0)
#          "print:<function>;<expression>" also prints the expression at every hit (print/x), e.g.
#                                          "print:ftRemixCommandPlayHitFGM;\$rdx"; the values land in gdb.out after "HIT"
#   Quote every spec that holds a space, "$", ">" or "(" (inside a parallel.sh custom file, escape "$" as "\$").
#   fkinds "-" skips SSB64_VS_FKINDS (use it with GDB_START_SCENE=16, 17, 52, 54...).
# Env: GDB_START_SCENE (default 22), GDB_TIMEOUT seconds (default 900), SHOTS, any SSB64_* variable of the caller.
#      GDB_PRE: a file of extra gdb commands added after the spec breakpoints and before "run" (state changes with
#      "set var", calls, "commands" blocks that print several fields). A C++ member without debug type can be
#      reached by its offset from the disassembly (e.g. "*(unsigned char*)($rdi + 8)" for GuiElement::mIsVisible).
# Output files: build/agent-shots/vs/<name>/{gdb.cmds,gdb.out,ssb64.log,stdout.log}.
# "crash=1" on the result line means the process stopped on SIGSEGV, SIGBUS or SIGABRT.
set -u
if [ $# -lt 5 ]; then
  echo "usage: gdb_count.sh <name> <fkinds|-> <input_script> <frames> <spec>..." >&2
  exit 2
fi
ROOT="$(builtin cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
name="$1"; fkinds="$2"; input="$3"; frames="$4"; shift 4
dir="$ROOT/build/agent-shots/vs/$name"
mkdir -p "$dir"
rm -f "$dir"/frame_*.png
input="$(builtin cd "$(dirname "$input")" && pwd)/$(basename "$input")"
cmds="$dir/gdb.cmds"
labels=()
{
  echo "set pagination off"
  echo "set confirm off"
  echo "set breakpoint pending on"
  echo "handle SIGSEGV SIGBUS SIGABRT stop print"
  echo "handle SIGPIPE SIGUSR1 SIGUSR2 SIG32 SIG33 nostop noprint pass"
} > "$cmds"

n=0
for spec in "$@"; do
  n=$((n + 1))
  case "$spec" in
    print:*)
      body="${spec#print:}"; func="${body%%;*}"; expr="${body#*;}"
      echo "break $func" >> "$cmds"
      printf 'commands %d\nsilent\nprintf "HIT %s\\n"\nprint/x %s\ncontinue\nend\n' "$n" "$func" "$expr" >> "$cmds"
      labels+=("$func") ;;
    *)
      echo "break $spec" >> "$cmds"
      printf 'commands %d\nsilent\ncontinue\nend\n' "$n" >> "$cmds"
      labels+=("${spec%% if *}") ;;
  esac
done
if [ -n "${GDB_PRE:-}" ]; then cat "$GDB_PRE" >> "$cmds"; fi
{
  echo "run"
  echo "info breakpoints"
  echo "quit"
} >> "$cmds"

builtin cd "$ROOT/build" || exit 2
fkind_env=()
if [ "$fkinds" != "-" ]; then fkind_env=(SSB64_VS_FKINDS="$fkinds"); fi
env SSB64_START_SCENE="${GDB_START_SCENE:-22}" "${fkind_env[@]}" SSB64_INPUT_SCRIPT="$input" SSB64_MAX_FRAMES="$frames" \
  SSB64_LOG_PATH="$dir/ssb64.log" SSB64_SAVE_PATH="$dir/save.bin" \
  SSB64_SCREENSHOT_FRAMES="${SHOTS:-600}" SSB64_SCREENSHOT_DIR="$dir" \
  timeout "${GDB_TIMEOUT:-900}" gdb -batch -x "$cmds" ./BattleShip-Remix >"$dir/gdb.out" 2>"$dir/stdout.log"
rc=$?

summary=""
for i in "${!labels[@]}"; do
  num=$((i + 1))
  hits=$(awk -v num="$num" '
    $1 == num && $2 == "breakpoint" { inside = 1; next }
    $1 ~ /^[0-9]+$/ && $2 == "breakpoint" { inside = 0 }
    inside && /breakpoint already hit/ { print $4; exit }
  ' "$dir/gdb.out")
  summary="$summary ${labels[$i]}=${hits:-0}"
done
crash=0
if grep -qE "received signal (SIGSEGV|SIGBUS|SIGABRT)" "$dir/gdb.out"; then crash=1; fi
echo "vs=$name rc=$rc shots=$(ls "$dir"/*.png 2>/dev/null | wc -l | tr -d ' ') crash=$crash$summary"
