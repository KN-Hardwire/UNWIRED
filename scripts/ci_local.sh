#!/usr/bin/env bash
# Local mirror of .github/workflows/kicad-erc-drc.yml — run before pushing to
# know CI's verdict without pushing. Uses the locally installed kicad-cli.
#
# Usage:
#   scripts/ci_local.sh              # validate the whole branch vs main
#   scripts/ci_local.sh --force-drc  # run DRC even with schematic-only label logic
set -euo pipefail
cd "$(git rev-parse --show-toplevel)"

FORCE_DRC=0
[ "${1:-}" = "--force-drc" ] && FORCE_DRC=1

CHANGED=$(git diff --name-only origin/main...HEAD)
echo "--- changed files:"; echo "$CHANGED"

sch=""; pcb=""
for f in $CHANGED; do
  case "$f" in
    *.kicad_sch) sch="$sch $f" ;;
    *.kicad_pcb) pcb="$pcb $f" ;;
  esac
done
echo "--- sch:[$sch] pcb:[$pcb]"

status=0

# --- ERC (errors + warnings) ---
if [ -n "$sch" ]; then
  # resolve changed sheets to their project root sheet
  map=$(mktemp)
  while read -r f; do
    { grep -oE '"[^"]+\.kicad_sch"' "$f" 2>/dev/null || true; } | tr -d '"' \
      | while read -r ref; do echo "$ref $f"; done >> "$map"
  done < <(find hardware modules \( -name .history -prune \) -o \( -name '*.kicad_sch' \))
  resolve() {
    local f="$1" ref
    ref=$(grep "^$(basename "$f") " "$map" | awk '{print $2}' | head -1)
    if [ -n "$ref" ] && [ "$ref" != "$f" ]; then resolve "$ref"; else echo "$f"; fi
  }
  roots=""
  for f in $sch; do roots="$roots $(resolve "$f")"; done
  roots=$(printf '%s\n' $roots | sort -u)

  for root in $roots; do
    echo "--- ERC: $root"
    out=$(mktemp --suffix=.json)
    # shortcut: mirror of kicad#24409 filter (cross-instance different_unit_net)
    if kicad-cli sch erc --severity-all --exit-code-violations --format json \
         --output "$out" "$root"; then
      echo "ERC clean: $root"
    else
      if python3 - "$out" <<'EOF'
import json, re, sys
d = json.load(open(sys.argv[1]))
bad = 0
for s in d.get('sheets', []):
    for v in s.get('violations', []):
        if v.get('type') == 'different_unit_net' and re.match(
                r'Pin \S+ is connected to both /[^/]+/(\S+) and /[^/]+/\1$',
                v.get('description', '')):
            print(f"filtered (kicad#24409): {v['description']}")
            continue
        if v.get('severity') in ('error', 'warning'):
            print(f"ERC {v['severity']}: {v['description']}")
            bad += 1
sys.exit(1 if bad else 0)
EOF
      then echo "ERC: only kicad#24409 artifacts filtered"
      else status=1
      fi
    fi
  done
else
  echo "--- ERC skipped (no schematic changed)"
fi

# --- DRC (errors only) ---
if [ -n "$pcb" ]; then
  for p in $pcb; do
    echo "--- DRC: $p"
    kicad-cli pcb drc --severity-error --schematic-parity --exit-code-violations \
      --format json --output "$(mktemp --suffix=.json)" "$p" || status=1
  done
else
  echo "--- DRC skipped (no pcb changed)"
fi

[ $status -eq 0 ] && echo "=== LOCAL CI: PASS ===" || echo "=== LOCAL CI: FAIL ==="
exit $status
