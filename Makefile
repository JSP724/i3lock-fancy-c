PRGM = i3lock-fancy-c
PREFIX = /usr
SHRDIR = /usr/share
BINDIR = /usr/bin
CXX = g++
CXXFLAGS = -std=c++20 -O3 -march=native -flto=auto -DNDEBUG -s -lpthread
SRC = i3lock-fancy-c.cpp

# Debug build flags
DEBUG_CXXFLAGS = -std=c++20 -O0 -g -Wall -Wextra -pthread -DDEBUG_TIMING

.PHONY: all debug install uninstall clean check-deps

all: $(PRGM)

$(PRGM): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $<

debug: $(SRC)
	$(CXX) $(DEBUG_CXXFLAGS) -o $(PRGM)-debug $<

install: $(PRGM)
	@install -Dm755 $(PRGM)             -t $(DESTDIR)$(BINDIR)
	@install -Dm644 icons/*             -t $(DESTDIR)$(SHRDIR)/$(PRGM)/icons
	@install -Dm644 doc/i3lock-fancy-c.1  -t $(DESTDIR)$(SHRDIR)/man/man1
	@install -Dm644 LICENSE             -t $(DESTDIR)$(SHRDIR)/licenses/$(PRGM)

uninstall:
	@rm -f $(DESTDIR)$(BINDIR)/$(PRGM)
	@rm -rf $(DESTDIR)$(SHRDIR)/$(PRGM)
	@rm -f $(DESTDIR)$(SHRDIR)/man/man1/i3lock-fancy-c.1
	@rm -rf $(DESTDIR)$(SHRDIR)/licenses/$(PRGM)

clean:
	@rm -f $(PRGM) $(PRGM)-debug

# Check system dependencies
check-deps:
	@echo "Checking dependencies..."
	@command -v g++ >/dev/null 2>&1 || { echo "ERROR: g++ not found"; exit 1; }
	@command -v convert >/dev/null 2>&1 || { echo "ERROR: ImageMagick (convert) not found"; exit 1; }
	@command -v xrandr >/dev/null 2>&1 || { echo "ERROR: xrandr not found"; exit 1; }
	@command -v i3lock >/dev/null 2>&1 || { echo "ERROR: i3lock not found (try i3lock-color)"; exit 1; }
	@echo "Core dependencies found ✓"
	@echo "Optional dependencies:"
	@command -v maim >/dev/null 2>&1 && echo "  maim found ✓ (fastest screenshot tool)" || echo "  maim not found (recommended: sudo apt install maim)"
	@command -v scrot >/dev/null 2>&1 && echo "  scrot found ✓" || echo "  scrot not found (optional: sudo apt install scrot)"
	@echo "Dependencies check complete."

# Show available screenshot tools
show-tools:
	@echo "Available screenshot tools:"
	@command -v maim >/dev/null 2>&1 && echo "  ✓ maim (fastest, recommended)" || echo "  ✗ maim"
	@command -v scrot >/dev/null 2>&1 && echo "  ✓ scrot (fast)" || echo "  ✗ scrot"  
	@command -v import >/dev/null 2>&1 && echo "  ✓ import (ImageMagick, slowest)" || echo "  ✗ import"
