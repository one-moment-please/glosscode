#!/bin/sh
# Prints the release notes for a version: its section of CHANGELOG.md plus the download notes.
# The release workflow uses this as the GitHub release's description, which the About window shows.
#
#   scripts/release-notes.sh [version]     (default: the VERSION file)
set -eu
src=$(cd "$(dirname "$0")/.." && pwd)
version=${1:-$(tr -d ' \r\n' < "$src/VERSION")}

section=$(awk -v v="$version" '
    index($0, "## [" v "]") == 1 { on = 1; next }
    on && /^## \[/ { exit }
    on { print }' "$src/CHANGELOG.md")
[ -n "$section" ] || { echo "CHANGELOG.md has no \"## [$version]\" section" >&2; exit 1; }

printf '%s\n' "$section" | sed -e '/./,$!d'
cat <<TXT

### Which file to download
| | |
|---|---|
| Windows, installed | \`GlossCode-$version-windows-x64-setup.exe\` |
| Windows, portable | \`GlossCode-$version-windows-x64-portable.zip\` |
| Debian, Ubuntu | \`GlossCode-$version-linux-amd64.deb\` |
| Fedora, openSUSE | \`GlossCode-$version-linux-x86_64.rpm\` |
| Linux, portable | \`GlossCode-$version-linux-x86_64-portable.tar.gz\` |
| Sample programs | \`GlossCode-$version-examples.zip\` |

The Windows files aren't code-signed yet, so Windows may show "Windows protected your PC" the first time. Choose **More info**, then **Run anyway**. \`SHA256SUMS.txt\` lists the checksum of every file.
TXT
