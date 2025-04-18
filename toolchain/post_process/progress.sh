#!/bin/bash

SCRIPTDIR=$(dirname "$BASH_SOURCE")
source $SCRIPTDIR/../MaxOS.sh

# Count how many info logs we have
TOTAL=$(grep -r "Logger::INFO" "${SCRIPTDIR}/../../kernel/src/" | wc -l)
msg "Total INFO: ${TOTAL}"

# Store in the source file
$SED_EXC -i "s/static const uint8_t s_progress_total { 100 };/static const uint8_t s_progress_total { ${TOTAL} };/" "${SCRIPTDIR}/../../kernel/include/common/logger.h"