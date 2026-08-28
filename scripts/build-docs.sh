#!/usr/bin/env bash
# Generates the Doxygen API docs into public/api, mirroring what
# .github/workflows/site.yml does before it builds the site.
#
# Without this, public/api only ever exists in CI, so the header's "API Docs"
# link 404s in `npm run dev` / `npm run preview` for every developer.
#
# Deliberately not wired into predev: Doxygen takes ~10s, which is too slow to
# pay on every dev-server start. Run it once, then it persists in public/.
set -euo pipefail

cd "$(dirname "$0")/.."

# The stylesheets Doxyfile references via HTML_EXTRA_STYLESHEET. CI gets these
# from `submodules: true` on its checkout.
echo "==> submodule: doxygen-awesome-css"
git submodule update --init doxygen-awesome-css

# Doxyfile's INPUT is master-src/App and master-src/MCP/Server, so the C++ sources
# have to appear at that path -- CI does it with a second actions/checkout of
# master into master-src/. This branch is a worktree of the same repository, so
# master is already in the shared object store and no clone is needed.
#
# Removed first because `git worktree add` refuses an existing directory
# ("fatal: 'master-src' already exists"), which would break every run after the
# first. --detach avoids "already checked out" if a sibling worktree holds master.
echo "==> master-src: worktree at origin/master"
git fetch origin master
git worktree remove --force master-src 2>/dev/null || true
rm -rf master-src
git worktree add --detach master-src origin/master

echo "==> doxygen"
doxygen Doxyfile

echo "==> public/api"
rm -rf public/api
cp -r docs/html public/api

echo "Done. public/api now holds $(find public/api -type f | wc -l) files."
