#!/bin/sh
#	Run the conformance suite against an implementation.
#
#		sh conformance/run.sh              # uses C++/SliP
#		sh conformance/run.sh path/to/slip
#
#	The suite is written in SliP, not in any implementation's host language, so
#	it describes the language rather than one interpreter.  An implementation
#	passes if it can:
#
#	  - run a file, printing every toplevel value, given -p
#	  - report errors on stderr as ...:LINE: MESSAGE and exit non-zero
#
#	cases/NAME.slip   its -p output must equal cases/NAME.out exactly
#	errors/NAME.slip  must exit non-zero, with stderr containing errors/NAME.err
set -e
cd "$( dirname "$0" )/.."

SLIP="${1:-./C++/SliP}"
[ -x "$SLIP" ] || { echo "conformance: $SLIP is not executable" >&2; exit 2; }

pass=0
fail=0

#	Note: not `for _ in`, however much the rest of this repository likes _ as a
#	name — the shell overwrites $_ with the previous command's last argument.
for src in conformance/cases/*.slip; do
	want="${src%.slip}.out"
	#	CR is stripped before comparing.  A console program on Windows writes
	#	CRLF, which is that host's business and not the language's — the suite is
	#	here to describe SliP, not to insist every implementation agree with Unix
	#	about how a line ends.  The stripping is not in the pipeline that runs the
	#	implementation, because then the exit status would be tr's.
	if got="$( "$SLIP" -p "$src" 2>&1 )" && got="$( printf '%s\n' "$got" | tr -d '\r' )" \
	&& [ "$got" = "$( cat "$want" )" ]; then
		pass=$(( pass + 1 ))
	else
		printf '  FAIL %s\n' "$src"
		printf '%s\n' "$got" | diff -u "$want" - | sed 's/^/    /' || true
		fail=$(( fail + 1 ))
	fi
done

for src in conformance/errors/*.slip; do
	want="$( cat "${src%.slip}.err" )"
	if got="$( "$SLIP" "$src" 2>&1 )"; then
		printf '  FAIL %s ( expected a non-zero exit )\n' "$src"
		fail=$(( fail + 1 ))
		continue
	fi
	got="$( printf '%s\n' "$got" | tr -d '\r' )"
	case "$got" in
	*"$want"* )
		pass=$(( pass + 1 ))
		;;
	* )
		printf '  FAIL %s\n    want substring: %s\n    got:            %s\n' "$src" "$want" "$got"
		fail=$(( fail + 1 ))
		;;
	esac
done

printf 'conformance: %d passed, %d failed\n' "$pass" "$fail"
[ "$fail" -eq 0 ]
