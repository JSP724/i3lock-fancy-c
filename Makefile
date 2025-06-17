# Makefile mejorado para i3lock-fancy-c

PRGM          = i3lock-fancy-c
PREFIX        = /usr
SHRDIR        = $(PREFIX)/share
BINDIR        = $(PREFIX)/bin
MANDIR        = $(SHRDIR)/man/man1
LICDIR        = $(SHRDIR)/licenses/$(PRGM)
ICONSDIR      = $(SHRDIR)/$(PRGM)/icons

CXX           = g++
CXXFLAGS      = -std=c++20 -O3 -march=native -flto=auto -DNDEBUG -pthread

VIPS_CFLAGS  := $(shell pkg-config --cflags vips-cpp)
VIPS_LIBS    := $(shell pkg-config --libs   vips-cpp)

SRC           = i3lock-fancy-c.cpp
OBJ           = $(SRC:.cpp=.o)


# Debug build flags
DEBUG_CXXFLAGS = -std=c++20 -O0 -g -Wall -Wextra -pthread -DDEBUG_TIMING

.PHONY: all debug install uninstall clean check-deps show-tools

all: $(PRGM)

$(PRGM): $(OBJ)
	$(CXX) $(CXXFLAGS) $(VIPS_CFLAGS) -o $@ $^ $(VIPS_LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

debug: CXXFLAGS := $(DEBUG_CXXFLAGS)
debug: LDFLAGS   :=
debug: clean $(PRGM)-debug
	@mv $(PRGM) $(PRGM)-debug

install: all
	@echo "Installing $(PRGM)..."
	install -d $(DESTDIR)$(BINDIR) $(DESTDIR)$(ICONSDIR) $(DESTDIR)$(MANDIR) $(DESTDIR)$(LICDIR)
	install -m755 $(PRGM)           $(DESTDIR)$(BINDIR)
	install -m644 icons/*           $(DESTDIR)$(ICONSDIR)
	install -m644 doc/$(PRGM).1     $(DESTDIR)$(MANDIR)
	install -m644 LICENSE           $(DESTDIR)$(LICDIR)
	@echo "Installation complete."

uninstall:
	@echo "Removing $(PRGM)..."
	rm -f $(DESTDIR)$(BINDIR)/$(PRGM)
	rm -rf $(DESTDIR)$(SHRDIR)/$(PRGM)
	rm -f $(DESTDIR)$(MANDIR)/$(PRGM).1
	rm -rf $(DESTDIR)$(LICDIR)
	@echo "Uninstallation complete."

clean:
	rm -f $(OBJ) $(PRGM) $(PRGM)-debug

check-deps:
	@echo "Checking dependencies..."
	@command -v g++ >/dev/null 2>&1 || { echo "ERROR: g++ not found"; exit 1; }
	@command -v convert >/dev/null 2>&1 || { echo "ERROR: ImageMagick (convert) not found"; exit 1; }
	@command -v xrandr >/dev/null 2>&1 || { echo "ERROR: xrandr not found"; exit 1; }
	@command -v i3lock >/dev/null 2>&1 || { echo "ERROR: i3lock not found (try i3lock-color)"; exit 1; }
	@echo "Core dependencies: OK"
	@echo "Optional dependencies:" \
	 && (command -v maim >/dev/null && echo "  ✓ maim" || echo "  ✗ maim (recommended: sudo apt install maim)") \
	 && (command -v scrot >/dev/null && echo "  ✓ scrot" || echo "  ✗ scrot (optional: sudo apt install scrot)") \
	 && (command -v import >/dev/null && echo "  ✓ import" || echo "  ✗ import")

show-tools:
	@echo "Available screenshot tools:"
	@command -v maim >/dev/null && echo "  ✓ maim (recomendado)" || echo "  ✗ maim"
	@command -v scrot >/dev/null && echo "  ✓ scrot"         || echo "  ✗ scrot"
	@command -v import >/dev/null && echo "  ✓ import"        || echo "  ✗ import"  
