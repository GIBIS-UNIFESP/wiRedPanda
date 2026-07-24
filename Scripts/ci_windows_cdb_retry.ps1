# Copyright 2015 - 2026, GIBIS-Unifesp and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Finds tests CTest reported as failed in the given build directory's
# LastTestsFailed.log and re-runs each live under cdb, retrying up to a
# bounded number of times, to capture a full dump at the moment of a real
# crash. This exists instead of passive WER LocalDumps because WER can
# never see a QTest crash on Windows: QTest installs its own
# SetUnhandledExceptionFilter handler (windowsFaultHandler, in
# qtestcrashhandler_win.cpp) which always returns EXCEPTION_EXECUTE_HANDLER
# after printing its own diagnostics, so the exception never reaches the
# OS's true "unhandled exception" path that WER hooks into (confirmed
# directly against Qt's own source, not assumed). A debugger attached via
# cdb sees the exception as a first-chance debug event *before* that
# in-process filter runs, so it isn't fooled the same way. Mirrors
# ci_macos_lldb_retry.sh's bounded-retry design for the same reason lldb
# is used there: no passive mechanism is viable, so a live debugger is the
# only option, and a single retry has weak odds against a rare,
# timing-dependent crash.
# Called only from CI's `if: failure()` steps -- see .github/workflows/build.yml.

param(
    [Parameter(Mandatory = $true)][string]$BuildDir,
    [Parameter(Mandatory = $true)][string]$TestBinary,
    [Parameter(Mandatory = $true)][string]$OutDir,
    [int]$MaxAttempts = 20
)

$LogFile = Join-Path $BuildDir "Testing\Temporary\LastTestsFailed.log"
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

if (-not (Test-Path $LogFile)) {
    Write-Host "No $LogFile -- nothing failed."
    exit 0
}

$cdb = Get-Command cdb.exe -ErrorAction SilentlyContinue
if (-not $cdb) {
    Write-Host "cdb.exe not found -- cannot retry failed tests."
    exit 0
}

# LastTestsFailed.log lines are "<TestCount>:<Name>" (confirmed against
# CMake's own cmCTestTestHandler.cxx, LogFailedTests()).
$failedTests = Get-Content $LogFile | ForEach-Object {
    if ($_ -match '^\d+:(.+)$') { $matches[1] }
} | Sort-Object -Unique

foreach ($TestName in $failedTests) {
    Write-Host "=== Failed test: $TestName -- retrying up to $MaxAttempts times under cdb ==="
    $reproduced = $false
    for ($attempt = 1; $attempt -le $MaxAttempts; $attempt++) {
        $dumpFile = Join-Path $OutDir "$TestName.attempt$attempt.dmp"
        $outFile = Join-Path $OutDir "$TestName.attempt$attempt.txt"
        # -g/-G: don't stop at the initial create-process / final exit
        # breakpoints, only at a real exception (access violation breaks on
        # first chance by default). "g" in the command list runs the
        # process forward to that next stop; .dump then captures it.
        & $cdb.Source -g -G -c "g; .dump /ma $dumpFile; q" $TestBinary $TestName *> $outFile
        if ((Test-Path $dumpFile) -and ((Get-Item $dumpFile).Length -gt 0)) {
            Write-Host "Reproduced on attempt $attempt -- dump at $dumpFile"
            $reproduced = $true
            break
        }
        Remove-Item -ErrorAction SilentlyContinue $dumpFile, $outFile # didn't crash -- don't keep a useless clean-run log
    }
    if (-not $reproduced) {
        "Did not reproduce within $MaxAttempts attempts for $TestName." |
            Tee-Object -FilePath (Join-Path $OutDir "$TestName.not-reproduced.txt")
    }
}
