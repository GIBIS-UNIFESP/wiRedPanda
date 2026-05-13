$ErrorActionPreference = 'Stop'

$packageName = 'wiredpanda'
$toolsDir    = "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"
$version     = '5.0.1'

$packageArgs = @{
  packageName       = $packageName
  unzipLocation     = $toolsDir
  url64bit          = "https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/download/$version/wiRedPanda-$version-Windows-x86_64-Qt6.9.3-Portable.zip"
  urlArm64          = "https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/download/$version/wiRedPanda-$version-Windows-arm64-Qt6.9.3-Portable.zip"
  checksum64        = '0000000000000000000000000000000000000000000000000000000000000000'
  checksumType64    = 'sha256'
  checksumArm64     = '0000000000000000000000000000000000000000000000000000000000000000'
  checksumTypeArm64 = 'sha256'
}

Install-ChocolateyZipPackage @packageArgs
