import { describe, it, expect } from 'vitest';
import { parseReleaseAssets, type GitHubAsset } from './release-assets';

function asset(name: string): GitHubAsset {
  return { name, browser_download_url: `https://example.com/${name}` };
}

const TYPICAL_ASSETS: GitHubAsset[] = [
  asset('wiRedPanda-4.5.0-Windows-x86_64.zip'),
  asset('wiRedPanda-4.5.0-Windows-arm64.zip'),
  asset('wiRedPanda-4.5.0-Ubuntu-x86_64.AppImage'),
  asset('wiRedPanda-4.5.0-Linux-arm64.AppImage'),
  asset('wiRedPanda-4.5.0-macOS-universal.dmg'),
  asset('wiRedPanda-4.5.0-source.tar.gz'),
];

describe('parseReleaseAssets', () => {
  it('finds all assets in a typical release', () => {
    const r = parseReleaseAssets(TYPICAL_ASSETS);
    expect(r.windowsX64?.name).toBe('wiRedPanda-4.5.0-Windows-x86_64.zip');
    expect(r.windowsArm64?.name).toBe('wiRedPanda-4.5.0-Windows-arm64.zip');
    expect(r.linuxX64?.name).toBe('wiRedPanda-4.5.0-Ubuntu-x86_64.AppImage');
    expect(r.linuxArm64?.name).toBe('wiRedPanda-4.5.0-Linux-arm64.AppImage');
    expect(r.macosUniversal?.name).toBe('wiRedPanda-4.5.0-macOS-universal.dmg');
  });

  describe('Windows', () => {
    it('matches .zip asset', () => {
      const r = parseReleaseAssets([asset('WiRedPanda-Windows-x86_64.zip')]);
      expect(r.windowsX64).toBeDefined();
    });

    it('matches .exe asset', () => {
      const r = parseReleaseAssets([asset('WiRedPanda-Windows-x86_64-setup.exe')]);
      expect(r.windowsX64).toBeDefined();
    });

    it('does not match .tar.gz', () => {
      const r = parseReleaseAssets([asset('WiRedPanda-Windows-x86_64.tar.gz')]);
      expect(r.windowsX64).toBeUndefined();
    });

    it('does not match a Linux asset as Windows', () => {
      const r = parseReleaseAssets([asset('WiRedPanda-Linux-x86_64.AppImage')]);
      expect(r.windowsX64).toBeUndefined();
    });

    it('matches arm64 separately from x86_64', () => {
      const assets = [asset('App-Windows-x86_64.zip'), asset('App-Windows-arm64.zip')];
      const r = parseReleaseAssets(assets);
      expect(r.windowsX64?.name).toContain('x86_64');
      expect(r.windowsArm64?.name).toContain('arm64');
    });
  });

  describe('Linux', () => {
    it('matches Ubuntu-named AppImage', () => {
      const r = parseReleaseAssets([asset('App-Ubuntu-x86_64.AppImage')]);
      expect(r.linuxX64).toBeDefined();
    });

    it('matches generic Linux AppImage', () => {
      const r = parseReleaseAssets([asset('App-Linux-x86_64.AppImage')]);
      expect(r.linuxX64).toBeDefined();
    });

    it('does not match .deb as AppImage', () => {
      const r = parseReleaseAssets([asset('App-Linux-x86_64.deb')]);
      expect(r.linuxX64).toBeUndefined();
    });

    it('does not match Windows asset as Linux', () => {
      const r = parseReleaseAssets([asset('App-Windows-x86_64.zip')]);
      expect(r.linuxX64).toBeUndefined();
    });

    it('matches arm64 Linux separately', () => {
      const assets = [asset('App-Ubuntu-x86_64.AppImage'), asset('App-Linux-arm64.AppImage')];
      const r = parseReleaseAssets(assets);
      expect(r.linuxX64?.name).toContain('x86_64');
      expect(r.linuxArm64?.name).toContain('arm64');
    });
  });

  describe('macOS', () => {
    it('matches macOS .dmg', () => {
      const r = parseReleaseAssets([asset('App-macOS-universal.dmg')]);
      expect(r.macosUniversal).toBeDefined();
    });

    it('does not match non-dmg macOS files', () => {
      const r = parseReleaseAssets([asset('App-macOS-universal.zip')]);
      expect(r.macosUniversal).toBeUndefined();
    });

    it('does not match a Linux .dmg (hypothetical)', () => {
      const r = parseReleaseAssets([asset('App-Linux-x86_64.dmg')]);
      expect(r.macosUniversal).toBeUndefined();
    });
  });

  describe('empty / missing assets', () => {
    it('returns all undefined for empty asset list', () => {
      const r = parseReleaseAssets([]);
      expect(r.windowsX64).toBeUndefined();
      expect(r.windowsArm64).toBeUndefined();
      expect(r.linuxX64).toBeUndefined();
      expect(r.linuxArm64).toBeUndefined();
      expect(r.macosUniversal).toBeUndefined();
    });

    it('returns undefined for a platform with no matching asset', () => {
      const r = parseReleaseAssets([asset('wiRedPanda-4.5.0-Windows-x86_64.zip')]);
      expect(r.macosUniversal).toBeUndefined();
      expect(r.linuxX64).toBeUndefined();
    });

    it('ignores source archives and checksums', () => {
      const assets = [
        asset('wiRedPanda-4.5.0-source.tar.gz'),
        asset('wiRedPanda-4.5.0.sha256'),
        asset('checksums.txt'),
      ];
      const r = parseReleaseAssets(assets);
      expect(r.windowsX64).toBeUndefined();
      expect(r.linuxX64).toBeUndefined();
      expect(r.macosUniversal).toBeUndefined();
    });
  });

  it('download URLs are propagated correctly', () => {
    const r = parseReleaseAssets(TYPICAL_ASSETS);
    expect(r.windowsX64?.browser_download_url).toBe(
      'https://example.com/wiRedPanda-4.5.0-Windows-x86_64.zip'
    );
  });
});
