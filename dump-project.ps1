# dump-project.ps1
param(
    [string]$Root     = (Get-Location).Path,
    [string]$Output   = "project_dump.xml",
    [int]$MaxFileSizeKb = 512
)

$excludeDirs = @(
    '.git', '.vs', '.vscode', '.idea', '.continue',
    'Build', 'build', 'out', 'bin', 'obj',
    'vcpkg_installed', 'node_modules',
    'ImGui'                      # 5 MB third-party — уберите, если нужен
)

$extensions = @(
    '.hpp', '.h', '.cpp', '.cxx', '.cc', '.c', '.inl',
    '.ecs', '.ecs_module', '.ecs_package',
    '.lua', '.cmake', '.txt', '.md',
    '.json', '.yaml', '.yml', '.toml',
    '.ps1', '.sh', '.bat', '.py'
)

$Root = (Resolve-Path $Root).Path
$outputPath = Join-Path $Root $Output

function Test-BinaryFile {
    param([string]$Path)
    try {
        $fs = [System.IO.File]::OpenRead($Path)
        $buf = New-Object byte[] 1024
        $n = $fs.Read($buf, 0, $buf.Length)
        $fs.Close()
        for ($i = 0; $i -lt $n; $i++) {
            if ($buf[$i] -eq 0) { return $true }
        }
        return $false
    } catch { return $true }
}

$files = Get-ChildItem -Path $Root -Recurse -File | Where-Object {
    $rel = $_.FullName.Substring($Root.Length + 1)
    $parts = $rel.Split([IO.Path]::DirectorySeparatorChar)
    foreach ($p in $parts) {
        if ($excludeDirs -contains $p) { return $false }
    }
    if ($_.FullName -eq $outputPath) { return $false }
    if ($extensions -notcontains $_.Extension.ToLower()) { return $false }
    if ($_.Length -gt ($MaxFileSizeKb * 1024)) { return $false }
    return $true
} | Sort-Object FullName

$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
$writer = New-Object System.IO.StreamWriter($outputPath, $false, $utf8NoBom)

try {
    $writer.WriteLine("<!-- Project dump -->")
    $writer.WriteLine("<!-- Root: $Root -->")
    $writer.WriteLine("<!-- Files: $($files.Count) -->")
    $writer.WriteLine()

    $i = 0
    $totalBytes = 0

    foreach ($f in $files) {
        $i++
        $rel = $f.FullName.Substring($Root.Length + 1).Replace('\', '/')

        if (Test-BinaryFile $f.FullName) {
            Write-Host "[$i/$($files.Count)] SKIP (binary): $rel"
            continue
        }

        Write-Host "[$i/$($files.Count)] $rel"

        $content = Get-Content -Path $f.FullName -Raw -Encoding UTF8
        if ($null -eq $content) { $content = "" }

        $writer.WriteLine("<file path=`"$rel`">")
        $writer.WriteLine("<content>")
        $writer.Write($content)
        if (-not $content.EndsWith("`n")) { $writer.WriteLine() }
        $writer.WriteLine("</content>")
        $writer.WriteLine("</file>")
        $writer.WriteLine()

        $totalBytes += $f.Length
    }

    Write-Host ""
    Write-Host "Done: $outputPath"
    Write-Host "Files: $($files.Count), size: $([math]::Round($totalBytes / 1KB, 1)) KB"
} finally {
    $writer.Close()
}