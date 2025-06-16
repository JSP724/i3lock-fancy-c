#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <cstdlib>
#include <unistd.h>
#include <getopt.h>
#include <cstdio>
#include <memory>
#include <array>
#include <filesystem>
#include <sstream>
#include <string_view>
#include <cstring>
//verion optimizada de i3lock-fancy, de bash a c++

class I3LockFancy {
private:
    std::string hue_params;
    std::string effect_params;
    std::string font;
    std::string image_path;
    std::string text;
    
    static constexpr std::string_view SCRIPT_PATH = "/usr/share/i3lock-fancy-c";
    static constexpr std::string_view DEFAULT_HUE = "-level 0%,100%,0.6";
    static constexpr std::string_view DEFAULT_EFFECT = "-filter Gaussian -resize 20% -define filter:sigma=1.5 -resize 500.5%";
    static constexpr std::string_view PIXELATE_EFFECT = "-scale 10% -scale 1000%";
    static constexpr std::string_view GREYSCALE_HUE = "-level 0%,100%,0.6 -set colorspace Gray -separate -average";
    
    // Optimized command execution with move semantics
    std::string exec_command(std::string_view cmd) const {
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.data(), "r"), pclose);
        if (!pipe) [[unlikely]] return {};
        
        std::string result;
        result.reserve(1024); // Pre-allocate reasonable size
        
        std::array<char, 256> buffer;
        while (fgets(buffer.data(), buffer.size(), pipe.get())) {
            result += buffer.data();
        }
        
        return result;
    }
    
    // Optimized font detection with early exit
    std::string get_default_font() const noexcept {
        try {
            const auto fc_output = exec_command("fc-match sans -f \"%{family}\\n\"");
            if (fc_output.empty()) return "DejaVu-Sans";
            
            const auto family = fc_output.substr(0, fc_output.find('\n'));
            const auto font_list = exec_command("convert -list font");
            
            std::istringstream iss(font_list);
            std::string prev_line, line;
            const std::string target = "family: " + family;
            
            while (std::getline(iss, line)) {
                if (line.find(target) != std::string::npos && !prev_line.empty()) {
                    std::istringstream font_line(prev_line);
                    std::string word;
                    if (font_line >> word >> word) return word;
                }
                prev_line = std::move(line);
            }
        } catch (...) {}
        
        return "DejaVu-Sans";
    }
    
    // Faster temporary file creation
    std::string create_temp_file() const {
        static constexpr char TEMPLATE[] = "/tmp/i3lock_XXXXXX.png";
        char temp_path[sizeof(TEMPLATE)];
        std::memcpy(temp_path, TEMPLATE, sizeof(TEMPLATE));
        
        const int fd = mkstemps(temp_path, 4);
        if (fd == -1) [[unlikely]] throw std::runtime_error("Failed to create temp file");
        
        close(fd);
        return temp_path;
    }
    
    // Runtime string lookup for localization
    std::string_view get_localized_text() const noexcept {
        const char* lang = std::getenv("LANG");
        if (!lang) return "Type password to unlock";
        
        // Use string_view for efficient prefix comparison
        const std::string_view lang_sv(lang);
        if (lang_sv.starts_with("de_")) return "Bitte Passwort eingeben";
        if (lang_sv.starts_with("es_")) return "Ingrese su contraseña";
        if (lang_sv.starts_with("fr_")) return "Entrez votre mot de passe";
        if (lang_sv.starts_with("pl_")) return "Podaj hasło";
        
        return "Type password to unlock";
    }
    
    // Optimized regex compilation and overlay generation
    std::string generate_lock_overlays() const {
        static const std::regex resolution_regex(R"((\d+)x(\d+)\+(\d+)\+(\d+))");
        
        const auto xrandr_output = exec_command("xrandr");
        std::istringstream iss(xrandr_output);
        std::string line, overlays;
        overlays.reserve(2048); // Pre-allocate
        
        while (std::getline(iss, line)) {
            std::smatch matches;
            if (!std::regex_search(line, matches, resolution_regex)) continue;
            
            // Parse dimensions once
            const int w = std::stoi(matches[1]);
            const int h = std::stoi(matches[2]);
            const int xoff = std::stoi(matches[3]);
            const int yoff = std::stoi(matches[4]);
            
            // Calculate positions
            const int midxi = w / 2 + xoff - 30;  // 60/2 = 30
            const int midyi = h / 2 + yoff - 30;
            const int midxt = w / 2 + xoff - 142; // 285/2 ≈ 142
            const int midyt = h / 2 + yoff + 160; // 320/2 = 160
            
            // Build overlay string directly
            overlays += " -font \"" + font + "\" -pointsize 26 -fill lightgrey -stroke grey10"
                       " -strokewidth 2 -annotate +" + std::to_string(midxt) + "+" + std::to_string(midyt) + 
                       " \"" + text + "\" -fill lightgrey -stroke lightgrey -strokewidth 1"
                       " -annotate +" + std::to_string(midxt) + "+" + std::to_string(midyt) + 
                       " \"" + text + "\" \"" + std::string(SCRIPT_PATH) + "/lock.png\""
                       " -geometry +" + std::to_string(midxi) + "+" + std::to_string(midyi) + " -composite";
        }
        
        return overlays;
    }
    
    // Single-pass image processing
    void apply_effects() const {
        // Build complete command in one go
        std::string cmd;
        cmd.reserve(1024);
        cmd = "convert \"" + image_path + "\" " + hue_params + " " + effect_params + 
              generate_lock_overlays() + " \"" + image_path + "\"";
        
        if (std::system(cmd.c_str()) != 0) [[unlikely]] {
            throw std::runtime_error("ImageMagick convert failed");
        }
    }
    
    // Optimized i3lock execution with fallback
    void execute_i3lock() const noexcept {
        // Try enhanced i3lock first (single string construction)
        const std::string enhanced_cmd = 
            "i3lock -n --textcolor=ffffff00 --insidecolor=ffffff1c --ringcolor=ffffff3e "
            "--linecolor=ffffff00 --keyhlcolor=00000080 --ringvercolor=00000000 "
            "--separatorcolor=22222260 --insidevercolor=0000001c --ringwrongcolor=00000055 "
            "--insidewrongcolor=0000001c -i \"" + image_path + "\" >/dev/null 2>&1";
        
        if (std::system(enhanced_cmd.c_str()) != 0) {
            // Fallback to basic i3lock
            const std::string basic_cmd = "i3lock -n -i \"" + image_path + "\"";
            std::system(basic_cmd.c_str());
        }
    }

public:
    I3LockFancy() 
        : hue_params(DEFAULT_HUE)
        , effect_params(DEFAULT_EFFECT)
        , font(get_default_font())
        , image_path(create_temp_file())
        , text(get_localized_text()) {}
    
    ~I3LockFancy() {
        std::filesystem::remove(image_path);
    }
    
    // Delete copy constructor and assignment operator for performance
    I3LockFancy(const I3LockFancy&) = delete;
    I3LockFancy& operator=(const I3LockFancy&) = delete;
    I3LockFancy(I3LockFancy&&) = default;
    I3LockFancy& operator=(I3LockFancy&&) = default;
    
    static void print_help(std::string_view program_name) noexcept {
        std::cout << "Usage: " << program_name << " [options]\n\n"
                  << "Options:\n"
                  << "    -h, --help       This help menu.\n"
                  << "    -g, --greyscale  Set background to greyscale instead of color.\n"
                  << "    -p, --pixelate   Pixelate the background instead of blur, runs faster.\n"
                  << "    -f <fontname>, --font <fontname>  Set a custom font.\n\n";
    }
    
    void set_greyscale() noexcept { hue_params = GREYSCALE_HUE; }
    void set_pixelate() noexcept { effect_params = PIXELATE_EFFECT; }
    void set_font(std::string_view custom_font) noexcept {
        if (!custom_font.empty()) font = custom_font;
    }
    
    void run() const {
        // Take screenshot with error checking
        const std::string screenshot_cmd = "import -window root \"" + image_path + "\"";
        if (std::system(screenshot_cmd.c_str()) != 0) [[unlikely]] {
            throw std::runtime_error("Screenshot failed");
        }
        
        apply_effects();
        execute_i3lock();
    }
};

int main(int argc, char* argv[]) {
    try {
        I3LockFancy i3lock_fancy;
        
        // Optimized option parsing with compile-time array
        static constexpr option long_options[] = {
            {"help", no_argument, nullptr, 'h'},
            {"greyscale", no_argument, nullptr, 'g'},
            {"pixelate", no_argument, nullptr, 'p'},
            {"font", required_argument, nullptr, 'f'},
            {nullptr, 0, nullptr, 0}
        };
        
        int opt;
        while ((opt = getopt_long(argc, argv, "hgpf:", long_options, nullptr)) != -1) {
            switch (opt) {
                case 'h':
                    I3LockFancy::print_help(argv[0]);
                    return 1;
                case 'g':
                    i3lock_fancy.set_greyscale();
                    break;
                case 'p':
                    i3lock_fancy.set_pixelate();
                    break;
                case 'f':
                    i3lock_fancy.set_font(optarg ? optarg : "");
                    break;
                default:
                    std::cerr << "error\n";
                    return 1;
            }
        }
        
        i3lock_fancy.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    
    return 0;
}
