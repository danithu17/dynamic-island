param (
    [string]$AppDir = "dist\AuraIsland"
)

$ErrorActionPreference = "Stop"

Write-Host "========================================"
Write-Host "Verifying DLL Dependencies for: $AppDir"
Write-Host "========================================"

if (-not (Test-Path $AppDir)) {
    Write-Error "Directory '$AppDir' does not exist!"
    exit 1
}

# Resolve full path
$FullAppDir = (Resolve-Path $AppDir).Path
$ExePath = Join-Path $FullAppDir "AuraIsland.exe"

if (-not (Test-Path $ExePath)) {
    Write-Error "AuraIsland.exe not found at '$ExePath'!"
    exit 1
}

# Collect all local files in the package (case-insensitive dictionary)
$LocalFiles = @{}
Get-ChildItem -Path $FullAppDir -Recurse -File | ForEach-Object {
    $LocalFiles[$_.Name.ToLower()] = $_.FullName
}

Write-Host "Found $($LocalFiles.Count) files packaged in '$AppDir'."

# Known Windows system DLL prefixes / DLLs
$SystemDlls = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
@(
    "kernel32.dll", "kernelbase.dll", "user32.dll", "gdi32.dll", "gdi32full.dll",
    "shell32.dll", "ole32.dll", "oleaut32.dll", "advapi32.dll", "ws2_32.dll",
    "crypt32.dll", "dwmapi.dll", "uxtheme.dll", "imm32.dll", "winmm.dll",
    "shlwapi.dll", "version.dll", "dbghelp.dll", "windowsapp.dll", "powrprof.dll",
    "dxva2.dll", "ntdll.dll", "credui.dll", "netapi32.dll", "secur32.dll",
    "bcrypt.dll", "rpcrt4.dll", "comctl32.dll", "comdlg32.dll", "mpr.dll",
    "dnsapi.dll", "iphlpapi.dll", "wldap32.dll", "wtsapi32.dll", "userenv.dll",
    "d3d11.dll", "dxgi.dll", "d2d1.dll", "dwrite.dll", "msvcp_win.dll",
    "ucrtbase.dll", "cfgmgr32.dll", "propsys.dll", "winhttp.dll", "setupapi.dll"
) | ForEach-Object { [void]$SystemDlls.Add($_) }

function IsSystemDll([string]$dllName) {
    $lower = $dllName.ToLower()
    if ($lower.StartsWith("api-ms-") -or $lower.StartsWith("ext-ms-")) {
        return $true
    }
    if ($SystemDlls.Contains($lower)) {
        return $true
    }
    # Check if exists in Windows\System32
    $sys32Path = Join-Path $env:windir "System32\$lower"
    if (Test-Path $sys32Path) {
        return $true
    }
    return $false
}

# Function to extract dependent DLLs using dumpbin or PE parsing
function Get-Dependencies([string]$filePath) {
    $deps = @()
    # Check if dumpbin is available
    $dumpbin = Get-Command dumpbin -ErrorAction SilentlyContinue
    if ($dumpbin) {
        $output = & dumpbin /dependents $filePath 2>$null
        $inSection = $false
        foreach ($line in $output) {
            if ($line -match "Image has the following dependencies:") {
                $inSection = $true
                continue
            }
            if ($inSection) {
                if ($line -match "^\s+([a-zA-Z0-9_\-\.]+\.dll)") {
                    $deps += $matches[1]
                } elseif ($line -match "Summary") {
                    break
                }
            }
        }
        return $deps
    }

    # Fallback PE reader for imported DLL names
    try {
        $bytes = [System.IO.File]::ReadAllBytes($filePath)
        if ($bytes.Length -lt 64) { return $deps }
        $e_lfanew = [BitConverter]::ToInt32($bytes, 0x3C)
        if ($e_lfanew + 4 -gt $bytes.Length) { return $deps }
        $peSig = [BitConverter]::ToUInt32($bytes, $e_lfanew)
        if ($peSig -ne 0x00004550) { return $deps } # 'PE\0\0'

        # Optional header magic
        $optHeaderOffset = $e_lfanew + 24
        $magic = [BitConverter]::ToUInt16($bytes, $optHeaderOffset)
        $is64 = ($magic -eq 0x20B)

        $importDirRvaOffset = if ($is64) { $optHeaderOffset + 120 } else { $optHeaderOffset + 104 }
        if ($importDirRvaOffset + 8 -gt $bytes.Length) { return $deps }
        $importRva = [BitConverter]::ToUInt32($bytes, $importDirRvaOffset)
        if ($importRva -eq 0) { return $deps }

        # Find section containing import table
        $numSections = [BitConverter]::ToUInt16($bytes, $e_lfanew + 6)
        $secTableOffset = $optHeaderOffset + (if ($is64) { 240 } else { 224 })

        $importRawOffset = 0
        for ($i = 0; $i -lt $numSections; $i++) {
            $curSec = $secTableOffset + ($i * 40)
            $vSize = [BitConverter]::ToUInt32($bytes, $curSec + 8)
            $vAddr = [BitConverter]::ToUInt32($bytes, $curSec + 12)
            $rSize = [BitConverter]::ToUInt32($bytes, $curSec + 16)
            $rAddr = [BitConverter]::ToUInt32($bytes, $curSec + 20)

            if ($importRva -ge $vAddr -and $importRva -lt ($vAddr + $vSize)) {
                $importRawOffset = $rAddr + ($importRva - $vAddr)
                break
            }
        }

        if ($importRawOffset -eq 0) { return $deps }

        # Iterate Import Descriptors (each is 20 bytes: 12=Name RVA)
        $descOffset = $importRawOffset
        while ($descOffset + 20 -le $bytes.Length) {
            $nameRva = [BitConverter]::ToUInt32($bytes, $descOffset + 12)
            if ($nameRva -eq 0) { break }

            # Find section for name RVA
            $nameRaw = 0
            for ($i = 0; $i -lt $numSections; $i++) {
                $curSec = $secTableOffset + ($i * 40)
                $vSize = [BitConverter]::ToUInt32($bytes, $curSec + 8)
                $vAddr = [BitConverter]::ToUInt32($bytes, $curSec + 12)
                $rAddr = [BitConverter]::ToUInt32($bytes, $curSec + 20)
                if ($nameRva -ge $vAddr -and $nameRva -lt ($vAddr + $vSize)) {
                    $nameRaw = $rAddr + ($nameRva - $vAddr)
                    break
                }
            }

            if ($nameRaw -ne 0 -and $nameRaw -lt $bytes.Length) {
                $strBytes = [System.Collections.Generic.List[byte]]::new()
                $idx = $nameRaw
                while ($idx -lt $bytes.Length -and $bytes[$idx] -ne 0) {
                    $strBytes.Add($bytes[$idx])
                    $idx++
                }
                $dllName = [System.Text.Encoding]::ASCII.GetString($strBytes.ToArray())
                if ($dllName.EndsWith(".dll", [System.StringComparison]::OrdinalIgnoreCase)) {
                    $deps += $dllName
                }
            }
            $descOffset += 20
        }
    } catch {
        Write-Warning "Could not parse dependencies for $filePath : $_"
    }
    return $deps
}

# Queue for recursive dependency analysis
$CheckedBinaries = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
$Queue = [System.Collections.Generic.Queue[string]]::new()

$Queue.Enqueue($ExePath)
$MissingDlls = [System.Collections.Generic.Dictionary[string, string]]::new([System.StringComparer]::OrdinalIgnoreCase)

while ($Queue.Count -gt 0) {
    $currentBinary = $Queue.Dequeue()
    if ($CheckedBinaries.Contains($currentBinary)) { continue }
    [void]$CheckedBinaries.Add($currentBinary)

    $deps = Get-Dependencies $currentBinary
    $binaryName = [System.IO.Path]::GetFileName($currentBinary)

    foreach ($dep in $deps) {
        $depLower = $dep.ToLower()
        if (IsSystemDll $depLower) {
            continue
        }

        if ($LocalFiles.ContainsKey($depLower)) {
            $localPath = $LocalFiles[$depLower]
            if (-not $CheckedBinaries.Contains($localPath)) {
                $Queue.Enqueue($localPath)
            }
        } else {
            if (-not $MissingDlls.ContainsKey($depLower)) {
                $MissingDlls[$depLower] = $binaryName
            }
        }
    }
}

Write-Host "Scanned $($CheckedBinaries.Count) PE binaries recursively."

if ($MissingDlls.Count -gt 0) {
    Write-Host ""
    Write-Host "::error::DEPENDENCY VERIFICATION FAILED! The following required DLLs are MISSING:" -ForegroundColor Red
    foreach ($kv in $MissingDlls.GetEnumerator()) {
        Write-Host "  [MISSING] $($kv.Key) (Required by: $($kv.Value))" -ForegroundColor Red
    }
    Write-Host ""
    Write-Host "Ensure windeployqt has packaged all modules and MSVC runtime DLLs are present next to the exe." -ForegroundColor Yellow
    exit 1
}

Write-Host "SUCCESS: All dependencies are accounted for (present in package or system DLL)." -ForegroundColor Green
exit 0
