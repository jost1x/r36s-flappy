# Solución de problemas

## Docker no encuentra `lvgl/lvgl.h`

Los iconos generados por LVGL incluyen `lvgl.h` cuando se define `LV_LVGL_H_INCLUDE_SIMPLE`. El Dockerfile ya compila esos recursos con esa macro y con `LV_CONF_INCLUDE_SIMPLE`. Si reaparece el error, verifica que el comando de compilación de `src/weather/icons/*.c` contenga ambas definiciones y `-I/opt/lvgl`.

## No se detecta un mando

La aplicación continúa funcionando con teclado si SDL2 no detecta un mando compatible. Comprueba que el mando aparezca en SDL2 y que el sistema publique los botones como un game controller estándar.

## El binario ARM64 no inicia en ArkOS

Genera el binario con `make r36s`; no copies una build local. El contenedor usa Debian Buster precisamente para evitar incompatibilidades de glibc con ArkOS.

## La API no devuelve datos

La pantalla conserva la última información válida cuando una actualización falla. Revisa la conectividad de la consola y el log en `/opt/r36s-app/r36s-hello.log` tras desplegarla.
