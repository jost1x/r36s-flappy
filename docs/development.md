# Desarrollo local

## Requisitos

- CMake 3.13 o superior
- Compilador C++17
- SDL2 y las cabeceras de Wayland (`libsdl2-dev`, `libwayland-dev`, `libxkbcommon-dev` y `libegl1-mesa-dev` en Debian/Ubuntu)

CMake descarga y compila raylib 6.0 y raygui 4.5 automáticamente vía FetchContent.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/r36s-flappy
```

Los atajos `make local`, `make release`, `make run` y `make r36s` cubren los flujos habituales. `make run` usa Wayland de forma nativa cuando la sesión local es Wayland, evitando una capa XWayland problemática.

Para probar el modo de pantalla completa localmente:

```sh
R36S_FULLSCREEN=1 ./build/r36s-flappy
```

## Validación

```sh
make verify    # format + lint + tests
make format    # solo clang-format
make lint      # solo clang-tidy
make test      # solo tests
```
