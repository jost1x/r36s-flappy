# Changelog

Todos los cambios notables se documentan en este archivo.

## [Unreleased]

### Fase 1 — Quick Wins
- **Rendimiento**: Sky gradient con `DrawRectangleGradientV` (reemplaza 100+ draw calls por frame)
- **Vsync**: Habilitado por defecto (`FLAG_VSYNC_HINT`) para evitar tearing en PC y R36S
- **Vibración**: Feedback háptico al colisionar (`SetGamepadVibration`)
- **Medallas nuevas**: Diamante (50+) y Platino (100+)
- **Controles**: Soporte para shoulder buttons (L1/R1) y tecla W

### Fase 2 — Core
- **Config centralizada**: `config.h` con todos los constantes del juego en un solo lugar
- **InputHandler**: Clase separada para abstracción de entrada (teclado, gamepad, mouse)
- **Sprites procedurales**: Bird, tuberías y nubes generados como texturas al inicio
- **Parallax background**: Sistema de capas con velocidades independientes
- **Binario autocontenido**: Opción `EMBED_ASSETS` para embeber la fuente en el binario (activado por defecto en R36S)
- **Partículas optimizadas**: Reemplazado `std::rand()` por `std::mt19937` con distribuciones uniformes
- **Daily seed**: `computeDailySeed()` para generar semillas basadas en la fecha actual

### Fase 3 — Polish
- **Transiciones**: Fade-in al inicio, fade-out al pausar (`ScreenTransition`)
- **Música procedural**: Loop de 4 segundos con melodía pentatónica, bajo y percusión suave (`BgmManager`)
- **Menú de opciones**: Sliders de volumen SFX/BGM, toggles de vibración y mostrar FPS
- **Settings persistentes**: `GameSettings` se guarda/carga desde `$XDG_DATA_HOME/r36s-flappy/settings.ini`
- **CI/CD mejorado**: Build ARM64 con Docker Buildx, upload de artefactos, release automático en tags
- **Tests de colisión**: 6 nuevos tests edge cases (borde superior/inferior del gap, centrado, past pipe, múltiples tuberías)
- **BGM control**: Se detiene al pausar/game over, se reanuda al jugar

### Cambios anteriores
- Limpieza de código del proyecto anterior (weather app)
- Dificultad progresiva: velocidad y gap cambian cada 5 puntos
- Rotación del pájaro según velocidad vertical
- Animación de aleteo
- Partículas al pasar tubería
- Flash de colisión
- Parallax en nubes y fondo
- Efectos de sonido (flap, punto, game over) con toggle mute
- Medallas: bronce (5+), plata (15+), oro (30+)
- Contador animado al puntuar
- Opción de pantalla completa desde el menú
- Tests unitarios para física, colisiones y puntuación
- CI/CD mejorado con matrix Debug/Release
- Build system simplificado (FetchContent, sin vcpkg)
