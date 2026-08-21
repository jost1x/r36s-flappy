# R36S Flappy Bird

Juego tipo Flappy Bird para la consola R36S, programado en C++17 y renderizado con [raylib](https://www.raylib.com/). Usa raygui para los menús y está diseñado para la pantalla nativa de 640 x 480.

## Características

- Pantalla de inicio, vuelo, pausa y game over
- Dificultad progresiva: velocidad y gap cambian cada 5 puntos
- Récord persistente en `$XDG_DATA_HOME/r36s-flappy/high-score.txt`
- Controles para teclado, mouse y gamepad (R36S) — incluye shoulder buttons (L1/R1)
- Build nativo (Linux) y cross-compile ARM64 para ArkOS
- Sprites pixel-art de pájaro y tuberías, integrados desde el atlas incluido; fallback procedural si no está disponible
- Fondo parallax con nubes en capas
- Vibración del gamepad al colisionar
- Sistema de medallas: bronce (5), plata (15), oro (30), diamante (50), platino (100)
- Binario autocontenido con assets embebidos (opcional, activado por defecto en R36S)
- Vsync habilitado para evitar tearing

## Inicio rápido

```sh
make local
make run
```

Para crear el binario ARM64 compatible con ArkOS:

```sh
make r36s
```

Consulta [los controles](docs/controls.md) y [la guía de R36S](docs/r36s.md) para más detalles.

## Desarrollo

```sh
make install   # dependencias del sistema
make hooks     # activar git hooks
make verify    # format + lint + tests
```

Ver [guía de desarrollo](docs/development.md) para más detalles.

## CI/CD

Los push normales no ejecutan builds de distribución. Añade uno de estos marcadores al mensaje del commit para solicitarlos:

- `(release)` — compila artefactos para PC y R36S.
- `(release:pc)` o `(release:r36s)` — compila sólo el destino indicado.
- `(deploy)` — compila ambos destinos y publica una GitHub Release con los binarios.
- `(deploy:pc)` o `(deploy:r36s)` — publica sólo el destino indicado.

Las pull requests siempre ejecutan formato, lint y tests.

## Arquitectura

```
src/
  main.cpp              # Punto de entrada
  app.cpp               # Game loop y coordinación
  input_handler.cpp     # Abstracción de entrada (teclado, gamepad, mouse)
  sprite_manager.cpp    # Generación procedural de sprites
  parallax_background.cpp # Sistema de fondo parallax
  game/
    bird.cpp            # Física y dibujo del pájaro
    pipe_manager.cpp    # Gestión de tuberías y dificultad
    particle.cpp        # Sistema de partículas
    sound_manager.cpp   # Sonidos procedurales
```

## Controles

| Acción | Teclado | R36S |
|---|---|---|
| Volar | Espacio, Enter, ↑, W, clic | A, B, L1, R1 |
| Navegar menús | Flechas, W/S | Cruceta |
| Confirmar / volver | Enter/Espacio / Escape | A / B |
| Opciones | O | Select |
| Pausar | P | Start |
| Salir | Escape | Select + Start |
| Silenciar | M | — |
