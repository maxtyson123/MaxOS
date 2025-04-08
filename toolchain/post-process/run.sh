#!/bin/bash

SCRIPTDIR=$(dirname "$BASH_SOURCE")
source $SCRIPTDIR/../MaxOS.sh

bash $SCRIPTDIR/version.sh
bash $SCRIPTDIR/progress.sh