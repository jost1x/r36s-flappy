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

## CI/CD en GitHub Actions

Las pull requests siempre ejecutan formato, lint, compilación de validación y tests. Los push a `main` o `master` no generan binarios de distribución a menos que el mensaje del commit incluya uno de estos marcadores:

| Marcador | Resultado |
| --- | --- |
| `(release)` | Genera artifacts para PC y R36S. |
| `(release:pc)` | Genera sólo el binario de PC. |
| `(release:r36s)` | Genera sólo el binario ARM64 para R36S. |
| `(deploy)` | Genera ambos binarios y publica una GitHub Release. |
| `(deploy:pc)` | Publica una GitHub Release sólo con el binario de PC. |
| `(deploy:r36s)` | Publica una GitHub Release sólo con el binario de R36S. |

Por ejemplo:

```sh
git commit -m "feat: tune difficulty (release:r36s)"
git commit -m "fix: preserve settings on handheld (deploy)"
```

Los artifacts de `release` se conservan durante 30 días. Un marcador `deploy` crea una release con etiqueta `deploy-<SHA>` y adjunta los binarios solicitados. Esto publica los binarios en GitHub; no reemplaza el despliegue SSH local descrito en la guía de R36S.
