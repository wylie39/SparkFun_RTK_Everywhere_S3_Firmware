# Patches SparkFun_Qwiic_OLED_Arduino_Library's QwGrSSD1306::setScreenBufferAddress()
# so that the X offset (used for SH1106-based displays, which need the classic
# 2-column RAM offset) is applied in real pixel units.
#
# As shipped, the offset is added directly to the high-nibble column command byte:
#   sendDevCommand((kCmdPageModeColTopBase | (column >> 4)) + m_viewport.x);
#   sendDevCommand(kCmdPageModeColLowBase & column);
# Since the high-nibble command's low nibble represents bits 4-7 of the column
# address, adding to it directly only shifts the physical column in units of 16
# (offset=2 -> a 32-column shift, not 2), and the low-nibble command never gets
# the offset at all. That mismatch is what produced the stripe of unwritten/
# wrapped GDDRAM on screen.
#
# Only used for this fork's custom ESP32-S3 Postcard + SH1106 display build - see
# Dockerfile, which only runs this patch when compiling for esp32s3.
#
#   python patch_qwiic_oled_offset.py <path-to-qwiic_grssd1306.cpp>

import sys

if len(sys.argv) != 2:
    print("Usage: python patch_qwiic_oled_offset.py <path-to-qwiic_grssd1306.cpp>")
    sys.exit(1)

path = sys.argv[1]

old = (
    "    // For the column start address, add the viewport x offset. Some devices\n"
    "    // (Micro OLED) don't start at column 0 in the screen buffer\n"
    "    sendDevCommand((kCmdPageModeColTopBase | (column >> 4)) + m_viewport.x);\n"
    "    sendDevCommand(kCmdPageModeColLowBase & column);\n"
)

new = (
    "    // For the column start address, add the viewport x offset. Some devices\n"
    "    // (Micro OLED) don't start at column 0 in the screen buffer\n"
    "    // PATCHED: combine offset+column before splitting into high/low nibble\n"
    "    // commands, so the offset is applied in real pixel units (needed for SH1106).\n"
    "    {\n"
    "        uint8_t patchedColumn = column + m_viewport.x;\n"
    "        sendDevCommand(kCmdPageModeColTopBase | (patchedColumn >> 4));\n"
    "        sendDevCommand(kCmdPageModeColLowBase & patchedColumn);\n"
    "    }\n"
)

with open(path, "r") as f:
    contents = f.read()

if new in contents:
    print("patch_qwiic_oled_offset: already patched, skipping")
    sys.exit(0)

if old not in contents:
    print("patch_qwiic_oled_offset: ERROR - expected source text not found in " + path)
    sys.exit(1)

contents = contents.replace(old, new, 1)

with open(path, "w") as f:
    f.write(contents)

print("patch_qwiic_oled_offset: patched " + path)
