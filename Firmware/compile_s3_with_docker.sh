#!/usr/bin/env bash
# Builds the ESP32-S3 / LG290P / SH1106 (RTK_S3) firmware with Docker and copies the
# compiled binaries out to build_output_s3/.
#
# This is the S3 equivalent of compile_with_docker.bat, using the BUILD_FQBN /
# BUILD_BOARD_OPTIONS / BUILD_MAX_SIZE / BUILD_OUTPUT_DIR build-args added to the
# Dockerfile for this fork. Run from the Firmware directory:
#   ./compile_s3_with_docker.sh
#
# Uncomment to clear the Docker build cache (fixes arduino-cli not finding the latest
# libraries, at the cost of a much slower rebuild):
#   docker builder prune -f

set -euo pipefail

IMAGE_NAME=rtk_everywhere_firmware_s3
CONTAINER_NAME=rtk_everywhere_s3
OUTPUT_DIR=build_output_s3

mkdir -p "$OUTPUT_DIR"

docker build -t "$IMAGE_NAME" --no-cache-filter deployment \
  --build-arg BUILD_FQBN=esp32:esp32:esp32s3 \
  --build-arg BUILD_BOARD_OPTIONS=PSRAM=opi,FlashMode=qio,FlashSize=16M \
  --build-arg BUILD_MAX_SIZE=6291456 \
  --build-arg BUILD_OUTPUT_DIR=esp32.esp32.esp32s3 \
  .

docker create --name="$CONTAINER_NAME" "$IMAGE_NAME":latest

docker cp "$CONTAINER_NAME":/RTK_Everywhere.ino.bin "$OUTPUT_DIR"/
docker cp "$CONTAINER_NAME":/RTK_Everywhere.ino.elf "$OUTPUT_DIR"/
docker cp "$CONTAINER_NAME":/RTK_Everywhere.ino.bootloader.bin "$OUTPUT_DIR"/
docker cp "$CONTAINER_NAME":/RTK_Everywhere.ino.partitions.bin "$OUTPUT_DIR"/
docker cp "$CONTAINER_NAME":/RTK_Everywhere.ino.merged.bin "$OUTPUT_DIR"/

docker container rm "$CONTAINER_NAME" > /dev/null

echo ""
echo "Done. Binaries in $OUTPUT_DIR/:"
ls -la "$OUTPUT_DIR"

echo ""
echo "Flash with:"
echo "  esptool.exe --chip esp32s3 --port COMxx --baud 921600 --before default_reset --after hard_reset write_flash -z 0x0 $OUTPUT_DIR/RTK_Everywhere.ino.merged.bin"
