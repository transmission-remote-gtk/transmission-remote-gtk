#!/usr/bin/env bash
set -eo pipefail

# Things to remember:
# 1. Change `version:` in meson project to $VERSION
# 2. Update appdata.xml with "release notes" for $VERSION (high level)
# 3. Commit changes with message "Release $VERSION"
# 4. tag new commit with $VERSION
# 5. Run this script

meson setup --wipe release-build
meson dist -C release-build
