# i3lock-fancy-c

`i3lock-fancy-c` es un fork en C++ de `i3lock-fancy` que bloquea tu pantalla mostrando un fondo desenfocado o pixelado, junto con un icono de candado.

![Captura de pantalla de i3lock-fancy-c](https://raw.githubusercontent.com/meskarune/i3lock-fancy/master/screenshot.png)

---

## Características

* **Rendimiento**: Código en C++20 compilado con LTO y `-march=native`.
* **Desenfoque o pixelado**: Modos configurables desde CLI.
* **Multimonitor**: Detección automática de pantallas con `xrandr`.
* **Personalización básica**: Fuente ajustable mediante `-f <fuente>`.
Nota: Como se vé esta lago mas limitado que i3lock-fancy, pero se mantienen las opciones más usadas
---

## Dependencias

* **Obligatorias**

  * `g++` con soporte C++20
  * `i3lock-color`
  * `ImageMagick` (`import`, `convert`)
  * `xrandr`
* **Opcionales**

  * `scrot` o `maim` para captura de pantalla más rápida que `import`

---

## Compilación

Clona el repositorio y ejecuta:

```bash
make
```

El binario generado será `i3lock-fancy-c`.

---

## Instalación

```bash
sudo make install
```

* El ejecutable va a `/usr/bin/i3lock-fancy-c`.
* `lock.png` se copia a `/usr/share/i3lock-fancy-c/icons/`.
* La página de manual a `/usr/share/man/man1/i3lock-fancy-c.1`.
* La licencia a `/usr/share/licenses/i3lock-fancy-c/LICENSE`.

Para desinstalar:

```bash
sudo make uninstall
```

---

## Uso

```bash
i3lock-fancy-c [opciones] -- <comando-de-captura>
```

### Opciones disponibles

Nota: El uso de `--` es opcional. Puedes pasar directamente el comando de captura sin él, pero se recomienda usarlo para evitar ambigüedades si el comando incluye opciones que podrían ser confundidas con las del propio programa.

* `-h`, `--help`       : Muestra esta ayuda.
* `-g`, `--greyscale`  : Escala de grises.
* `-p`, `--pixelate`   : Pixelado en vez de desenfoque.
* `-f <fuente>`        : Fuente para texto (p. ej. `Liberation-Sans`).

Todo lo que aparezca tras `--` se usa para tomar la captura, por ejemplo `-- scrot -z`.

#### Ejemplo

```bash
i3lock-fancy-c -g -p -f "Liberation-Sans" -- maim
```

---

## Estructura de recursos

```
icons/
  └─ lock.png     # Icono mostrado al bloquear
```

Los recursos se instalan en `/usr/share/i3lock-fancy-c/`.

---

## Contribuciones

1. Abre un *issue* para sugerencias o errores.
2. Crea un *branch* con tus cambios.
3. Haz un *pull request* detallado.

---

Notas:
- Es posible que no se note una gran mejoria a comparacion del i3lock-fancy original, ya que lo que consume más recursos es la captura de la imagen y su procesamiento
- Se tiene pensado añadir más idiomas
- Se tiene pensado añadir unos archivos extra para configurarlo más facilmente

---

## Licencia

© 2025 JSP24. Licencia MIT. Consulta el fichero [LICENSE](LICENSE) para más información.
