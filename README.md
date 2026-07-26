# R36S Weather App

Aplicación meteorológica para la R36S, escrita en C++17. La interfaz se construye con LVGL y se presenta mediante SDL2; los datos se obtienen de Open-Meteo.

La pantalla está diseñada para 640 × 480 e incluye carga inicial con skeleton, selector de ciudades y controles para teclado y mando.

## Inicio rápido

```sh
make local
make run
```

Para generar el binario ARM64 compatible con ArkOS:

```sh
make r36s
```

Consulta [docs/README.md](docs/README.md) para la documentación completa.
