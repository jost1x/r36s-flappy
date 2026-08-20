# Solución de problemas

## Linux de escritorio

El proyecto compila raylib con el backend SDL2. Si se muestra una pantalla negra, vuelve a configurar desde cero con `make clean && make run` para evitar reutilizar una build anterior.

Si falta SDL2:
```sh
sudo apt install libsdl2-dev libwayland-dev libxkbcommon-dev libegl1-mesa-dev
```

## R36S

Revisa `/opt/r36s-app/r36s-flappy.log` después de abrir el port para consultar cualquier error de inicio. La versión ARM64 usa DRM/KMS directamente; vuelve a compilar y desplegar con `make r36s && make deploy` después de cambiar el backend.

Si el binario no arranca, verifica que Docker esté usando la plataforma correcta:
```sh
docker buildx build --platform linux/arm64 ...
```
