#!/bin/sh

### create disk image ###############################################

echo "Creating disk image"
imagedir="/tmp/Aerolith.$$"
mkdir $imagedir
cp -R Aerolith.app $imagedir

# TODO: copy over additional files, if any
hdiutil create -ov -srcfolder $imagedir -format UDCO -volname "Aerolith" "Aerolith.dmg"
hdiutil internet-enable -yes "Aerolith.dmg"
rm -rf $imagedir
