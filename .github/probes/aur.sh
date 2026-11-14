#!/usr/bin/env bash
# Runs the AUR path in an Arch container (see aur-in-arch.sh). Results land in probe-out/.
# The source is an archive of HEAD (the PR tree plus these probes), rooted at the directory
# name the PKGBUILD cds into.
set -uo pipefail
mkdir -p probe-out
git archive --format=tar.gz --prefix=wiRedPanda-5.2.2/ -o probe-out/pr-source.tar.gz HEAD
docker run --rm -v "$PWD:/work" archlinux:latest bash /work/.github/probes/aur-in-arch.sh
echo "docker run exit=$?"
exit 0
