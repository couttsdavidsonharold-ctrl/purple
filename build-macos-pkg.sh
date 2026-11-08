#!/bin/bash
# Builds an installer .pkg for macOS (VST3 + AU, plus AAX if built).
set -euo pipefail
cd "$(dirname "$0")/.."
ART=build/PURPLE_artefacts/Release
STAGE=installer/stage
OUT=installer/Output
rm -rf "$STAGE" "$OUT"; mkdir -p "$STAGE/Library/Audio/Plug-Ins/VST3" "$STAGE/Library/Audio/Plug-Ins/Components" "$OUT"

cp -R "$ART/VST3/PURPLE by Hdax.vst3"        "$STAGE/Library/Audio/Plug-Ins/VST3/"
cp -R "$ART/AU/PURPLE by Hdax.component"     "$STAGE/Library/Audio/Plug-Ins/Components/"
if [ -d "$ART/AAX/PURPLE by Hdax.aaxplugin" ]; then
  mkdir -p "$STAGE/Library/Application Support/Avid/Audio/Plug-Ins"
  cp -R "$ART/AAX/PURPLE by Hdax.aaxplugin" "$STAGE/Library/Application Support/Avid/Audio/Plug-Ins/"
fi

# Optional signing: set SIGN_ID="Developer ID Installer: Your Name (TEAMID)"
SIGN=()
[ -n "${SIGN_ID:-}" ] && SIGN=(--sign "$SIGN_ID")

pkgbuild --root "$STAGE" --identifier com.hdax.purple --version 0.1.0 \
         --install-location / "${SIGN[@]}" "$OUT/PURPLE-by-Hdax-0.1.0-macOS.pkg"
echo "Created $OUT/PURPLE-by-Hdax-0.1.0-macOS.pkg"
