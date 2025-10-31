#!/bin/bash
source ./MaxOS.sh

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
doxygen Doxyfile || { fail "Doxygen generation failed"; exit 1; }

msg "Documentation generated successfully!"