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
#include <thread>
#include <future>
#include <chrono>
//version optimizada de i3lock-fancy, de bash a c++

class I3LockFancy {
private:
    std::string hue_params;
    std::string effect_params;
    std::string font;
    std::string image_path;
    std::string text;
    std::string screenshot_command;
    
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
    
    // Check if a command exists in PATH
    bool command_exists(const std::string& command) const {
        const std::string check_cmd = "which " + command + " >/dev/null 2>&1";
        return std::system(check_cmd.c_str()) == 0;
    }
    
    // Detect best screenshot tool available
    std::string detect_screenshot_tool() const {
        // Priority order: maim (fastest) -> scrot -> import (fallback)
        const std::vector<std::pair<std::string, std::string>> tools = {
            {"maim", "maim \"{}\""},
            {"scrot", "scrot -z \"{}\""},
            {"import", "import -window root \"{}\""}
        };
        
        for (const auto& [tool, cmd_template] : tools) {
            if (command_exists(tool)) {
                std::string cmd = cmd_template;
                size_t pos = cmd.find("{}");
                if (pos != std::string::npos) {
                    cmd.replace(pos, 2, image_path);
                }
                return cmd;
            }
        }
        
        // Fallback to import without checking (should always exist with ImageMagick)
        return "import -window root \"" + image_path + "\"";
    }
    
    // Build custom screenshot command from arguments
    std::string build_custom_screenshot_command(const std::vector<std::string>& args) const {
        if (args.empty()) return detect_screenshot_tool();
        
        std::string cmd;
        cmd.reserve(256);
        
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) cmd += " ";
            cmd += args[i];
        }
        
        // Add image path if not already present
        if (cmd.find(image_path) == std::string::npos) {
            cmd += " \"" + image_path + "\"";
        }
        
        return cmd;
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
        if (lang_sv.starts_with("en_")) return "Type password to unlock";
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
                       " \"" + text + "\" \"" + std::string(SCRIPT_PATH) + "/icons/lock.png\""
                       " -geometry +" + std::to_string(midxi) + "+" + std::to_string(midyi) + " -composite";
        }
        
        return overlays;
    }
    
    // Parallelized image processing
    void apply_effects_parallel() const {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Start overlay generation in parallel (I/O bound - xrandr call)
        auto overlay_future = std::async(std::launch::async, [this]() {
            return generate_lock_overlays();
        });
        
        // Apply base effects to image (CPU bound)
        std::string base_cmd;
        base_cmd.reserve(512);
        base_cmd = "convert \"" + image_path + "\" " + hue_params + " " + effect_params + " \"" + image_path + "\"";
        
        if (std::system(base_cmd.c_str()) != 0) [[unlikely]] {
            throw std::runtime_error("ImageMagick base effects failed");
        }
        
        // Wait for overlay generation and apply overlays
        const auto overlays = overlay_future.get();
        
        if (!overlays.empty()) {
            std::string overlay_cmd;
            overlay_cmd.reserve(1024 + overlays.size());
            overlay_cmd = "convert \"" + image_path + "\" " + overlays + " \"" + image_path + "\"";
            
            if (std::system(overlay_cmd.c_str()) != 0) [[unlikely]] {
                throw std::runtime_error("ImageMagick overlay composition failed");
            }
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        // Optional: Print timing info for debugging (can be removed in release)
        #ifdef DEBUG_TIMING
        std::cerr << "Image processing took: " << duration.count() << "ms\n";
        #endif
    }
    
    // Legacy single-threaded processing (fallback)
    void apply_effects_sequential() const {
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

    // Take screenshot with custom or auto-detected command
    void take_screenshot() const {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        if (std::system(screenshot_command.c_str()) != 0) [[unlikely]] {
            throw std::runtime_error("Screenshot failed with command: " + screenshot_command);
        }
        
        // Verify screenshot was created
        if (!std::filesystem::exists(image_path)) [[unlikely]] {
            throw std::runtime_error("Screenshot file was not created: " + image_path);
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        #ifdef DEBUG_TIMING
        std::cerr << "Screenshot took: " << duration.count() << "ms using: " << screenshot_command << "\n";
        #endif
    }

public:
    I3LockFancy() 
        : hue_params(DEFAULT_HUE)
        , effect_params(DEFAULT_EFFECT)
        , font(get_default_font())
        , image_path(create_temp_file())
        , text(get_localized_text())
        , screenshot_command(detect_screenshot_tool()) {}
    
    ~I3LockFancy() {
        std::filesystem::remove(image_path);
    }
    
    // Delete copy constructor and assignment operator for performance
    I3LockFancy(const I3LockFancy&) = delete;
    I3LockFancy& operator=(const I3LockFancy&) = delete;
    I3LockFancy(I3LockFancy&&) = default;
    I3LockFancy& operator=(I3LockFancy&&) = default;
    
    static void print_help(std::string_view program_name) noexcept {
        std::cout << "Usage: " << program_name << " [options] [-- screenshot_command]\n\n"
                  << "Options:\n"
                  << "    -h, --help       This help menu.\n"
                  << "    -g, --greyscale  Set background to greyscale instead of color.\n"
                  << "    -p, --pixelate   Pixelate the background instead of blur, runs faster.\n"
                  << "    -f <fontname>, --font <fontname>  Set a custom font.\n"
                  << "    --sequential     Use sequential processing instead of parallel.\n\n"
                  << "Screenshot commands:\n"
                  << "    Auto-detected priority: maim > scrot > import\n"
                  << "    Custom: -- scrot -z\n"
                  << "    Custom: -- maim --format png\n\n"
                  << "Examples:\n"
                  << "    " << program_name << "                    # Auto-detect best tool\n"
                  << "    " << program_name << " -g -p              # Greyscale + pixelate\n"
                  << "    " << program_name << " -- scrot -z        # Use scrot with compression\n"
                  << "    " << program_name << " -- maim --format png  # Use maim with PNG format\n\n";
    }
    
    void set_greyscale() noexcept { hue_params = GREYSCALE_HUE; }
    void set_pixelate() noexcept { effect_params = PIXELATE_EFFECT; }
    void set_font(std::string_view custom_font) noexcept {
        if (!custom_font.empty()) font = custom_font;
    }
    
    void set_screenshot_command(const std::vector<std::string>& args) {
        screenshot_command = build_custom_screenshot_command(args);
    }
    
    void run(bool use_parallel = true) const {
        take_screenshot();
        
        if (use_parallel && std::thread::hardware_concurrency() > 1) {
            apply_effects_parallel();
        } else {
            apply_effects_sequential();
        }
        
        execute_i3lock();
    }
};

int main(int argc, char* argv[]) {
    try {
        I3LockFancy i3lock_fancy;
        bool use_sequential = false;
        
        // Optimized option parsing with compile-time array
        static constexpr option long_options[] = {
            {"help", no_argument, nullptr, 'h'},
            {"greyscale", no_argument, nullptr, 'g'},
            {"pixelate", no_argument, nullptr, 'p'},
            {"font", required_argument, nullptr, 'f'},
            {"sequential", no_argument, nullptr, 's'},
            {nullptr, 0, nullptr, 0}
        };
        
        int opt;
        while ((opt = getopt_long(argc, argv, "hgpf:", long_options, nullptr)) != -1) {
            switch (opt) {
                case 'h':
                    I3LockFancy::print_help(argv[0]);
                    return 0;
                case 'g':
                    i3lock_fancy.set_greyscale();
                    break;
                case 'p':
                    i3lock_fancy.set_pixelate();
                    break;
                case 'f':
                    i3lock_fancy.set_font(optarg ? optarg : "");
                    break;
                case 's':
                    use_sequential = true;
                    break;
                default:
                    std::cerr << "Error: Unknown option. Use -h for help.\n";
                    return 1;
            }
        }
        
        // Handle custom screenshot command after --
        std::vector<std::string> screenshot_args;
        for (int i = optind; i < argc; i++) {
            screenshot_args.emplace_back(argv[i]);
        }
        
        if (!screenshot_args.empty()) {
            i3lock_fancy.set_screenshot_command(screenshot_args);
        }
        
        i3lock_fancy.run(!use_sequential);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    
    return 0;
}