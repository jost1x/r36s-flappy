# Arquitectura

## Resumen

La aplicación renderiza una única pantalla meteorológica a 640 × 480. LVGL compone la interfaz y SDL2 se encarga de la ventana, el renderizado del framebuffer y la entrada.

```text
SDL events → InputManager → Action → WeatherScreen
                                      ↓
Open-Meteo ← WeatherClient ← WeatherScreen
                                      ↓
                            LVGL objects → SDL renderer
```

## Componentes

| Área | Archivos | Responsabilidad |
| --- | --- | --- |
| Ciclo de vida | `src/main.cpp`, `src/app.cpp` | Inicializa SDL2/LVGL, procesa eventos y presenta los fotogramas. |
| Entrada | `src/input_manager.cpp` | Convierte teclado, D-pad, sticks y hombros del mando a `Action`. |
| Datos | `src/weather/weather_client.cpp` | Consulta Open-Meteo de forma asíncrona y entrega resultados a la pantalla. |
| Interfaz | `src/weather/weather_screen.cpp` | Construye la pantalla, el selector de ciudades, estados de carga y controles. |
| Recursos | `src/weather/icons/` | Iconos LVGL compilados junto con la aplicación. |

## Memoria de LVGL

La interfaz usa iconos en formato alpha A8, capas modales y animaciones. Por ello, `include/lv_conf.h` configura un pool de LVGL de 512 KiB (`LV_MEM_SIZE`) y activa el renderizado A8 (`LV_DRAW_SW_SUPPORT_A8`). Esta configuración es parte del requisito de la interfaz: el pool predeterminado de 64 KiB puede dejar los iconos sin dibujar cuando falta memoria temporal.

## Flujo de datos

1. `WeatherScreen::show()` selecciona la ciudad inicial y solicita sus datos.
2. En la primera carga se muestra un skeleton; las recargas posteriores conservan la información visible hasta recibir una respuesta.
3. `WeatherClient` publica un resultado que `WeatherScreen::update()` consume.
4. Al recibir datos válidos, la pantalla actualiza temperatura, condición, métricas, pronóstico y paleta según el clima.

## Selector de ciudades

El selector es un modal LVGL sobre una capa oscura. La capa atenúa la pantalla de fondo para priorizar la lista; no aplica un filtro de desenfoque de píxeles.
