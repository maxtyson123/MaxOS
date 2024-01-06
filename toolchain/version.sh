#!/bin/bash

SCRIPTDIR=$(dirname "$BASH_SOURCE")
source $SCRIPTDIR/maxOS.sh

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
MINOR_VERSION="1"
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

# Write the version header
cat > "${SCRIPTDIR}/../kernel/include/common/version.h.tmp" << EOF
//
// This file is generated automatically by the maxOS build system.
//

#ifndef MAX_OS_COMMON_VERSION_H
#define MAX_OS_COMMON_VERSION_H


#include <stdint.h>
#include <common/string.h>
#include <common/time.h>

namespace maxOS{

    namespace common
    {

       const uint8_t MAJOR_VERSION  = ${MAJOR_VERSION};
       const uint8_t MINOR_VERSION  = ${MINOR_VERSION};
       const string VERSION_STRING  = "${MAJOR_VERSION}.${MINOR_VERSION}";
       const string VERSION_NAME    = "${VERSION_NAME}";
       const int BUILD_NUMBER       = ${BUILD_NUMBER};
       const Time BUILD_DATE        = {${BUILD_YEAR}, ${BUILD_MONTH}, ${BUILD_DAY}, ${BUILD_HOUR}, ${BUILD_MINUTE}, ${BUILD_SECOND}};
       const string GIT_REVISION    = "${GIT_REVISION}";
       const string GIT_BRANCH      = "${GIT_BRANCH}";
       const int GIT_COMMIT         = ${GIT_COMMIT};
       const string GIT_AUTHOR      = "${GIT_AUTHOR}";

    }

}

#endif //MAX_OS_VERSION_H
EOF