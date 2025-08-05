# Claude Documentation Directory

This directory contains analysis reports, crash investigations, and fix tracking documentation for the wiRedPanda project.

## Directory Structure

- **Crash Analysis**: Files like `SENTRY_CRASH_ANALYSIS.md` contain detailed crash analysis from Sentry
- **Fix Tracking**: Documentation of implemented fixes and their verification
- **Issue Analysis**: Deep dives into specific bugs or feature implementations

## Naming Convention

- `SENTRY_CRASH_ANALYSIS.md` - Main Sentry crash analysis document
- `ISSUE_<ID>_<DESCRIPTION>.md` - Specific issue analysis (e.g., `ISSUE_WIREDPANDA-J_CRASH_FIX.md`)
- `FIX_<DATE>_<DESCRIPTION>.md` - Fix documentation (e.g., `FIX_2024-01-15_NULL_POINTER.md`)

## Guidelines

1. Always create new documentation when analyzing complex issues
2. Include relevant code snippets and stack traces
3. Document the root cause analysis process
4. Track verification steps for fixes
5. Reference Sentry issue IDs in filenames and content
