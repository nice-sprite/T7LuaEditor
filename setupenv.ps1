# start-process "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -ArgumentList "-prerelease", "-latest", "-property", "installationPath" -RedirectStandardOutput 
pushd "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools"
cmd /c "VsDevCmd.bat -arch=amd64 -host_arch=amd64&set" |
foreach {
  if ($_ -match "=") {
    $v = $_.split("=", 2); set-item -force -path "ENV:\$($v[0])"  -value "$($v[1])" 
  }
}
popd
Write-Host "`nVisual Studio 2022 Command Prompt variables set." -ForegroundColor Yellow
