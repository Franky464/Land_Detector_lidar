#!/bin/bash

set -e

# work from either ArduPilot directory or above
[ -d ArduPlane ] || cd ArduPilot

export DOCS_OUTPUT_BASE=./docs

(
$DOCS_OUTPUT_BASE/build-libs.sh
$DOCS_OUTPUT_BASE/build-arduplane.sh
$DOCS_OUTPUT_BASE/build-arducopter.sh
$DOCS_OUTPUT_BASE/build-ArduPilotrover2.sh
$DOCS_OUTPUT_BASE/build-ardusub.sh
) > $DOCS_OUTPUT_BASE/build_docs.log 2>&1
