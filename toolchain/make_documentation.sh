#!/bin/bash
source ./MaxOS.sh

# Parse arguments
ENFORCE=0

for arg in "$@"; do
    case "$arg" in
        --enforce)
            ENFORCE=1
            ;;
        *)
            echo "Unknown argument: $arg"
            echo "Usage: $0 [--enforce]"
            exit 2
            ;;
    esac
done


# Go to the project root directory
cd ../docs/doxy

# Install Required Packages
sudo apt-get install doxygen

# Download required extensions
msg "Downloading extensions..."
ls doxygen-awesome.css                  || wget https://raw.githubusercontent.com/jothepro/doxygen-awesome-css/main/doxygen-awesome.css
ls doxygen-awesome-darkmode-toggle.js   || wget https://raw.githubusercontent.com/jothepro/doxygen-awesome-css/main/doxygen-awesome-darkmode-toggle.js
ls doxygen-awesome-paragraph-link.js    || wget https://raw.githubusercontent.com/jothepro/doxygen-awesome-css/main/doxygen-awesome-paragraph-link.js
ls doxygen-awesome-interactive-toc.js   || wget https://raw.githubusercontent.com/jothepro/doxygen-awesome-css/main/doxygen-awesome-interactive-toc.js

msg "Setting up extensions..."

# Make a dummy header.html file
touch header.html

# Create the header template
doxygen -w html header.html delete_me.html delete_me.css

# Find head tag in script and add the extensions
sed -i '/<\/head>/i <script type="text/javascript" src="$relpath^doxygen-awesome-darkmode-toggle.js"></script>\n<script type="text/javascript" src="$relpath^doxygen-awesome-paragraph-link.js"></script>\n<script type="text/javascript" src="$relpath^doxygen-awesome-interactive-toc.js"></script>\n<script type="text/javascript">\n    DoxygenAwesomeDarkModeToggle.init()\n    DoxygenAwesomeParagraphLink.init()\n    DoxygenAwesomeInteractiveToc.init()\n</script>' header.html

# Generate the documentation
msg "Generating documentation..."

DOXYFILE="Doxyfile"
DOXYFILE_TMP=""

if [ "$ENFORCE" -eq 1 ]; then
    msg "Enforcing Doxygen warnings as errors"

    DOXYFILE_TMP=$(mktemp "$PWD/.Doxyfile.XXXXXX") || fail "Could not create temporary Doxyfile"

    # Override warning settings without modifying the original Doxyfile
    awk '
        /^[[:space:]]*WARNINGS[[:space:]]*=/ {
            print "WARNINGS = YES"
            warnings = 1
            next
        }
        /^[[:space:]]*WARN_AS_ERROR[[:space:]]*=/ {
            print "WARN_AS_ERROR = YES"
            enforce = 1
            next
        }
        { print }
        END {
            if (!warnings) print "WARNINGS = YES"
            if (!enforce) print "WARN_AS_ERROR = YES"
        }
    ' Doxyfile > "$DOXYFILE_TMP" || fail "Could not configure enforcement"

    DOXYFILE="$DOXYFILE_TMP"
fi

doxygen "$DOXYFILE"
DOXYGEN_STATUS=$?

[ -z "$DOXYFILE_TMP" ] || rm -f "$DOXYFILE_TMP"

if [ "$DOXYGEN_STATUS" -ne 0 ]; then
    fail "Doxygen generation failed"
    exit 1
fi

msg "Documentation generated successfully!"