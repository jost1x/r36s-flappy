#!/usr/bin/env bash
# Script de compilación para R36S (ARM64)

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

if ! command -v docker >/dev/null 2>&1; then
    echo "Error: Docker es necesario para generar un binario compatible con ArkOS." >&2
    exit 1
fi

if ! docker buildx version >/dev/null 2>&1; then
    echo "Error: Docker Buildx no está disponible." >&2
    exit 1
fi

echo "Compilando para R36S (ARM64, glibc compatible con ArkOS)..."
OUTPUT_DIR="$SCRIPT_DIR/bin/r36s-arm64"
mkdir -p "$OUTPUT_DIR"
docker buildx build \
    --platform linux/arm64 \
    --output "type=local,dest=$OUTPUT_DIR" \
    --file "$SCRIPT_DIR/Dockerfile.r36s" \
    "$SCRIPT_DIR"

test -f "$OUTPUT_DIR/r36s-hello"
chmod +x "$OUTPUT_DIR/r36s-hello"

echo "✓ Compilación completada: ./bin/r36s-arm64/r36s-hello"
echo "Ahora ejecuta: ./deploy-r36s.sh"
