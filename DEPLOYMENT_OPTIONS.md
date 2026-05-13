# wiRedPanda Deployment Options

## Current deployments

Portable ZIP (Windows), AppImage (Linux), DMG (macOS). All three are manual-download artifacts; users redownload to update.

## Rollout status

Packaging manifests for all 11 `planned` channels live under `Packaging/`. Nine of them — every channel except Nix and Flathub — additionally have a matching publish workflow under `.github/workflows/publish-*.yml`, gated by a per-channel repository variable (`PUBLISH_<CHANNEL>_ENABLED=true`) and dormant until the variable plus the channel's secret are set. Nix and Flathub have no publish workflow by design: each is a one-time upstream PR (into `nixpkgs` / `flathub/flathub`), and once merged the upstream bot (`nixpkgs-update`, `flatpak-external-data-checker`) maintains it independently of this repo. Activating any channel requires the corresponding publisher account / repo / GPG key, which is human-only work.

## Comparison

### Windows

| Option | Status | Prerequisites | Effort (setup / ongoing) | Cost | Auto-update | Edu reach |
|---|---|---|---|---|---|---|
| Portable ZIP | shipping | none | S / manual | $0 | no | med |
| MSI / MSIX | future | code-sign cert | M / manual | $65–1000/yr\* | partial† | high |
| Chocolatey | planned | community PR‡ | S / manual | $0 | yes (choco) | low |
| Scoop | planned | bucket PR‡ | S / auto | $0 | yes (scoop) | low |
| WinGet | planned | `winget-pkgs` PR‡ | S / auto | $0 | yes (winget) | high |
| Microsoft Store | planned | Partner Center + MSIX | L / manual | $0§ | yes | high |

### Linux

| Option | Status | Prerequisites | Effort (setup / ongoing) | Cost | Auto-update | Edu reach |
|---|---|---|---|---|---|---|
| AppImage | shipping | none | S / manual | $0 | no¶ | med |
| Flatpak / Flathub | planned | Flathub review | M / auto | $0 | yes | high |
| Snap | planned | Snapcraft account | M / auto | $0 | yes | high |
| `.deb` PPA | planned | Launchpad + GPG | M / manual | $0 | yes (apt) | med |
| `.rpm` COPR | planned | Fedora account | M / manual | $0 | yes (dnf) | low |
| AUR | planned | AUR account | S / manual | $0 | yes (helper) | low |
| Nix | planned | `nixpkgs` PR‡ | S / manual | $0 | yes (nix) | low |

### macOS

| Option | Status | Prerequisites | Effort (setup / ongoing) | Cost | Auto-update | Edu reach |
|---|---|---|---|---|---|---|
| DMG | shipping | none\*\* | S / manual | $0\*\* | no | med |
| Notarized + signed | future | Apple Dev ID | M / auto-after-setup | $99/yr | n/a†† | high |
| `.pkg` installer | future | Apple Dev ID | M / manual | $99/yr | no | high |
| Homebrew Cask | planned | `homebrew-cask` PR‡ | S / auto | $0 | yes (brew) | high |
| Mac App Store | skip | Dev ID + sandbox refactor | L / manual | $99/yr | yes | med |

## Highest ROI picks

For an educational Qt app, the cheap-win channels surface clearly from the tables: **WinGet** (Windows), **Flathub** (Linux), and **Homebrew Cask** (macOS). All three are $0, low setup effort, support auto-updates, and reach the audiences students and teachers actually use. They dramatically improve "how do I install this" once a manifest is in place.

## Legend & notes

- **Status**: `shipping` = already in the release pipeline; `planned` = committed for the next rollout; `future` = not yet built and not committed; `skip` = considered and ruled out.
- **Effort (setup)**: `S` ≈ days, `M` ≈ a week, `L` ≈ multi-week.
- **Effort (ongoing)**: `auto` = community/CI bumps the manifest on a new release tag; `manual` = maintainer must update each release.
- \* OV (Organization Validation) certs from third-party CAs range from ~$65/yr (SSL.com) to ~$700/yr (DigiCert) — the CA verifies the org exists. EV (Extended Validation) certs are ~$1000/yr; stricter vetting + hardware-token-only private key, but skip Windows SmartScreen warnings on day one. Azure Trusted Signing is Microsoft's managed alternative at ~$120/yr.
- † MSIX supports auto-update; bare MSI does not.
- ‡ Community channels require upstream PR review on first submission; subsequent updates are usually automated.
- § Microsoft Store costs $0 end-to-end: Partner Center registration is free for all developers (new onboarding flow), and the Store signs MSIX packages on submission so no code-sign cert is needed.
- ¶ AppImage has no built-in auto-update; AppImageUpdate exists but is not wired in today.
- \*\* DMG row assumes today's unsigned artifact. If signing/notarization is already in place, treat **Notarized + signed** as `shipping` and update DMG's prerequisites to `Apple Dev ID`, cost to `$99/yr`.
- †† Notarization is a foundational requirement, not a delivery channel — it gates DMG, `.pkg`, Cask, and Store quality.
- **F-Droid** is omitted from the tables — it targets Android / Linux mobile and remains `skip` unless a mobile build is ever shipped.

