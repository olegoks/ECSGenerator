# unpack-project.ps1
# Восстанавливает файлы проекта из дампа, созданного dump-project.ps1.
# По умолчанию распаковывает в ту же папку, где лежит сам скрипт.
#
# Примеры:
#   .\unpack-project.ps1
#   .\unpack-project.ps1 -Force
#   .\unpack-project.ps1 -DumpFile C:\backup\project_dump.xml -DestRoot C:\restore -Force
#   .\unpack-project.ps1 -StripTrailingNewline
param(
    [string]$DumpFile,
    [string]$DestRoot,
    [switch]$Force,
    [switch]$StripTrailingNewline
)

# Папка скрипта (с фолбэком, если $PSScriptRoot пуст)
$scriptDir = if ($PSScriptRoot) { $PSScriptRoot } else { (Get-Location).Path }

if (-not $DumpFile) { $DumpFile = Join-Path $scriptDir "project_dump.xml" }
if (-not $DestRoot) { $DestRoot = $scriptDir }

if (-not (Test-Path -LiteralPath $DumpFile)) {
    Write-Error "Dump file not found: $DumpFile"
    exit 1
}
$DumpFile = (Resolve-Path -LiteralPath $DumpFile).Path

if (-not (Test-Path -LiteralPath $DestRoot)) {
    New-Item -ItemType Directory -Path $DestRoot -Force | Out-Null
}
$DestRoot = (Resolve-Path -LiteralPath $DestRoot).Path
$destRootNormalized = [IO.Path]::GetFullPath($DestRoot).TrimEnd(
    [IO.Path]::DirectorySeparatorChar
) + [IO.Path]::DirectorySeparatorChar

# Дамп не должен перезаписать сам себя (на случай -Force)
$dumpFileNormalized = [IO.Path]::GetFullPath($DumpFile)

$text = [System.IO.File]::ReadAllText($DumpFile, [System.Text.Encoding]::UTF8)

$pattern = '(?s)<file path="([^"]+)">\r?\n<content>\r?\n(.*?)</content>\r?\n</file>'
$matches = [regex]::Matches($text, $pattern)

Write-Host "Script dir:  $scriptDir"
Write-Host "Dump:        $DumpFile"
Write-Host "Destination: $DestRoot"
Write-Host "Entries:     $($matches.Count)"
Write-Host ""

$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
$written = 0
$skipped = 0
$totalBytes = 0

foreach ($m in $matches) {
    $relPath = $m.Groups[1].Value
    $content = $m.Groups[2].Value

    if ($StripTrailingNewline) {
        if ($content.EndsWith("`r`n")) {
            $content = $content.Substring(0, $content.Length - 2)
        } elseif ($content.EndsWith("`n")) {
            $content = $content.Substring(0, $content.Length - 1)
        }
    }

    $relativeFsPath     = $relPath -replace '/', [IO.Path]::DirectorySeparatorChar
    $fullPath           = Join-Path $DestRoot $relativeFsPath
    $fullPathNormalized = [IO.Path]::GetFullPath($fullPath)

    # Защита от path traversal
    if (-not $fullPathNormalized.StartsWith(
            $destRootNormalized,
            [System.StringComparison]::OrdinalIgnoreCase)) {
        Write-Warning "Skipping (path traversal): $relPath"
        $skipped++
        continue
    }

    # Не перезаписываем сам дамп
    if ([string]::Equals($fullPathNormalized, $dumpFileNormalized,
            [System.StringComparison]::OrdinalIgnoreCase)) {
        Write-Warning "Skipping (would overwrite dump file): $relPath"
        $skipped++
        continue
    }

    if ((Test-Path -LiteralPath $fullPathNormalized) -and -not $Force) {
        Write-Warning "Skipping (exists, use -Force): $relPath"
        $skipped++
        continue
    }

    $parent = [IO.Path]::GetDirectoryName($fullPathNormalized)
    if ($parent -and -not (Test-Path -LiteralPath $parent)) {
        New-Item -ItemType Directory -Path $parent -Force | Out-Null
    }

    [System.IO.File]::WriteAllText($fullPathNormalized, $content, $utf8NoBom)
    $bytes = [System.Text.Encoding]::UTF8.GetByteCount($content)
    $totalBytes += $bytes
    $written++

    Write-Host ("[{0}/{1}] {2}  ({3} KB)" -f `
        $written, $matches.Count, $relPath, [math]::Round($bytes / 1KB, 1))
}

Write-Host ""
Write-Host "Done."
Write-Host "Written: $written   Skipped: $skipped"
Write-Host "Total:   $([math]::Round($totalBytes / 1KB, 1)) KB"
Write-Host "Output:  $DestRoot"