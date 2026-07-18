#!/usr/bin/env node
// Fetches the latest wiRedPanda GitHub release, resolves per-platform download
// URLs via the same parseReleaseAssets logic the site itself uses, and writes
// them to src/data/latest-release.json. Run by .github/workflows/
// update-release-data.yml (on master) whenever a release is published; the
// site build reads the committed file directly and never calls the GitHub
// API itself, so an outage here can't block unrelated site deploys.

import { writeFile } from 'node:fs/promises';
import { parseReleaseAssets } from '../src/lib/release-assets.ts';

const OWNER = 'GIBIS-UNIFESP';
const REPO = 'wiRedPanda';
const OUTPUT_PATH = new URL('../src/data/latest-release.json', import.meta.url);

async function main() {
  const headers = { Accept: 'application/vnd.github+json' };
  if (process.env.GITHUB_TOKEN) {
    headers.Authorization = `Bearer ${process.env.GITHUB_TOKEN}`;
  }

  const response = await fetch(`https://api.github.com/repos/${OWNER}/${REPO}/releases/latest`, {
    headers,
  });
  if (!response.ok) {
    throw new Error(`GitHub API request failed: ${response.status} ${response.statusText}`);
  }

  const release = await response.json();
  const parsed = parseReleaseAssets(release.assets ?? []);

  const data = {
    version: release.tag_name,
    windowsX64: parsed.windowsX64?.browser_download_url ?? null,
    windowsArm64: parsed.windowsArm64?.browser_download_url ?? null,
    linuxX64: parsed.linuxX64?.browser_download_url ?? null,
    linuxArm64: parsed.linuxArm64?.browser_download_url ?? null,
    macosUniversal: parsed.macosUniversal?.browser_download_url ?? null,
  };

  await writeFile(OUTPUT_PATH, JSON.stringify(data, null, 2) + '\n');
  console.log(`Wrote ${OUTPUT_PATH.pathname}:`);
  console.log(data);
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
