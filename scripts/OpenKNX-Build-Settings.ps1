
# set product names, allows mapping of (devel) name in Project to a more consistent name in release
$settings = lib/OGM-Common/build-scripts/OpenKNX-Build-Settings.ps1 $args[0] "ModbusGateway"

$releaseIndication = $args[0]
if ($releaseIndication) {
    $settings.appRelease=$releaseIndication
    $settings.releaseName="$($settings.sourceName)-$releaseIndication"
} else {
    $settings.appRelease="Release"
    $settings.releaseName="$($settings.sourceName)"
}

Return $settings