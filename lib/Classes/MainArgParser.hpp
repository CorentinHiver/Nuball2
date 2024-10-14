#ifndef MAINARGPARSER
#define MAINARGPARSER

#include <variant>
// using Function = std::variant<void(*)(int), void(*)(double), void(*)(const std::string&)>;
using ArgumentType = std::variant<int, double, std::string>;
class MainArgParser {
public:
    MainArgParser(int argc, char* argv[]) : argc_(argc) {
        argv_ = new char*[argc + 1];
        for (int i = 0; i < argc; ++i) {
            argv_[i] = argv[i];
        }
        argv_[argc] = nullptr;
    }

    ~MainArgParser() {
        delete[] argv_;
    }

void addParameter(const std::string& flag, const std::function<void(const std::string&)>& func) {
    parameters_[flag] = func;
}

    void parseAndDispatch() {
        if (argc_ < 2) {
            std::cerr << "Usage: " << argv_[0] << " [options]\n";
            return;
        }

        for (size_t i = 1; i < argc_; ++i) {
            if (parameters_.find(argv_[i]) != parameters_.end()) {
                if (i + 1 >= argc_) {
                    std::cerr << "Error: Missing argument for flag '" << argv_[i] << "'\n";
                    return;
                }
                parameters_[argv_[i]](argv_[i+1]);
                ++i;
            } else if (argv_[i] == std::string("-h") || argv_[i] == std::string("--help")) {
                printHelp();
                return;
            } else {
                std::cerr << "Error: Unknown flag '" << argv_[i] << "'\n";
                return;
            }
        }
    }

private:
    void printHelp() const {
        std::cout << "Usage: " << argv_[0] << " [options]\n";
        for (const auto& [flag, func] : parameters_) {
            std::cout << "  " << flag << " <arg>  " << func.target_type().name() << "\n";
        }
    }

    size_t argc_;
    char** argv_;
    std::unordered_map<std::string, std::function<void(const std::string&)>> parameters_;
};

#endif //MAINARGPARSER