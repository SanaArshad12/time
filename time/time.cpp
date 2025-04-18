#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <stack>
#include <unordered_map>
#include <iomanip>
#include <locale>

using namespace std;

// ANSI color codes
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"
#define UNDERLINE "\033[4m"

// Time complexity enumeration
enum class Complexity {
    CONSTANT,      // O(1)
    LINEAR,        // O(n)
    QUADRATIC,     // O(n²)
    CUBIC,         // O(n³)
    LINEARITHMIC,  // O(n log n)
    UNKNOWN
};

// Structure to hold analysis results
struct CodeAnalysis {
    int line_number;
    string code;
    Complexity complexity;
    string reason;
};

class ComplexityAnalyzer {
private:
    vector<string> code_lines;
    unordered_map<string, int> function_calls;
    stack<string> block_stack;
    string current_function;
    int nesting_level = 0;
    int max_nesting = 0;

    // Helper function to trim whitespace
    static string trim(const string& str) {
        const regex pattern("^\\s+|\\s+$");
        return regex_replace(str, pattern, "");
    }

    // Check if line is a comment
    static bool is_comment(const string& line) {
        return line.empty() || line.substr(0, 2) == "//";
    }

    // Detect recursive function calls
    bool is_recursive(const string& line, const string& func_name) const {
        if (func_name.empty()) return false;
        return regex_search(line, regex("\\b" + func_name + "\\s*\\("));
    }

    // Track function definitions in the code
    void track_function_definitions() {
        for (const auto& line : code_lines) {
            smatch match;
            if (regex_search(line, match, regex(R"(\b([a-zA-Z_][a-zA-Z0-9_]*)\s*\([^)]*\)\s*(?:const)?\s*[{;])"))) {
                function_calls[match[1].str()]++;
            }
        }
    }

public:
    explicit ComplexityAnalyzer(const vector<string>& code) : code_lines(code) {}

    // Convert complexity enum to string with color
    static string complexity_to_string(Complexity c) {
        switch (c) {
        case Complexity::CONSTANT:     return GREEN + string("O(1)") + RESET;
        case Complexity::LINEAR:       return YELLOW + string("O(n)") + RESET;
        case Complexity::QUADRATIC:    return RED + string("O(n²)") + RESET;
        case Complexity::CUBIC:        return MAGENTA + string("O(n³)") + RESET;
        case Complexity::LINEARITHMIC: return CYAN + string("O(n log n)") + RESET;
        default:                      return WHITE + string("Unknown") + RESET;
        }
    }

    // Get explanation for the complexity with color
    string get_complexity_reason(const string& line, Complexity complexity) const {
        switch (complexity) {
        case Complexity::CONSTANT:
            return GREEN + string("Constant time operation (no loops)") + RESET;

        case Complexity::LINEAR:
            if (regex_search(line, regex(R"(\b(for|while)\s*\()"))) {
                return YELLOW + string("Single loop running n times") + RESET;
            }
            return YELLOW + string("Linear time operation") + RESET;

        case Complexity::QUADRATIC:
            return RED + string("Nested loops (n × n iterations)") + RESET;

        case Complexity::CUBIC:
            return MAGENTA + string("Triple nested loops (n × n × n iterations)") + RESET;

        case Complexity::LINEARITHMIC:
            return CYAN + string("Divide-and-conquer or recursive algorithm") + RESET;

        default:
            return WHITE + string("Unable to determine complexity") + RESET;
        }
    }

    // Analyze a single line of code
    Complexity analyze_line(const string& line) {
        if (is_comment(line)) return Complexity::CONSTANT;

        // Check for loops
        if (regex_search(line, regex(R"(\b(for|while)\s*\()"))) {
            if (nesting_level == 0) return Complexity::LINEAR;
            if (nesting_level == 1) return Complexity::QUADRATIC;
            return Complexity::CUBIC;
        }

        // Check for recursion
        if (!current_function.empty() && is_recursive(line, current_function)) {
            return Complexity::LINEARITHMIC;
        }

        // Check for function calls
        if (regex_search(line, regex(R"(\b[a-zA-Z_][a-zA-Z0-9_]*\s*\([^)]*\)\s*;)"))) {
            return Complexity::UNKNOWN;
        }

        return Complexity::CONSTANT;
    }

    // Analyze the entire code
    vector<CodeAnalysis> analyze() {
        vector<CodeAnalysis> results;
        track_function_definitions();

        for (size_t i = 0; i < code_lines.size(); ++i) {
            string line = trim(code_lines[i]);

            // Track function declarations
            smatch match;
            if (regex_search(line, match, regex(R"(\b([a-zA-Z_][a-zA-Z0-9_]*)\s*\([^)]*\)\s*(?:const)?\s*[{])"))) {
                current_function = match[1].str();
            }

            // Track block openings
            if (regex_search(line, regex(R"(\b(for|while)\s*\()"))) {
                block_stack.push("loop");
                nesting_level++;
                max_nesting = max(max_nesting, nesting_level);
            }
            else if (line.find('{') != string::npos) {
                block_stack.push("block");
            }

            // Analyze the line
            Complexity comp = analyze_line(line);
            results.push_back({
                static_cast<int>(i + 1),
                line,
                comp,
                get_complexity_reason(line, comp)
                });

            // Track block closings
            if (line.find('}') != string::npos && !block_stack.empty()) {
                if (block_stack.top() == "loop") nesting_level--;
                block_stack.pop();
            }
        }

        return results;
    }

    // Estimate overall complexity based on max nesting
    Complexity estimate_overall_complexity() const {
        switch (max_nesting) {
        case 0: return Complexity::CONSTANT;
        case 1: return Complexity::LINEAR;
        case 2: return Complexity::QUADRATIC;
        case 3: return Complexity::CUBIC;
        default: return Complexity::LINEARITHMIC;
        }
    }
};

// Print analysis results with colored ASCII formatting
void print_results(const vector<CodeAnalysis>& results) {
    cout << "\n" << BOLD << BLUE << "Line-by-Line Complexity Analysis:" << RESET << "\n";
    cout << BOLD << "================================" << RESET << "\n";

    for (const auto& result : results) {
        cout << BOLD << "Line " << setw(3) << result.line_number << ": " << RESET
            << WHITE << result.code << RESET << "\n";
        cout << "  " << BOLD << GREEN << "->" << RESET << " Complexity: "
            << ComplexityAnalyzer::complexity_to_string(result.complexity) << "\n";
        cout << "  " << BOLD << YELLOW << "* " << RESET << "Reason: " << result.reason << "\n";
        cout << BOLD << "--------------------------------" << RESET << "\n";
    }
}

// Print final complexity with colored ASCII formatting
void print_final_complexity(Complexity complexity) {
    cout << "\n" << BOLD << "================================" << RESET << "\n";
    cout << BOLD << "Final Complexity: " << RESET
        << ComplexityAnalyzer::complexity_to_string(complexity) << "\n";
    cout << BOLD << "================================" << RESET << "\n";
}

int main() {
    // Set locale for consistent output
    ios_base::sync_with_stdio(false);
    locale::global(locale(""));
    cout.imbue(locale());

    cout << BOLD << CYAN << "C++ Time Complexity Analyzer" << RESET << "\n";
    cout << BOLD << "Enter your code (type 'END' on a new line to finish):" << RESET << "\n\n";

    // Read input code
    vector<string> code;
    string line;
    while (getline(cin, line) && line != "END") {
        code.push_back(line);
    }

    // Analyze and display results
    ComplexityAnalyzer analyzer(code);
    auto results = analyzer.analyze();
    print_results(results);
    print_final_complexity(analyzer.estimate_overall_complexity());

    return 0;
}