#!/bin/bash

SCRIPTDIR=$(dirname "$BASH_SOURCE")
source $SCRIPTDIR/../MaxOS.sh

# If the buildCount file doesn't exist, create it
if [ ! -f ".buildCount" ]; then
  echo "0" > .buildCount
fi

# Increment the buildCount by 1
new_value=$(($(cat .buildCount) + 1))
echo "$new_value" > .buildCount
msg "New build count: $new_value"

msg "Writing version header"
# Version data
MAJOR_VERSION="0"
MINOR_VERSION="3"
VERSION_NAME="Kernel Focus"
BUILD_NUMBER=$(cat .buildCount)
BUILD_YEAR="$(date +'%-Y')"
BUILD_MONTH="$(date +'%-m')"
BUILD_DAY="$(date +'%-d')"
BUILD_HOUR="$(date +'%-H')"
BUILD_MINUTE="$(date +'%-M')"
BUILD_SECOND="$(date +'%-S')"
BUILD_TYPE="Debug" #/release
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
 * @date 2025-11-1
 * @author Max Tyson
 */
#ifndef MAX_OS_COMMON_VERSION_H
#define MAX_OS_COMMON_VERSION_H


#include <stdint.h>
#include <common/string.h>
#include <common/time.h>

namespace MaxOS {

	namespace common {

		/**
		 * @typedef vstring
		 * @brief A type alias for a const char* string
		 */
		typedef const char* vstring;

		const uint8_t   MAJOR_VERSION = ${MAJOR_VERSION};                                                                	///< The official release version
		const uint8_t   MINOR_VERSION = ${MINOR_VERSION};                                                                	///< The significant feature update version
		const vstring   VERSION_STRING = "${MAJOR_VERSION}.${MINOR_VERSION}";                                                           	///< A c style string representing the MAJOR.MINOR version
		const vstring   VERSION_NAME = "${VERSION_NAME}";                                             	///< The human readable version name
		const int       BUILD_NUMBER = ${BUILD_NUMBER};                                                                   ///< How many times the code has been built on this device
		const time_t    BUILD_DATE = {${BUILD_YEAR}, ${BUILD_MONTH}, ${BUILD_DAY}, ${BUILD_HOUR}, ${BUILD_MINUTE}, ${BUILD_SECOND}};     ///< The build date and time
		const vstring   BUILD_TYPE = "${BUILD_TYPE}";                                                            	///< The build type (Debug/Release)
		const vstring   GIT_REVISION = "${GIT_REVISION}";                                                          ///< The git revision hash
		const vstring   GIT_BRANCH = "${GIT_BRANCH}";                                              ///< The name of the git branch that was built
		const int       GIT_COMMIT = ${GIT_COMMIT};                                                                   ///< The commit number in the git repository that this build was made from
		const vstring   GIT_AUTHOR = "${GIT_AUTHOR}";                                                           ///< Who authored the build of this version

	}

}

#endif //MAX_OS_VERSION_H


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

        /**
         * @typedef vstring
         * @brief A type alias for a const char* string
         */
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

# Copy the version to the doxygen file (replace PROJECT_NUMBER         = x.x with the VERSION_STRING value)
sed -i.bak "s/PROJECT_NUMBER         = .*/PROJECT_NUMBER         = ${MAJOR_VERSION}.${MINOR_VERSION}/" "${SCRIPTDIR}/../../docs/doxy/Doxyfile"
rm "${SCRIPTDIR}/../../docs/Doxyfile.bak"