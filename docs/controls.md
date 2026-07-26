# Controles

| Acción | Teclado | Mando |
| --- | --- | --- |
| Cambiar ciudad | Flechas arriba/abajo | D-pad arriba/abajo o stick vertical |
| Cambiar día | Flechas izquierda/derecha | D-pad izquierda/derecha, stick horizontal, L1/R1 |
| Abrir detalle del día | Enter o Espacio | A o Start |
| Abrir/cerrar selector de ciudades | Retroceso | B o Back |
| Actualizar clima | R | Y |
| Añadir/quitar favorito | F | X |
| Reordenar favorito | Q / E | Pulsar stick izquierdo/derecho |
| Salir | Escape | Start + Back |

Desde el resumen, A abre el detalle del día seleccionado. Dentro del detalle, izquierda y derecha cambian de día; A o B regresan al resumen. B abre el selector de ciudades.

Cuando el selector está abierto, arriba y abajo recorren la lista; A confirma y aplica la ciudad seleccionada.
Izquierda y derecha alternan entre **Favoritos** y **Todas las ciudades**. El encabezado muestra la posición actual (por ejemplo, `6 / 18`); arriba y abajo recorren todo el catálogo, aunque solo cuatro filas caben en pantalla. X/F añade o quita la ciudad resaltada de favoritos; la lista de favoritos conserva el orden configurado.
En la pestaña Favoritos, Q/E o los clics de stick mueven la ciudad seleccionada en la lista.

En la R36S, SDL etiqueta el botón físico **A** como `BUTTON_B` y el físico **B** como `BUTTON_A`; `InputManager` adapta esa diferencia para conservar las acciones indicadas en esta tabla.

Los sticks tienen una zona muerta amplia y el eje dominante decide la dirección. Tras el primer movimiento, la repetición espera 320 ms y continúa cada 140 ms mientras se mantenga la inclinación, evitando cambios bruscos por diagonales o pequeñas oscilaciones.
