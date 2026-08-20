# Arquitectura

`src/main.cpp` crea `App`; toda la lógica del juego se concentra en `src/app.cpp` y usa raylib para ventana, renderizado, entrada y tiempo. raygui construye los menús de inicio, pausa y fin de partida.

```text
raylib input → InputHandler → App::update() → física, tuberías y colisiones
                                                ↓
                               App::draw() → sprites + primitivas raylib + raygui
```

## Estructura

```
src/
  main.cpp                  # Punto de entrada, maneja excepciones
  app.cpp                   # Game loop y coordinación
  input_handler.cpp         # Abstracción de entrada
  sprite_manager.cpp        # Generación procedural de sprites
  parallax_background.cpp   # Sistema de fondo parallax
  game/
    bird.cpp                # Física y dibujo del pájaro
    pipe_manager.cpp        # Gestión de tuberías y dificultad
    particle.cpp            # Sistema de partículas
    sound_manager.cpp       # Sonidos procedurales
include/
  app.h                     # Declaración de App
  config.h                  # Constantes centralizadas del juego
  input_handler.h           # Interfaz de entrada
  sprite_manager.h          # Gestión de sprites procedurales
  parallax_background.h     # Sistema de parallax
  game/
    bird.h, pipe.h, pipe_manager.h, particle.h, sound_manager.h
```

## Componentes

### InputHandler
Abstrae la entrada de teclado, gamepad y mouse. Mantiene estado de botones para detección de "pressed" vs "down". Soporta:
- Teclado: Space, Enter, Up, W, P, Escape, M
- Gamepad: A, B, Start, Select, L1/R1 (trigger 1)
- Mouse: Click izquierdo

### SpriteManager
Genera texturas procedurales al inicio del juego:
- Bird sprite (48x48) con cuerpo, ala, ojo y pico
- Pipe body (76x256) con highlight y borde
- Pipe lip (90x28) con highlight y borde
- Cloud sprite (128x128) con forma de nube

Fallback: si los sprites no están cargados, usa primitivas raylib.

### ParallaxBackground
Sistema de capas con velocidades independientes. Cada capa tiene:
- Textura (compartida o propia)
- Velocidad de scroll
- Offset Y
- Alpha/transparencia

### Config
Struct con todas las constantes del juego centralizadas:
- Dimensiones de pantalla
- Física del juego (gravedad, velocidad, gap)
- Valores de medallas
- Parámetros de UI (posiciones de paneles)
- Configuración de vibración

## Game loop

El juego mantiene 3 tuberías reutilizables, limita el delta de cada fotograma para evitar saltos de física y guarda el récord al cerrar. Los sprites se generan una vez al inicio y se reusan durante toda la sesión.

## Build system

- **CMake** con FetchContent para raylib y raygui
- **R36S_DRM=ON** para compilar con backend DRM/KMS (R36S)
- **SDL** como backend local por defecto (mejor compatibilidad con Wayland/X11)
- **EMBED_ASSETS=ON** para embeber la fuente en el binario (activado por defecto en R36S)
- Docker con Debian Buster para cross-compile ARM64 compatible con ArkOS
