#!/bin/bash

SCRIPTDIR=$(dirname "$BASH_SOURCE")
source $SCRIPTDIR/../MaxOS.sh

# Count how many info logs we have
TOTAL=$(grep -r "Logger::INFO" "${SCRIPTDIR}/../../kernel/src/" | wc -l)
msg "Total INFO: ${TOTAL}"

# Store in the source file
$SED_EXC -i \
  -E "s/(s_progress_total[[:space:]]*=[[:space:]]*)[0-9]+;/\1${TOTAL};/" \
  "${SCRIPTDIR}/../../kernel/src/common/logger.cpp"