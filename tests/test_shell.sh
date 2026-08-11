#!/bin/bash
# ──────────────────────────────────────────────────────────────────────
# test_shell.sh — Automated smoke tests for MyShell
# ──────────────────────────────────────────────────────────────────────

set -euo pipefail

SHELL_BIN="./myshell"

if [ ! -x "$SHELL_BIN" ]; then
    echo "ERROR: $SHELL_BIN not found or not executable. Run 'make' first."
    exit 1
fi

PASS=0
FAIL=0
TOTAL=0

# ESC byte for stripping ANSI codes
ESC=$(printf '\033')

run_test() {
    local desc="$1"
    local input="$2"
    local expected="$3"
    ((TOTAL++)) || true

    # Feed input to shell, strip ANSI codes, filter prompt lines
    local actual
    actual=$(printf '%s\n' "$input" | $SHELL_BIN 2>/dev/null \
        | sed "s/${ESC}\[[0-9;]*m//g" \
        | grep -v '^myshell:' \
        | grep -v '^exit$' \
        | grep -v '^$' || true)

    if echo "$actual" | grep -qF "$expected"; then
        echo "  ✓ PASS: $desc"
        ((PASS++)) || true
    else
        echo "  ✗ FAIL: $desc"
        echo "    Expected: '$expected'"
        echo "    Got:      '$actual'"
        ((FAIL++)) || true
    fi
}

cleanup() {
    rm -f /tmp/myshell_test_*.txt
}
trap cleanup EXIT

echo ""
echo "═══════════════════════════════════════════════════"
echo "  MyShell — Automated Test Suite"
echo "═══════════════════════════════════════════════════"
echo ""

echo "── Basic Commands ──"
START_DIR=$(pwd)
run_test "echo prints text"       "echo hello"              "hello"
run_test "echo with arguments"    "echo hello world"        "hello world"
run_test "pwd shows cwd"          "pwd"                     "$START_DIR"

echo ""
echo "── Built-in Commands ──"
run_test "cd + pwd"               $'cd /tmp\npwd'           "/tmp"
run_test "cd home"                $'cd\npwd'                "$HOME"
run_test "help output"            "help"                    "MyShell"

echo ""
echo "── Pipes ──"
run_test "simple pipe"            "echo hello | tr a-z A-Z" "HELLO"
run_test "multi-pipe"             $'echo hello | tr a-z A-Z | cat' "HELLO"
run_test "pipe with grep"         $'printf "apple\\nbanana\\ncherry" | grep an' "banana"

echo ""
echo "── I/O Redirection ──"
run_test "output redirect"        $'echo redirect_test > /tmp/myshell_test_out.txt\ncat /tmp/myshell_test_out.txt' "redirect_test"
run_test "append redirect"        $'echo line1 > /tmp/myshell_test_append.txt\necho line2 >> /tmp/myshell_test_append.txt\ncat /tmp/myshell_test_append.txt' "line2"
run_test "input redirect"         $'echo input_data > /tmp/myshell_test_in.txt\ncat < /tmp/myshell_test_in.txt' "input_data"

echo ""
echo "── Edge Cases ──"
run_test "empty pipe error"       "| ls"                    ""
run_test "exit command"           "exit"                    ""

echo ""
echo "═══════════════════════════════════════════════════"
echo "  Results: $PASS passed, $FAIL failed (out of $TOTAL)"
echo "═══════════════════════════════════════════════════"
echo ""

if [ "$FAIL" -gt 0 ]; then
    exit 1
fi
