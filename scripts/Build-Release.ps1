# This script is just a template and has to be copied and modified per project
# This script should be called from .vscode/tasks.json with
#
#   scripts/Build-Release.ps1            - for Beta builds
#   scripts/Build-Release.ps1 Release    - for Release builds
#
# {
#     "label": "Build-Release",
#     "type": "shell",
#     "command": "scripts/Build-Release.ps1 Release",
#     "args": [],
#     "problemMatcher": [],
#     "group": "test"
# },
# {
#     "label": "Build-Beta",
#     "type": "shell",
#     "command": "scripts/Build-Release.ps1 ",
#     "args": [],
#     "problemMatcher": [],
#     "group": "test"
# }

# set product names, allows mapping of (devel) name in Project to a more consistent name in release
# $settings = scripts/OpenKNX-Build-Settings.ps1

# execute generic pre-build steps
../OGM-Common/scripts/setup/reusable/Build-Release-Preprocess.ps1 $args[0]
if (!$?) { exit 1 }

# build firmware for DEVICE_SMARTMF_MODBUS_RTU_3BE
lib/OGM-Common/scripts/setup/reusable/Build-Step.ps1 release_RP2040_TP_DEVICE_SMARTMF_MODBUS_RTU_3BE firmware-SMART-MF_Modbus_RTU_2TE uf2
if (!$?) { exit 1 }

# build firmware for DEVICE_SMARTMF_1TE_MODBUS
lib/OGM-Common/scripts/setup/reusable/Build-Step.ps1 release_RP2040_TP_DEVICE_SMARTMF_1TE_MODBUS firmware-SMART-MF_Modbus_RTU_1TE uf2
if (!$?) { exit 1 }

# build firmware for DEVICE_SMARTMF_1TE_MODBUS
lib/OGM-Common/scripts/setup/reusable/Build-Step.ps1 release_RP2040_TP_DEVICE_SMARTMF_MODBUS_AUSSEN firmware-SMART-MF_Modbus_RTU_AUSSEN uf2
if (!$?) { exit 1 }

# execute generic post-build steps
../OGM-Common/scripts/setup/reusable/Build-Release-Postprocess.ps1 $args[0]
if (!$?) { exit 1 }
