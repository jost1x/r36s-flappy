# Desarrollo local

## Requisitos

- CMake 3.21 o superior
- Compilador C++17
- SDL2, libcurl y nlohmann-json
- vcpkg para resolver dependencias de forma reproducible (recomendado)

LVGL 9.2.2 se descarga automáticamente durante la configuración de CMake.

## Hooks de Git

Activa los hooks versionados una vez después de clonar el repositorio:

```sh
make -f Makefile-cpp hooks
```

El hook `pre-commit` comprueba los espacios y conflictos de formato en los cambios preparados. El hook `pre-push` configura la build, compila con un único trabajo y ejecuta `ctest`; cancela el push si alguna comprobación falla.

Para ejecutar la validación de `pre-commit` manualmente:

```sh
make -f Makefile-cpp pre-commit
```

## Configuración con vcpkg

```sh
git clone https://github.com/microsoft/vcpkg.git "$HOME/vcpkg"
"$HOME/vcpkg/bootstrap-vcpkg.sh"
export VCPKG_ROOT="$HOME/vcpkg"

cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build
```

## Atajos del proyecto

```sh
make local    # configura y compila una build local de depuración
make release  # compila una build local optimizada
make run      # compila y ejecuta build/r36s-hello
make clean    # elimina artefactos locales generados
```

El ejecutable local se genera en `build/r36s-hello`. Para ejecutar a pantalla completa:

```sh
R36S_FULLSCREEN=1 ./build/r36s-hello
```

## Validación

Antes de enviar cambios, ejecuta como mínimo:

```sh
cmake --build build -j1
```
