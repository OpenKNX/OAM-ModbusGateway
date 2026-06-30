$b = 'src/ModbusGateway.baggages'
if (Test-Path $b) {
    Get-ChildItem $b -Recurse -Force | ForEach-Object {
        $_.Attributes = $_.Attributes -band (-bnot [System.IO.FileAttributes]::ReadOnly)
    }
    (Get-Item $b).Attributes = (Get-Item $b).Attributes -band (-bnot [System.IO.FileAttributes]::ReadOnly)
    Remove-Item $b -Recurse -Force -Confirm:$false
}
