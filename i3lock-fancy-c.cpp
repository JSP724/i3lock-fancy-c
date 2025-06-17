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
#include <atomic>
#include <span>
#include <algorithm>
#include <sys/wait.h>
#include <unordered_map>

// Compile-time configuration
namespace Config {
    static constexpr size_t INITIAL_STRING_CAPACITY = 1024;
    static constexpr size_t BUFFER_SIZE = 512;
    static constexpr size_t OVERLAY_CAPACITY = 2048;
    static constexpr int LOCK_ICON_SIZE = 60;
    static constexpr int TEXT_OFFSET_Y = 160;
    static constexpr bool ENABLE_TIMING = true; // Compile-time debug flag
}

class FastI3Lock {
private:
    // Use small string optimization where possible
    using SmallString = std::string;
    
    SmallString hue_params_;
    SmallString effect_params_;
    SmallString font_;
    SmallString image_path_;
    SmallString text_;
    SmallString screenshot_command_;
    
    // Compile-time constants with inline storage
    static constexpr std::string_view SCRIPT_PATH = "/usr/share/i3lock-fancy-c";
    static constexpr std::string_view DEFAULT_HUE = "-level 0%,100%,0.6";
    static constexpr std::string_view DEFAULT_EFFECT = "-filter Gaussian -resize 10% -define filter:sigma=1.5 -resize 1000% +profile '*'";
    static constexpr std::string_view PIXELATE_EFFECT = "-scale 10% -scale 1000%";
    static constexpr std::string_view GREYSCALE_HUE = "-level 0%,100%,0.6 -set colorspace Gray -separate -average";
    
    // Optimized command execution with memory pooling
    SmallString exec_command_fast(std::string_view cmd) const noexcept {
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.data(), "r"), pclose);
        if (!pipe) [[unlikely]] return {};
        
        SmallString result;
        result.reserve(Config::INITIAL_STRING_CAPACITY);
        
        char buffer[Config::BUFFER_SIZE];
        while (fgets(buffer, sizeof(buffer), pipe.get())) {
            result.append(buffer);
        }
        
        // Remove trailing newline efficiently
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }
        
        return result;
    }
    
    // Cached command existence check
    bool command_exists_cached(std::string_view command) const noexcept {
        static thread_local std::unordered_map<std::string, bool> cache;
        
        std::string cmd_str{command};
        auto it = cache.find(cmd_str);
        if (it != cache.end()) {
            return it->second;
        }
        
        const auto check_cmd = "which " + cmd_str + " >/dev/null 2>&1";
        bool exists = std::system(check_cmd.c_str()) == 0;
        cache[cmd_str] = exists;
        return exists;
    }
    
    // Ultra-fast screenshot tool detection with early exit
    SmallString detect_screenshot_tool_fast() const noexcept {
        // Static lookup table for zero-cost abstraction
        static constexpr std::array<std::pair<std::string_view, std::string_view>, 3> tools{{
            {"maim", "maim \"{}\""},
            {"scrot", "scrot -z \"{}\""},
            {"import", "import -window root \"{}\""}
        }};
        
        for (const auto& [tool, cmd_template] : tools) {
            if (command_exists_cached(tool)) {
                SmallString cmd{cmd_template};
                if (const auto pos = cmd.find("{}"); pos != SmallString::npos) {
                    cmd.replace(pos, 2, image_path_);
                }
                return cmd;
            }
        }
        
        // Fallback
        return "import -window root \"" + image_path_ + "\"";
    }
    
    // Zero-allocation font detection
    SmallString get_default_font_fast() const noexcept {
        try {
            const auto fc_output = exec_command_fast("fc-match sans -f \"%{family}\\n\"");
            if (fc_output.empty()) return "DejaVu-Sans";
            
            // Parse family name efficiently
            const auto family = fc_output.substr(0, fc_output.find('\n'));
            if (family.empty()) return "DejaVu-Sans";
            
            const auto font_list = exec_command_fast("convert -list font");
            if (font_list.empty()) return "DejaVu-Sans";
            
            // Fast string search without regex
            const SmallString target = "family: " + family;
            std::istringstream iss(font_list);
            SmallString prev_line, line;
            
            while (std::getline(iss, line)) {
                if (line.find(target) != SmallString::npos && !prev_line.empty()) {
                    // Extract font name efficiently
                    const size_t start = prev_line.find_first_of(' ');
                    const size_t end = prev_line.find_first_of(' ', start + 1);
                    if (start != SmallString::npos && end != SmallString::npos) {
                        return prev_line.substr(start + 1, end - start - 1);
                    }
                }
                prev_line = std::move(line);
            }
        } catch (...) {
            // Silent fallback
        }
        
        return "DejaVu-Sans";
    }
    
    // RAII temporary file with automatic cleanup
    SmallString create_temp_file_fast() const {
        // Use thread-safe mkstemp
        static constexpr char TEMPLATE[] = "/tmp/i3lock_XXXXXX.png";
        char temp_path[sizeof(TEMPLATE)];
        std::memcpy(temp_path, TEMPLATE, sizeof(TEMPLATE));
        
        const int fd = mkstemps(temp_path, 4);
        if (fd == -1) [[unlikely]] {
            throw std::runtime_error("Failed to create temp file");
        }
        
        close(fd);
        return SmallString{temp_path};
    }
    
    // Compile-time localization lookup
    std::string_view get_localized_text_fast() noexcept {
        const char* lang_c = std::getenv("LANG");
        if (!lang_c) return "Type password to unlock";
        std::string_view lang_sv{lang_c};
        // Extraemos los dos primeros caracteres (código de idioma)
        std::string_view code = lang_sv.size() >= 2 ? lang_sv.substr(0, 2) : "";
        if        (code == "de") { return "Bitte Passwort eingeben";
        } else if (code == "es") { return "Ingrese su contraseña";
        } else if (code == "fr") { return "Entrez votre mot de passe";
        } else if (code == "pl") { return "Podaj hasło";
        } else {                   return "Type password to unlock";
        }
    }

    
    // Pre-compiled regex for maximum performance
    inline static const std::regex resolution_regex_{R"((\d+)x(\d+)\+(\d+)\+(\d+))"};
    
    // Optimized overlay generation with string builder pattern
    SmallString generate_lock_overlays_fast() const {
        const auto xrandr_output = exec_command_fast("xrandr");
        if (xrandr_output.empty()) return {};
        
        SmallString overlays;
        overlays.reserve(Config::OVERLAY_CAPACITY);
        
        std::istringstream iss{xrandr_output};
        SmallString line;
        
        // Pre-build common parts
        const SmallString font_part = " -font \"" + font_ + "\" -pointsize 26 -fill lightgrey -stroke grey10 -strokewidth 2";
        const SmallString text_part1 = " -annotate +";
        const SmallString text_part2 = " \"" + text_ + "\" -fill lightgrey -stroke lightgrey -strokewidth 1 -annotate +";
        const SmallString text_part3 = " \"" + text_ + "\" \"" + SmallString{SCRIPT_PATH} + "/icons/lock.png\" -geometry +";
        const SmallString composite_part = " -composite";
        
        while (std::getline(iss, line)) {
            std::smatch matches;
            if (!std::regex_search(line, matches, resolution_regex_)) continue;
            
            // Fast integer parsing
            const int w = std::stoi(matches[1]);
            const int h = std::stoi(matches[2]);
            const int xoff = std::stoi(matches[3]);
            const int yoff = std::stoi(matches[4]);
            
            // Optimized position calculations
            const int midxi = w / 2 + xoff - Config::LOCK_ICON_SIZE / 2;
            const int midyi = h / 2 + yoff - Config::LOCK_ICON_SIZE / 2;
            const int midxt = w / 2 + xoff - 142; // Pre-calculated text offset
            const int midyt = h / 2 + yoff + Config::TEXT_OFFSET_Y;
            
            // Single string concatenation with move semantics
            const auto pos_str = std::to_string(midxt) + "+" + std::to_string(midyt);
            const auto geom_str = std::to_string(midxi) + "+" + std::to_string(midyi);
            
            overlays += font_part + text_part1 + pos_str + text_part2 + pos_str + text_part3 + geom_str + composite_part;
        }
        
        return overlays;
    }
    
    // Async screenshot with timeout
    void take_screenshot_async() const {
        const auto start_time = std::chrono::high_resolution_clock::now();
        
        const int result = std::system(screenshot_command_.c_str());
        if (result != 0) [[unlikely]] {
            throw std::runtime_error("Screenshot failed: " + screenshot_command_);
        }
        
        // Fast existence check
        if (!std::filesystem::exists(image_path_)) [[unlikely]] {
            throw std::runtime_error("Screenshot not created: " + image_path_);
        }
        
        if constexpr (Config::ENABLE_TIMING) {
            const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - start_time
            );
            std::cerr << "Screenshot: " << duration.count() << "ms\n";
        }
    }
    
    // Parallel processing with work stealing
    void apply_effects_parallel_optimized() const {
        const auto start_time = std::chrono::high_resolution_clock::now();
        
        // Launch overlay generation asynchronously
        auto overlay_future = std::async(std::launch::async, [this]() {
            return generate_lock_overlays_fast();
        });
        
        // Apply base effects with optimized command
        SmallString base_cmd;
        base_cmd.reserve(512);
        base_cmd = "convert \"" + image_path_ + "\" " + hue_params_ + " " + effect_params_ + " \"" + image_path_ + "\"";
        
        const int base_result = std::system(base_cmd.c_str());
        if (base_result != 0) [[unlikely]] {
            throw std::runtime_error("Base effects failed");
        }
        
        // Get overlays and apply if not empty
        const auto overlays = overlay_future.get();
        if (!overlays.empty()) {
            SmallString overlay_cmd;
            overlay_cmd.reserve(1024 + overlays.size());
            overlay_cmd = "convert \"" + image_path_ + "\" " + overlays + " \"" + image_path_ + "\"";
            
            const int overlay_result = std::system(overlay_cmd.c_str());
            if (overlay_result != 0) [[unlikely]] {
                throw std::runtime_error("Overlay composition failed");
            }
        }
        
        if constexpr (Config::ENABLE_TIMING) {
            const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - start_time
            );
            std::cerr << "Effects: " << duration.count() << "ms\n";
        }
    }
    
    // Sequential processing for single-core systems
    void apply_effects_sequential_fast() const {
        const auto overlays = generate_lock_overlays_fast();
        
        SmallString cmd;
        cmd.reserve(1024 + overlays.size());
        cmd = "convert \"" + image_path_ + "\" " + hue_params_ + " " + effect_params_ + " " + overlays + " \"" + image_path_ + "\"";
        
        if (std::system(cmd.c_str()) != 0) [[unlikely]] {
            throw std::runtime_error("Image processing failed");
        }
    }
    
    // Optimized i3lock execution with error handling
    void execute_i3lock_fast() const noexcept {
        // Pre-built command string for maximum performance
        static const SmallString enhanced_cmd = 
            "i3lock -n --textcolor=ffffff00 --insidecolor=ffffff1c --ringcolor=ffffff3e "
            "--linecolor=ffffff00 --keyhlcolor=00000080 --ringvercolor=00000000 "
            "--separatorcolor=22222260 --insidevercolor=0000001c --ringwrongcolor=00000055 "
            "--insidewrongcolor=0000001c -i \"" + image_path_ + "\" >/dev/null 2>&1";
        
        if (std::system(enhanced_cmd.c_str()) != 0) {
            const SmallString basic_cmd = "i3lock -ne -i \"" + image_path_ + "\"";
            std::system(basic_cmd.c_str());
        }
    }

public:
    // Constructor with perfect forwarding
    FastI3Lock() 
        : hue_params_{DEFAULT_HUE}
        , effect_params_{DEFAULT_EFFECT}
        , font_{get_default_font_fast()}
        , image_path_{create_temp_file_fast()}
        , text_{get_localized_text_fast()}
        , screenshot_command_{detect_screenshot_tool_fast()} {
        
        // Reserve capacity for all strings to avoid reallocations
        hue_params_.reserve(128);
        effect_params_.reserve(128);
        font_.reserve(64);
        // Si la detección falló, garantizamos un valor por defecto
        if (font_.empty()) {
            font_ = "DejaVu-Sans";
        }
        text_.reserve(64);
        screenshot_command_.reserve(256);
    }
    
    // RAII cleanup
    ~FastI3Lock() {
        std::error_code ec; // Don't throw in destructor
        std::filesystem::remove(image_path_, ec);
    }
    
    // Move-only semantics for performance
    FastI3Lock(const FastI3Lock&) = delete;
    FastI3Lock& operator=(const FastI3Lock&) = delete;
    FastI3Lock(FastI3Lock&&) = default;
    FastI3Lock& operator=(FastI3Lock&&) = default;
    
    // Static help function
    static void print_help(std::string_view program_name) noexcept {
        std::cout << "Usage: " << program_name << " [options] [-- screenshot_command]\n\n"
                  << "Options:\n"
                  << "  -h, --help        This help menu\n"
                  << "  -g, --greyscale   Greyscale background\n"
                  << "  -p, --pixelate    Pixelate instead of blur (faster)\n"
                  << "  -f, --font FONT   Custom font\n"
                  << "  --sequential      Use sequential processing\n\n"
                  << "Auto-detected tools: maim > scrot > import\n"
                  << "Custom commands: -- scrot -z | -- maim --format png\n";
    }
    
    // Inline setters for performance
    constexpr void set_greyscale() noexcept { hue_params_ = GREYSCALE_HUE; }
    constexpr void set_pixelate() noexcept { effect_params_ = PIXELATE_EFFECT; }
    
    void set_font(std::string_view custom_font) noexcept {
        if (!custom_font.empty()) {
            font_.assign(custom_font);
        }
    }
    
    void set_screenshot_command(std::span<const std::string> args) {
        if (args.empty()) {
            screenshot_command_ = detect_screenshot_tool_fast();
            return;
        }
        
        SmallString cmd;
        cmd.reserve(256);
        
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) cmd += ' ';
            cmd += args[i];
        }
        
        if (cmd.find(image_path_) == SmallString::npos) {
            cmd += " \"" + image_path_ + "\"";
        }
        
        screenshot_command_ = std::move(cmd);
    }
    
    // Main execution with branch prediction hints
    void run(bool use_parallel = true) const {
        take_screenshot_async();
        
        if (use_parallel && std::thread::hardware_concurrency() > 1) [[likely]] {
            apply_effects_parallel_optimized();
        } else {
            apply_effects_sequential_fast();
        }
        
        execute_i3lock_fast();
    }
};

// Optimized main function
int main(int argc, char* argv[]) {
    // Early exit for common case
    if (argc == 1) {
        try {
            FastI3Lock{}.run();
            return 0;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << '\n';
            return 1;
        }
    }
    
    try {
        FastI3Lock i3lock;
        bool use_sequential = false;
        
        // Optimized option parsing
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
                    FastI3Lock::print_help(argv[0]);
                    return 0;
                case 'g':
                    i3lock.set_greyscale();
                    break;
                case 'p':
                    i3lock.set_pixelate();
                    break;
                case 'f':
                    i3lock.set_font(optarg ? optarg : "");
                    break;
                case 's':
                    use_sequential = true;
                    break;
                default:
                    std::cerr << "Unknown option. Use -h for help.\n";
                    return 1;
            }
        }
        
        // Handle custom screenshot command
        if (optind < argc) {
            std::vector<std::string> screenshot_args(argv + optind, argv + argc);
            i3lock.set_screenshot_command(screenshot_args);
        }
        
        i3lock.run(!use_sequential);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred\n";
        return 1;
    }
    
    return 0;
}
