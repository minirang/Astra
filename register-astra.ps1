# register-astra.ps1

$AstraBin = "C:\Users\Skyhanry\Desktop\Astra\bin"

# 현재 사용자 PATH 가져오기
$UserPath = [Environment]::GetEnvironmentVariable("Path", "User")

# 이미 등록되어 있는지 확인
if ($UserPath -split ";" -contains $AstraBin) {
    Write-Host "Astra PATH는 이미 등록되어 있습니다."
}
else {
    [Environment]::SetEnvironmentVariable(
            "Path",
            "$UserPath;$AstraBin",
            "User"
    )

    Write-Host "Astra PATH 등록 완료!"
}

Write-Host ""
Write-Host "새 PowerShell을 열고 다음 명령어로 실행하세요:"
Write-Host "  astra"