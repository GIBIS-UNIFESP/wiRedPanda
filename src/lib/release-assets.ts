export interface GitHubAsset {
  name: string;
  browser_download_url: string;
}

export interface ReleaseAssets {
  windowsX64: GitHubAsset | undefined;
  windowsArm64: GitHubAsset | undefined;
  linuxX64: GitHubAsset | undefined;
  linuxArm64: GitHubAsset | undefined;
  macosUniversal: GitHubAsset | undefined;
}

function isWindowsAsset(a: GitHubAsset): boolean {
  return a.name.includes('Windows') && (a.name.endsWith('.zip') || a.name.endsWith('.exe'));
}

function isLinuxAppImage(a: GitHubAsset): boolean {
  return (a.name.includes('Ubuntu') || a.name.includes('Linux')) && a.name.endsWith('.AppImage');
}

function isMacOsDmg(a: GitHubAsset): boolean {
  return a.name.includes('macOS') && a.name.endsWith('.dmg');
}

export function parseReleaseAssets(assets: GitHubAsset[]): ReleaseAssets {
  return {
    windowsX64: assets.find((a) => isWindowsAsset(a) && a.name.includes('x86_64')),
    windowsArm64: assets.find((a) => isWindowsAsset(a) && a.name.includes('arm64')),
    linuxX64: assets.find((a) => isLinuxAppImage(a) && a.name.includes('x86_64')),
    linuxArm64: assets.find((a) => isLinuxAppImage(a) && a.name.includes('arm64')),
    macosUniversal: assets.find((a) => isMacOsDmg(a)),
  };
}
