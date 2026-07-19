import type { APIRoute } from 'astro';
import releaseData from '../data/latest-release.json';

// Re-serves the release data the site already builds download.astro from, as a public
// endpoint. wiRedPanda's UpdateChecker polls this instead of the GitHub API directly, so
// the per-user rate limit only ever applies to the CI job that regenerates the source file.
export const GET: APIRoute = () => {
  return new Response(JSON.stringify(releaseData), {
    headers: { 'Content-Type': 'application/json' },
  });
};
