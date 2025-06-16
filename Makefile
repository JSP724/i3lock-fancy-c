PRGM = i3lock-fancy
PREFIX = /usr
SHRDIR = /usr/share
BINDIR = /usr/bin
CXX = g++
CXXFLAGS = -std=c++20 -O3 -march=native -flto=auto -DNDEBUG -s
SRC = i3lock-fancy-c.cpp

all: $(PRGM)

$(PRGM): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $<

install: $(PRGM)
	@install -Dm755 $(PRGM)             -t $(DESTDIR)$(BINDIR)
	@install -Dm644 icons/*             -t $(DESTDIR)$(SHRDIR)/$(PRGM)/icons
	@install -Dm644 doc/i3lock-fancy.1  -t $(DESTDIR)$(SHRDIR)/man/man1
	@install -Dm644 LICENSE             -t $(DESTDIR)$(SHRDIR)/licenses/$(PRGM)

uninstall:
	@rm -f $(DESTDIR)$(BINDIR)/$(PRGM)
	@rm -rf $(DESTDIR)$(SHRDIR)/$(PRGM)
	@rm -f $(DESTDIR)$(SHRDIR)/man/man1/i3lock-fancy.1
	@rm -rf $(DESTDIR)$(SHRDIR)/licenses/$(PRGM)

clean:
	@rm -f $(PRGM)
