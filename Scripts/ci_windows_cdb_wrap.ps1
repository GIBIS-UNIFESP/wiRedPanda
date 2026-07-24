# Copyright 2015 - 2026, GIBIS-Unifesp and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Wraps every ctest test invocation on Windows under cdb, via CMake's
# CROSSCOMPILING_EMULATOR target property (see CMakeLists.txt), mirroring
# Scripts/ci_macos_lldb_wrap.sh. Catches a crash live, on its actual first
# occurrence, instead of retrying a failed test after the fact: that
# approach confirmed both unreliable (macOS: a genuinely rare race never
# reproduced across 20 retry attempts, twice) and unsafe (Windows: the
# retry step had no bound on total retry time across every failed test and
# hung a CI job for hours instead of the ~10 minutes the Test step itself
# is capped at).
#
# Only active when CI=true (set automatically by GitHub Actions) -- a local
# developer build gets zero overhead, running test_wiredpanda.exe directly.

$TestArgs = $args

if ($env:CI -ne "true" -or -not (Get-Command cdb.exe -ErrorAction SilentlyContinue)) {
    & $TestArgs[0] $TestArgs[1..($TestArgs.Length - 1)]
    exit $LASTEXITCODE
}

$TestName = if ($TestArgs.Length -ge 2) { $TestArgs[1] } else { "unknown" }
$OutDir = if ($env:CI_CRASH_DIAGNOSTICS_DIR) { $env:CI_CRASH_DIAGNOSTICS_DIR } else { "crash-diagnostics" }
$DumpFile = Join-Path $env:TEMP "$TestName.$PID.dmp"
$TmpLog = Join-Path $env:TEMP "$TestName.$PID.cdb.txt"

# -g/-G: don't stop at the initial create-process / final exit breakpoints,
# only at a real exception (access violation breaks on first chance by
# default). "g" in the command list runs the process forward to that next
# stop; .dump then captures it. The test binary + its own args must be the
# final items on cdb's command line -- everything after is passed through.
$cdbArgs = @("-g", "-G", "-c", "g; .dump /ma $DumpFile; q") + $TestArgs
& cdb.exe @cdbArgs *> $TmpLog
Get-Content $TmpLog | Write-Output

if ((Test-Path $DumpFile) -and ((Get-Item $DumpFile).Length -gt 0)) {
    New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
    Copy-Item $DumpFile (Join-Path $OutDir "$TestName.$PID.dmp")
    Remove-Item -ErrorAction SilentlyContinue $DumpFile, $TmpLog
    exit 139
}

$Failed = 0
if (Select-String -Path $TmpLog -Pattern '^FAIL!' -Quiet) { $Failed = 1 }
Remove-Item -ErrorAction SilentlyContinue $TmpLog
exit $Failed
