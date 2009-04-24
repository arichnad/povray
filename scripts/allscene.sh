#!/bin/sh
# This will find all of the .pov scenes in the installation,
# and render them.  It will also render the first frame of all the
# animations, but not the whole thing, because there are no common Unix
# animation utilities to convert all the files.  Customize to your liking.

# Be sure to remove complete.lst if you want to re-render the scenes.
# It is sort of a kludge because we can't simply test for the existence
# of the output files, as this would prevent us from resuming interrupted
# renderings.

mkdir allscene
echo "" >> complete.lst
find ../scenes -name "*.pov" -exec nice povray -P ../ini/allstill.ini \{\} \;