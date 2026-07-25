#!/bin/sh
#	Assemble the npm package. Run from the repository root:
#
#		sh WASM/build.sh      # or: sh WASM.sh, for the docker route
#		sh npm/build.sh
#		npm publish npm/      # requires npm credentials — not done by this script
#
#	The engine artifacts are build output and are gitignored, so the package is
#	staged here rather than committed.
set -e
cd "$( dirname "$0" )/.."

[ -f Web/SliP.js ]   || { echo "npm/build.sh: Web/SliP.js missing — run WASM/build.sh first"   >&2; exit 1; }
[ -f Web/SliP.wasm ] || { echo "npm/build.sh: Web/SliP.wasm missing — run WASM/build.sh first" >&2; exit 1; }

cp Web/SliP.js Web/SliP.wasm npm/

#	A package that cannot evaluate, or whose version does not match the engine
#	it ships, is not worth publishing.
node npm/smoke.mjs

echo "npm/ is ready. Publish with:  npm publish npm/"
