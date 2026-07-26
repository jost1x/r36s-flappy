#!/bin/bash
# Script local de compilación (desarrollo)

set -e

BUILD_DIR="build"
ARCHITECTURE="$(uname -m)"
OUTPUT_DIR="bin/$ARCHITECTURE"

echo "=== Compilación local ==="

# Crear directorio
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configurar
echo "Configurando con CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Compilar
echo "Compilando..."
cmake --build . -j$(nproc)

# Crear alias para fácil ejecución
mkdir -p "../$OUTPUT_DIR"
cp r36s-hello "../$OUTPUT_DIR/"

echo "✓ Compilación completada"
echo "Ejecuta: ./$OUTPUT_DIR/r36s-hello"
