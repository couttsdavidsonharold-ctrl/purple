#!/bin/bash
# Builds an installer .pkg for macOS (VST3 + AU, plus AAX if built).
set -eu
cd "$(dirname "$0")"
ART=build/PURPLE_artefacts/Release
STAGE=stage
OUT=Output
rm -rf "$STAGE" "$OUT"
mkdir -p "$STAGE/Library/Audio/Plug-Ins/VST3" "$STAGE/Library/Audio/Plug-Ins/Components" "$OUT"

cp -R "$ART/VST3/PURPLE by Hdax.vst3"    "$STAGE/Library/Audio/Plug-Ins/VST3/"
cp -R "$ART/AU/PURPLE by Hdax.component" "$STAGE/Library/Audio/Plug-Ins/Components/"
if [ -d "$ART/AAX/PURPLE by Hdax.aaxplugin" ]; then
  mkdir -p "$STAGE/Library/Application Support/Avid/Audio/Plug-Ins"
  cp -R "$ART/AAX/PURPLE by Hdax.aaxplugin" "$STAGE/Library/Application Support/Avid/Audio/Plug-Ins/"
fi

PKG="$OUT/PURPLE-by-Hdax-0.1.0-macOS.pkg"
if [ -n "${SIGN_ID:-}" ]; then
  pkgbuild --root "$STAGE" --identifier com.hdax.purple --version 0.1.0 --install-location / --sign "$SIGN_ID" "$PKG"
else
  pkgbuild --root "$STAGE" --identifier com.hdax.purple --version 0.1.0 --install-location / "$PKG"
fi
echo "Created $PKG"
