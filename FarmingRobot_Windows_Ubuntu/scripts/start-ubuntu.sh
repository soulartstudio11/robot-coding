#!/usr/bin/env bash
set -e
HERE="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$HERE/../web-terminal"
echo "Farming Robot terminal: http://localhost:8000"
command -v xdg-open >/dev/null 2>&1 && xdg-open http://localhost:8000 >/dev/null 2>&1 || true
if command -v python3 >/dev/null 2>&1; then exec python3 -m http.server 8000; fi
echo "Python 3 not found. Install it with: sudo apt install python3"
exit 1
