#!/bin/bash
# Setup script para instalar vcpkg y compilar r36s-app-c
# Uso: bash setup-vcpkg.sh

set -e

echo "╔═══════════════════════════════════════════════════════╗"
echo "║  R36S App C++ - Setup Script (vcpkg)                 ║"
echo "╚═══════════════════════════════════════════════════════╝"

# Colores
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

VCPKG_ROOT="${VCPKG_ROOT:-$HOME/vcpkg}"

# 1. Verificar dependencias del sistema
echo -e "${BLUE}1. Verificando dependencias del sistema...${NC}"
if ! command -v git &> /dev/null; then
    echo "❌ git no encontrado. Instala con: sudo apt install git"
    exit 1
fi
if ! command -v cmake &> /dev/null; then
    echo "⚠️  cmake no encontrado. Instalando..."
    sudo apt install -y cmake
fi
if ! command -v g++ &> /dev/null; then
    echo "⚠️  g++ no encontrado. Instalando..."
    sudo apt install -y build-essential
fi
echo -e "${GREEN}✓ Dependencias verificadas${NC}"

# 2. Clonar vcpkg si no existe
echo -e "${BLUE}2. Configurando vcpkg...${NC}"
if [ ! -d "$VCPKG_ROOT" ]; then
    echo "Clonando vcpkg..."
    git clone https://github.com/Microsoft/vcpkg.git "$VCPKG_ROOT"
    cd "$VCPKG_ROOT"
    ./bootstrap-vcpkg.sh
    echo -e "${GREEN}✓ vcpkg instalado en $VCPKG_ROOT${NC}"
else
    echo -e "${GREEN}✓ vcpkg ya existe en $VCPKG_ROOT${NC}"
fi

# 3. Exportar VCPKG_ROOT
echo -e "${BLUE}3. Configurando variables de entorno...${NC}"
export VCPKG_ROOT="$VCPKG_ROOT"
echo "export VCPKG_ROOT=$VCPKG_ROOT" >> ~/.bashrc
echo "export PATH=$VCPKG_ROOT:\$PATH" >> ~/.bashrc
echo -e "${GREEN}✓ VCPKG_ROOT exportado${NC}"

# 4. Compilar r36s-app-c
echo -e "${BLUE}4. Compilando r36s-app-c...${NC}"
cd /home/jostx/Proyectos/Personal/r36s-app-c

mkdir -p build
cd build

cmake .. -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
         -DCMAKE_BUILD_TYPE=Debug

cmake --build . -j$(nproc)

echo -e "${GREEN}✓ Compilación completada${NC}"

# 5. Crear binario en bin/
mkdir -p ../bin
cp r36s-hello ../bin/ 2>/dev/null || echo "Binario creado en: build/r36s-hello"

echo ""
echo -e "${BLUE}5. Ejecutar:${NC}"
echo -e "${YELLOW}  ../bin/r36s-hello${NC}"
echo ""
echo -e "${GREEN}✅ Setup completado!${NC}"
echo ""
echo "Próximos pasos:"
echo "  1. Abre una nueva terminal o ejecuta: source ~/.bashrc"
echo "  2. cd /home/jostx/Proyectos/Personal/r36s-app-c/bin"
echo "  3. ./r36s-hello"
