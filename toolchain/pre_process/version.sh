#!/bin/bash

SCRIPTDIR=$(dirname "$BASH_SOURCE")
source $SCRIPTDIR/../MaxOS.sh

msg "Incrementing build count"
# If the buildCount file doesn't exist, create it
if [ ! -f ".buildCount" ]; then
  echo "0" > .buildCount
fi

# Increment the buildCount by 1
new_value=$(($(cat .buildCount) + 1))
echo "$new_value" > .buildCount

msg "Writing version header"
# Version data
MAJOR_VERSION="0"
MINOR_VERSION="3"
VERSION_NAME="Development Version"
BUILD_NUMBER=$(cat .buildCount)
BUILD_YEAR="$(date +'%-Y')"
BUILD_MONTH="$(date +'%-m')"
BUILD_DAY="$(date +'%-d')"
BUILD_HOUR="$(date +'%-H')"
BUILD_MINUTE="$(date +'%-M')"
BUILD_SECOND="$(date +'%-S')"
GIT_REVISION="$(git rev-parse --short HEAD)"
GIT_BRANCH="$(git rev-parse --abbrev-ref HEAD)"
GIT_COMMIT="$(git rev-list --count HEAD)"
GIT_AUTHOR="$(git log -1 --pretty=format:'%an')"

# Make the output file
OUTPUT_FILE="${SCRIPTDIR}/../../kernel/include/common/version.h.tmp"

# If we are forcing it then remove the tmp option
if [ "$1" == "--force" ]; then
  OUTPUT_FILE="${SCRIPTDIR}/../../kernel/include/common/version.h"
fi

# Write the version header
cat > "${OUTPUT_FILE}" << EOF
/**
 * @file version.h
 * @brief Defines version information for MaxOS
 *
 * @date ${BUILD_YEAR}-${BUILD_MONTH}-${BUILD_DAY}
 * @author Max Tyson
 */
#ifndef MAX_OS_COMMON_VERSION_H
#define MAX_OS_COMMON_VERSION_H


#include <stdint.h>
#include <common/string.h>
#include <common/time.h>

namespace MaxOS{

    namespace common
    {

        typedef const char*              vstring;

       const uint8_t MAJOR_VERSION  = ${MAJOR_VERSION};
       const uint8_t MINOR_VERSION  = ${MINOR_VERSION};
       const vstring VERSION_STRING  = "${MAJOR_VERSION}.${MINOR_VERSION}";
       const vstring VERSION_NAME    = "${VERSION_NAME}";
       const int BUILD_NUMBER       = ${BUILD_NUMBER};
       const Time BUILD_DATE        = {${BUILD_YEAR}, ${BUILD_MONTH}, ${BUILD_DAY}, ${BUILD_HOUR}, ${BUILD_MINUTE}, ${BUILD_SECOND}};
       const vstring GIT_REVISION    = "${GIT_REVISION}";
       const vstring GIT_BRANCH      = "${GIT_BRANCH}";
       const int GIT_COMMIT         = ${GIT_COMMIT};
       const vstring GIT_AUTHOR      = "${GIT_AUTHOR}";

    }

}

#endif //MAX_OS_VERSION_H
EOF