#!/bin/bash
# Script de despliegue por SSH a R36S

set -e

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== R36S Deploy Script ===${NC}"

# Verificar que existe el binario
R36S_BINARY="bin/r36s-arm64/r36s-hello"

if [ ! -f "$R36S_BINARY" ]; then
    echo -e "${RED}Error: $R36S_BINARY no encontrado${NC}"
    echo "Compila primero con: ./build-r36s.sh"
    exit 1
fi

# Solicitar IP si no está en variable de entorno
if [ -z "$R36S_IP" ]; then
    read -p "Introduce la IP de tu R36S: " R36S_IP
fi

R36S_USER="${R36S_USER:-ark}"
R36S_PORT="${R36S_PORT:-22}"
R36S_PATH="/opt/r36s-app/bin"
R36S_SSH_KEY="${R36S_SSH_KEY:-}"
SSH_CONTROL_PATH="${TMPDIR:-/tmp}/r36s-app-ssh-%C"

# Reuse one authenticated SSH connection for the whole deployment. This means
# a password is requested at most once; an SSH key removes the prompt entirely.
SSH_OPTIONS=(
    -o "ControlMaster=auto"
    -o "ControlPersist=5m"
    -o "ControlPath=$SSH_CONTROL_PATH"
    -o "StrictHostKeyChecking=accept-new"
)

if [ -n "$R36S_SSH_KEY" ]; then
    SSH_OPTIONS+=(-i "$R36S_SSH_KEY")
fi

SSH_TARGET="$R36S_USER@$R36S_IP"

echo "Conectando a $SSH_TARGET:$R36S_PORT..."

# Crear directorio remoto
ssh "${SSH_OPTIONS[@]}" -p "$R36S_PORT" "$SSH_TARGET" "mkdir -p '$R36S_PATH'"

# Copiar binario
echo -e "${YELLOW}Copiando binario...${NC}"
scp "${SSH_OPTIONS[@]}" -P "$R36S_PORT" "$R36S_BINARY" "$SSH_TARGET:$R36S_PATH/r36s-hello"

# Hacer ejecutable
ssh "${SSH_OPTIONS[@]}" -p "$R36S_PORT" "$SSH_TARGET" "chmod +x '$R36S_PATH/r36s-hello'"

# Copiar script lanzador si existe puertos
echo -e "${YELLOW}Buscando directorio de puertos...${NC}"
PORT_DIR=$(ssh "${SSH_OPTIONS[@]}" -p "$R36S_PORT" "$SSH_TARGET" "ls -d /roms*/ports 2>/dev/null | head -1")

if [ -n "$PORT_DIR" ]; then
    echo "Instalando lanzador en $PORT_DIR..."
    LAUNCHER_FILE=$(mktemp)
    trap 'rm -f "$LAUNCHER_FILE"' EXIT
    cat > "$LAUNCHER_FILE" << 'EOF'
#!/bin/bash
APP="/opt/r36s-app/bin/r36s-hello"
LOG="/opt/r36s-app/r36s-hello.log"

mkdir -p "$(dirname "$LOG")"
printf '\n===== r36s-hello started: %s =====\n' "$(date)" >> "$LOG"
exec "$APP" >> "$LOG" 2>&1
EOF
    scp "${SSH_OPTIONS[@]}" -P "$R36S_PORT" "$LAUNCHER_FILE" "$SSH_TARGET:$PORT_DIR/r36s-hello.sh"
    ssh "${SSH_OPTIONS[@]}" -p "$R36S_PORT" "$SSH_TARGET" "chmod +x '$PORT_DIR/r36s-hello.sh'"
    echo -e "${GREEN}✓ Lanzador instalado${NC}"
else
    echo -e "${YELLOW}⚠ No se encontró directorio /roms*/ports${NC}"
fi

echo -e "${GREEN}✓ Despliegue completado${NC}"
echo -e "${YELLOW}Ejecuta desde EmulationStation → Ports → r36s-hello${NC}"
