# Solución de problemas

## Docker no encuentra `lvgl/lvgl.h`

Los iconos generados por LVGL incluyen `lvgl.h` cuando se define `LV_LVGL_H_INCLUDE_SIMPLE`. El Dockerfile ya compila esos recursos con esa macro y con `LV_CONF_INCLUDE_SIMPLE`. Si reaparece el error, verifica que el comando de compilación de `src/weather/icons/*.c` contenga ambas definiciones y `-I/opt/lvgl`.

## Los iconos del clima no aparecen

Los recursos de `src/weather/icons/` son máscaras alpha (`LV_COLOR_FORMAT_A8`). Además de habilitar `LV_DRAW_SW_SUPPORT_A8`, LVGL necesita memoria de trabajo para componerlas junto con las capas, el modal y las animaciones de la interfaz.

La aplicación reserva 512 KiB mediante `LV_MEM_SIZE` en `include/lv_conf.h`; el valor predeterminado de LVGL (64 KiB) no es suficiente para esta pantalla. Si el problema vuelve a aparecer tras agregar elementos visuales, aumenta ese valor y recompila por completo:

```sh
cmake --build build -j1
```

No reduzcas el valor sin comprobar en la R36S que se siguen viendo el icono principal, los iconos del pronóstico y los de métricas.

## No se detecta un mando

La aplicación continúa funcionando con teclado si SDL2 no detecta un mando compatible. Comprueba que el mando aparezca en SDL2 y que el sistema publique los botones como un game controller estándar.

## El binario ARM64 no inicia en ArkOS

Genera el binario con `make r36s`; no copies una build local. El contenedor usa Debian Buster precisamente para evitar incompatibilidades de glibc con ArkOS.

## La API no devuelve datos

La pantalla conserva la última información válida cuando una actualización falla. Revisa la conectividad de la consola y el log en `/opt/r36s-app/r36s-hello.log` tras desplegarla.
