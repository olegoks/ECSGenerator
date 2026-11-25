# dump-project.ps1
param(
    [string]$Root       = (Get-Location).Path,
    [string]$Output     = "project_dump.xml",
    [int]$MaxFileSizeKb = 512,
    [switch]$IncludeAuto,          # ���� ����� � �� ��������� auto_* �����
    [switch]$IncludeBinary         # ���� ����� � �� ��������� �� ����������
)

# ---------------------------------------------------------------------------
# 1. ��������, ������� �� �������
# ---------------------------------------------------------------------------
$excludeDirs = @(
    '.git', '.vs', '.vscode', '.idea', '.continue',
    'Build', 'build', 'out', 'bin', 'obj',
    'vcpkg_installed', 'node_modules',
    'ImGui',                       # third-party
    'CMakeTemplates_Generated',    # ������������ CMake configure
    'CPPTemplates_Generated'       # ������������ CMake configure
)

# ---------------------------------------------------------------------------
# 2. ����������, ������� ������
# ---------------------------------------------------------------------------
$extensions = @(
    '.hpp', '.h', '.cpp', '.cxx', '.cc', '.c', '.inl',
    '.ecs', '.ecs_module', '.ecs_package',
    '.lua', '.cmake', '.txt', '.md',
    '.json', '.yaml', '.yml', '.toml',
    '.ps1', '.sh', '.bat', '.py', '.in'
)

# ---------------------------------------------------------------------------
# 3. �������� Root
# ---------------------------------------------------------------------------
if (-not (Test-Path -LiteralPath $Root)) {
    Write-Error "Root path does not exist: $Root"
    exit 1
}
$Root = (Resolve-Path -LiteralPath $Root).Path
$outputPath = Join-Path $Root $Output

# ---------------------------------------------------------------------------
# 4. ������ �������� ������
# ---------------------------------------------------------------------------
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

# ---------------------------------------------------------------------------
# 5. ���� ������
# ---------------------------------------------------------------------------
$autoSkipped = 0

$files = Get-ChildItem -Path $Root -Recurse -File -Force | Where-Object {

    $rel   = $_.FullName.Substring($Root.Length + 1)
    $parts = $rel.Split([IO.Path]::DirectorySeparatorChar)

    # ����������� ��������
    foreach ($p in $parts) {
        if ($excludeDirs -contains $p) { return $false }
    }

    # ��� ����
    if ($_.FullName -eq $outputPath) { return $false }

    # ����������
    if ($extensions -notcontains $_.Extension.ToLower()) { return $false }

    # ������
    if ($_.Length -gt ($MaxFileSizeKb * 1024)) { return $false }

    # ---- �������: ���������� ������������������� ������ ----
    if (-not $IncludeAuto) {
        if ($_.Name -like 'auto_*') {
            $script:autoSkipped++
            return $false
        }
    }

    return $true
} | Sort-Object FullName

# ---------------------------------------------------------------------------
# 6. ������ ����� (� �������������� XML-���������)
# ---------------------------------------------------------------------------
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
$writer = New-Object System.IO.StreamWriter($outputPath, $false, $utf8NoBom)

try {
    $writer.WriteLine("<!-- Project dump -->")
    $writer.WriteLine("<!-- Root: $Root -->")
    $writer.WriteLine("<!-- Auto-generated files excluded: $(-not $IncludeAuto) -->")
    $writer.WriteLine("<!-- Files: $($files.Count) -->")
    $writer.WriteLine()

    $i = 0
    $totalBytes = 0
    $written    = 0

    foreach ($f in $files) {
        $i++
        $rel = $f.FullName.Substring($Root.Length + 1).Replace('\', '/')

        if ((-not $IncludeBinary) -and (Test-BinaryFile $f.FullName)) {
            Write-Host "[$i/$($files.Count)] SKIP (binary): $rel"
            continue
        }

        $content = Get-Content -Path $f.FullName -Raw -Encoding UTF8
        if ($null -eq $content) { $content = "" }

        # ���������� XML-��������, ����� ���� ��� �������� XML
        # � ��������� �������������� �������.
        $escaped = [System.Security.SecurityElement]::Escape($content)

        $writer.WriteLine("<file path=`"$rel`">")
        $writer.WriteLine("<content>")
        $writer.Write($escaped)
        $writer.WriteLine()
        $writer.WriteLine("</content>")
        $writer.WriteLine("</file>")
        $writer.WriteLine()

        $totalBytes += $f.Length
        $written++
        Write-Host "[$i/$($files.Count)] $rel"
    }

    Write-Host ""
    Write-Host "Done: $outputPath"
    Write-Host "Files written: $written, auto_* skipped: $autoSkipped"
    Write-Host "Total source size: $([math]::Round($totalBytes / 1KB, 1)) KB"
}
finally {
    $writer.Close()
}