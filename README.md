# i3lock-fancy-c

`i3lock-fancy-c` is a C++ fork of `i3lock-fancy` that locks your screen by displaying a blurred or pixelated background, along with a lock icon.

![i3lock-fancy-c Screenshot](https://raw.githubusercontent.com/meskarune/i3lock-fancy/master/screenshot.png)

---

## Features

* **Performance**: C++20 code compiled with LTO and `-march=native`
* **Parallel Processing**: Application of optimized effects with multithreading
* **Smart Capture**: Automatic tool detection (`maim` > `scrot` > `import`)
* **Custom Commands**: Full support for custom capture tools
* **Blur or Pixelate**: Configurable modes from the CLI
* **Multi-Monitor**: Automatic display detection with `xrandr`
* **Basic Customization**: Adjustable font using `-f <font>`

---
## Dependencies

* **Required**
* `g++` supported C++20
* `i3lock-color` (or basic `i3lock`)
* `ImageMagick` (`convert` for image processing)
* `xrandr` (monitor detection)

* **Optional (for screen capture)**
* `maim` - **Recommended** (faster)
* `scrot` - Fast alternative
* `import` (ImageMagick) - Automatic backup

---

## Compilation

``` tap
# Check dependencies
make checks

# Optimized compiler version
make

# Debug compiler version (with synchronization)
make debug
```

---

## Installation

``` tap
sudo make install
```

* Executable → `/usr/bin/i3lock-fancy-c`
* Icon → `/usr/share/i3lock-fancy-c/icons/lock.png`
* Manual → `/usr/share/man/man1/i3lock-fancy-c.1`
* License → `/usr/share/licenses/i3lock-fancy-c/LICENSE`

To uninstall:
``` tap
sudo make uninstall
```

---

##usage

### Basic Syntax
``` tap
i3lock-fancy-c [options] [-- capture_command]
```

### Available Options

* `-h, --help` - Display help
* `-g, --greyscale` - Grayscale
* `-p, --pixelate` - Pixelate (faster than blur)
* `-f <font>` - Custom font (e.g.: `Liberation-Sans`)
* `--sequential` - Sequential processing (no parallelization)

### Usage Examples

```tap
# Basic Usage (Automatic Best Tool Detection)
i3lock-fancy-c

# With Style Options
i3lock-fancy-c -g -p

# Using scrot with compression
i3lock-fancy-c --scrot -z

# Using maim with a specific format
i3lock-fancy-c --mutilate --png format

# Full Configuration
i3lock-fancy-c -g -f "Roboto" --mutilate --png format
```

---

## Capture Tools

The program automatically detects the best available tool in this order:

1. **`maim`** - Fastest, recommended
2. **`scrot`** - Fast alternative
3. **`import`** - Backup (ImageMagick)

### Check Available Tools
```scrot
make display tools
```

### Custom Commands
You can use any capture tool:

```scrot
# Scrot with maximum compression
i3lock-fancy-c --scrot -z -q 100

# Maim with window selection
i3lock-fancy-c --mutilate --window-root

# Gnome Screenshot
i3lock-fancy-c --gnome-screenshot -f
```

---

## Performance Optimizations

### Automatic Parallelization
- Parallel overlay generation with effects application
- Significant improvement on multi-core systems
- Use `--sequential` to disable if there are problems

### Best Practices
```scrot
# Faster: pixelate + mutilate
i3lock-fancy-c -p -- mutilate

# Speed/quality balance: blur + scrot
i3lock-fancy-c -- scrot -z

# Maximum quality: full effects + import
i3lock-fancy-c
```

---

## i3/Sway Integration

### i3 Configuration
```bash
# Basic Binding
bindsym $mod+l exec i3lock-fancy-c

# With suspend
bindsym $mod+Shift+l exec i3lock-fancy-c && systemctl suspend

# Quick Style
bindsym $mod+ctrl+l exec i3lock-fancy-c -p -- maim
```

### Sway Configuration
```bash
# Basic Binding
bindsym $mod+l exec i3lock-fancy-c

# With idle timeout
exec swayidle -w timeout 300 'i3lock-fancy-c' timeout 600 'swaymsg "output * dpms disabled"'
```

---

## Troubleshooting

### Common Errors

**"Missing Dependency"**
``` swipe
# Install Dependencies on Ubuntu/Debian
sudo apt install g++ imagemagick i3lock-color xrandr

# Optional Tools
sudo apt install mutilate scrot
```

**"Screenshot Failed"**
``` swipe
# Check Available Tools
make displaytools

# Test Command Manually
i3lock-fancy-c -- mutilate /tmp/test.png
```

**Slow Performance**
``` swipe
# Use pixelation instead of blur
i3lock-fancy-c -p

# Use mutilate if available
i3lock-fancy-c -- mutilate
```

---

##Development

### Version debug compiler
``` tap
make debug
./i3lock-fancy-c-debug # Show runtimes
```

### Project structure
```
i3lock-fancy-c/
├── i3lock-fancy-c.cpp # Main code
├── Makefile # C system
`maim` ~50ms, `scrot` ~80ms, `import` ~200ms
- **Processing**: Parallelization improves ~20-30% on multi-core
- **Total**: ~300-500ms depending on tools and hardware

---

## Contributions

1. Open an issue with suggestions or bugs
2. Create a branch with your changes
3. Make a detailed pull request

---

## License

© 2025 JSP724. MIT License. See the [LICENSE](LICENSE) file for more information.
