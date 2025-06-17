# i3lock-fancy-c

`i3lock-fancy-c` es un fork en C++ de `i3lock-fancy` que bloquea tu pantalla mostrando un fondo desenfocado o pixelado, junto con un icono de candado.

![Captura de pantalla de i3lock-fancy-c](https://raw.githubusercontent.com/meskarune/i3lock-fancy/master/screenshot.png)

---

## Características

* **Rendimiento**: Código en C++20 compilado con LTO y `-march=native`
* **Procesamiento paralelo**: Aplicación de efectos optimizada con multithreading
* **Captura inteligente**: Detección automática de herramientas (`maim` > `scrot` > `import`)
* **Comandos personalizados**: Soporte completo para herramientas de captura personalizadas
* **Desenfoque o pixelado**: Modos configurables desde CLI
* **Multimonitor**: Detección automática de pantallas con `xrandr`
* **Personalización básica**: Fuente ajustable mediante `-f <fuente>`

---

## Dependencias

* **Obligatorias**
  * `g++` con soporte C++20
  * `i3lock-color` (o `i3lock` básico)
  * `ImageMagick` (`convert` para procesamiento de imágenes)
  * `xrandr` (detección de monitores)

* **Opcionales (para captura de pantalla)**
  * `maim` - **Recomendado** (más rápido)
  * `scrot` - Alternativa rápida
  * `import` (ImageMagick) - Fallback automático

---

## Compilación

```bash
# Verificar dependencias
make check-deps

# Compilar versión optimizada
make

# Compilar versión debug (con timing)
make debug
```

---

## Instalación

```bash
sudo make install
```

* Ejecutable → `/usr/bin/i3lock-fancy-c`
* Icono → `/usr/share/i3lock-fancy-c/icons/lock.png`
* Manual → `/usr/share/man/man1/i3lock-fancy-c.1`
* Licencia → `/usr/share/licenses/i3lock-fancy-c/LICENSE`

Para desinstalar:
```bash
sudo make uninstall
```

---

## Uso

### Sintaxis básica
```bash
i3lock-fancy-c [opciones] [-- comando_de_captura]
```

### Opciones disponibles

* `-h, --help`       - Muestra ayuda
* `-g, --greyscale`  - Escala de grises
* `-p, --pixelate`   - Pixelado (más rápido que desenfoque)
* `-f <fuente>`      - Fuente personalizada (ej: `Liberation-Sans`)
* `--sequential`     - Procesamiento secuencial (sin paralelización)

### Ejemplos de uso

```bash
# Uso básico (detección automática de mejor herramienta)
i3lock-fancy-c

# Con opciones de estilo
i3lock-fancy-c -g -p

# Usando scrot con compresión
i3lock-fancy-c -- scrot -z

# Usando maim con formato específico
i3lock-fancy-c -- maim --format png

# Configuración completa
i3lock-fancy-c -g -f "Roboto" -- maim --format png
```

---

## Herramientas de captura

El programa detecta automáticamente la mejor herramienta disponible en este orden:

1. **`maim`** - Más rápido, recomendado
2. **`scrot`** - Alternativa rápida  
3. **`import`** - Fallback (ImageMagick)

### Verificar herramientas disponibles
```bash
make show-tools
```

### Comandos personalizados
Puedes usar cualquier herramienta de captura:

```bash
# Scrot con compresión máxima
i3lock-fancy-c -- scrot -z -q 100

# Maim con selección de ventana
i3lock-fancy-c -- maim --window root

# Gnome-screenshot
i3lock-fancy-c -- gnome-screenshot -f
```

---

## Optimizaciones de rendimiento

### Paralelización automática
- Generación de overlays en paralelo con aplicación de efectos
- Mejora significativa en sistemas multi-core
- Usar `--sequential` para deshabilitar si hay problemas

### Mejores prácticas
```bash
# Más rápido: pixelado + maim
i3lock-fancy-c -p -- maim

# Balance velocidad/calidad: desenfoque + scrot
i3lock-fancy-c -- scrot -z

# Máxima calidad: efectos completos + import
i3lock-fancy-c
```

---

## Integración con i3/sway

### i3 config
```bash
# Binding básico
bindsym $mod+l exec i3lock-fancy-c

# Con suspensión
bindsym $mod+Shift+l exec i3lock-fancy-c && systemctl suspend

# Estilo rápido
bindsym $mod+ctrl+l exec i3lock-fancy-c -p -- maim
```

### Sway config
```bash
# Binding básico
bindsym $mod+l exec i3lock-fancy-c

# Con idle timeout
exec swayidle -w timeout 300 'i3lock-fancy-c' timeout 600 'swaymsg "output * dpms off"'
```

---

## Solución de problemas

### Errores comunes

**"Missing dependency"**
```bash
# Instalar dependencias en Ubuntu/Debian
sudo apt install g++ imagemagick i3lock-color xrandr

# Herramientas opcionales
sudo apt install maim scrot
```

**"Screenshot failed"**
```bash
# Verificar herramientas disponibles
make show-tools

# Probar comando manualmente
i3lock-fancy-c -- maim /tmp/test.png
```

**Rendimiento lento**
```bash
# Usar pixelado en lugar de desenfoque
i3lock-fancy-c -p

# Usar maim si está disponible
i3lock-fancy-c -- maim
```

---

## Desarrollo

### Compilar versión debug
```bash
make debug
./i3lock-fancy-c-debug  # Muestra tiempos de ejecución
```

### Estructura del proyecto
```
i3lock-fancy-c/
├── i3lock-fancy-c.cpp  # Código principal
├── Makefile           # Sistema de build
├── icons/             # Recursos
│   └── lock.png      # Icono de candado
├── doc/              # Documentación
│   └── i3lock-fancy-c.1  # Página de manual
└── README.md         # Este archivo
```

---

## Notas de rendimiento

- **Captura**: `maim` ~50ms, `scrot` ~80ms, `import` ~200ms
- **Procesamiento**: Paralelización mejora ~20-30% en multi-core
- **Total**: ~300-500ms según herramientas y hardware

---

## Contribuciones

1. Abre un *issue* para sugerencias o errores
2. Crea un *branch* con tus cambios  
3. Haz un *pull request* detallado

---

## Licencia

© 2025 JSP24. Licencia MIT. Consulta el fichero [LICENSE](LICENSE) para más información.
