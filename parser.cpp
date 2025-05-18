class SimpleParser {
public:
    SimpleParser(const std::string& expression) : expr(expression), pos(0) {
        functions = {
            {"sin", [](double x) { return std::sin(x); }},
            {"cos", [](double x) { return std::cos(x); }},
            {"tan", [](double x) { return std::tan(x); }},
            {"tg",  [](double x) { return std::tan(x); }},
            {"ctg", [](double x) { return 1.0/std::tan(x); }},
            {"ctan",[](double x) { return 1.0/std::tan(x); }},
            {"log", [](double x) { return std::log(x); }},
            {"ln",  [](double x) { return std::log(x); }},
            {"lg",  [](double x) { return std::log10(x); }},
            {"sqrt",[](double x) { return std::sqrt(x); }},
            {"abs", [](double x) { return std::abs(x); }}
        };
        
        constants = {
            {"pi", 3.14159265358979323846},
            {"e",  2.71828182845904523536}
        };
    }

    double evaluate(double x) {
        this->x = x;
        pos = 0;
        try {
            double result = parseExpression();
            if (pos < expr.size()) {
                throw std::runtime_error("Unexpected characters at end of expression");
            }
            return result;
        } catch (const std::exception& e) {
            throw std::runtime_error(e.what());
        }
    }

private:
    std::string expr;
    size_t pos;
    double x;
    
    std::unordered_map<std::string, std::function<double(double)>> functions;
    std::unordered_map<std::string, double> constants;

    std::string formatError(const std::string& msg) const {
        std::ostringstream oss;
        oss << msg << " at position " << pos << " (near '";
        
        size_t start = (pos > 10) ? pos - 10 : 0;
        size_t end = (pos + 10 < expr.size()) ? pos + 10 : expr.size();
        oss << expr.substr(start, pos - start) << ">>>" << expr[pos] << "<<<" 
            << expr.substr(pos + 1, end - pos - 1) << "')";
            
        return oss.str();
    }

    double parseExpression() {
        double left = parseTerm();
        while (true) {
            if (match('+')) left += parseTerm();
            else if (match('-')) left -= parseTerm();
            else break;
        }
        return left;
    }

    double parseTerm() {
        double left = parseFactor();
        while (true) {
            if (match('*')) left *= parseFactor();
            else if (match('/')) {
                double divisor = parseFactor();
                if (divisor == 0.0) throw std::runtime_error("Division by zero");
                left /= divisor;
            }
            else if (implicitMultiplicationPending()) left *= parseFactor();
            else break;
        }
        return left;
    }

    double parseFactor() {
        bool negative = false;
        if (match('-')) negative = true;
        else if (match('+')) negative = false;

        double result = parsePrimary();

        if (match('^')) {
            double exponent = parseFactor();
            result = std::pow(result, exponent);
        }

        return negative ? -result : result;
    }

    double parsePrimary() {
        if (match('(')) {
            double result = parseExpression();
            if (!match(')')) throw std::runtime_error("Missing closing parenthesis");
            return result;
        }

        for (const auto& [name, func] : functions) {
            if (match(name)) {
                if (!match('(')) throw std::runtime_error("Expected '(' after function");
                double arg = parseExpression();
                if (!match(')')) throw std::runtime_error("Missing closing parenthesis");
                return func(arg);
            }
        }

        for (const auto& [name, value] : constants) {
            if (match(name)) {
                return value;
            }
        }

        if (match('x')) return x;
        if (std::isdigit(peek())) return parseNumber();

        throw std::runtime_error("Unexpected character");
    }

    bool implicitMultiplicationPending() {
        skipWhitespace();
        if (pos >= expr.size()) return false;

        char prev = (pos > 0) ? expr[pos-1] : '\0';
        char next = peek();

        return (std::isdigit(prev) && (next == 'x' || next == '(' || isFunctionStart())) ||
               (prev == 'x' && (std::isdigit(next) || next == '(' || isFunctionStart())) ||
               (prev == ')' && (std::isdigit(next) || next == 'x' || next == '(' || isFunctionStart()));
    }

    bool isFunctionStart() const {
        if (pos >= expr.size()) return false;
        char c = expr[pos];
        return (c == 's' && expr.substr(pos, 3) == "sin") ||
               (c == 'c' && expr.substr(pos, 3) == "cos") ||
               (c == 't' && (expr.substr(pos, 2) == "tg" || expr.substr(pos, 3) == "tan")) ||
               (c == 'l' && (expr.substr(pos, 2) == "ln" || expr.substr(pos, 2) == "lg" || expr.substr(pos, 3) == "log"));
    }

    double parseNumber() {
        skipWhitespace();
        size_t start = pos;
        bool hasDecimal = false;
        
        while (pos < expr.size()) {
            if (std::isdigit(expr[pos])) {
                pos++;
            } else if (expr[pos] == '.' && !hasDecimal) {
                hasDecimal = true;
                pos++;
            } else {
                break;
            }
        }
        
        try {
            return std::stod(expr.substr(start, pos - start));
        } catch (...) {
            throw std::runtime_error("Invalid number format");
        }
    }

    char peek() const {
        return pos < expr.size() ? expr[pos] : '\0';
    }

    bool match(char expected) {
        skipWhitespace();
        if (peek() == expected) {
            pos++;
            return true;
        }
        return false;
    }

    bool match(const std::string& expected) {
        skipWhitespace();
        if (expr.compare(pos, expected.length(), expected) == 0) {
            pos += expected.length();
            return true;
        }
        return false;
    }

    void skipWhitespace() {
        while (pos < expr.size() && std::isspace(expr[pos])) {
            pos++;
        }
    }
};