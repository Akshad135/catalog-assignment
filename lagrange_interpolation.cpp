#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <iomanip>
#include <cctype>
#include <stdexcept>
#include <iterator>
#include <sstream>

struct Point {
    double x, y;
    Point(double x_val, double y_val) : x(x_val), y(y_val) {}
};

long long convertFromBase(const std::string &value, int base) {
    return std::stoll(value, nullptr, base);
}

// Lagrange interpolation to find the value at x = 0 (constant term)
double lagrangeInterpolation(const std::vector<Point>& points, double x) {
    double result = 0.0;
    int n = points.size();
    
    for (int i = 0; i < n; i++) {
        double term = points[i].y;
        
        // Calculate the Lagrange basis polynomial L_i(x)
        for (int j = 0; j < n; j++) {
            if (i != j) {
                term = term * (x - points[j].x) / (points[i].x - points[j].x);
            }
        }
        
        result += term;
    }
    
    return result;
}

double findConstantTerm(const std::vector<Point>& points) {
    return lagrangeInterpolation(points, 0.0);
}

int main() {
    // Read JSON from standard input
    std::string s;
    std::string line;
    
    std::cout << "Enter JSON input (press Ctrl+D on Unix/Linux or Ctrl+Z on Windows when done):" << std::endl;
    
    while (std::getline(std::cin, line)) {
        s += line + "\n";
    }
    
    if (s.empty()) {
        std::cerr << "No input provided." << std::endl;
        return 1;
    }
    
    std::map<int, std::pair<int, std::string>> entries;
    std::vector<Point> points;
    size_t pos = 0;
    
    // Parse JSON more carefully to avoid duplicates
    while (true) {
        // Find the start of a key
        size_t q1 = s.find('"', pos);
        if (q1 == std::string::npos) break;
        size_t q2 = s.find('"', q1 + 1);
        if (q2 == std::string::npos) break;
        std::string key = s.substr(q1 + 1, q2 - q1 - 1);
        
        // Skip non-numeric keys
        if (key == "keys" || key == "n" || key == "k" || key == "base" || key == "value") {
            pos = q2 + 1;
            continue;
        }
        
        if (key.empty()) {
            pos = q2 + 1;
            continue;
        }
        
        if (!std::isdigit(static_cast<unsigned char>(key[0])) && key[0] != '-') {
            pos = q2 + 1;
            continue;
        }
        
        int x;
        try { 
            x = std::stoi(key); 
        }
        catch (...) { 
            pos = q2 + 1;
            continue; 
        }
        
        if (x == 0) {
            pos = q2 + 1;
            continue;
        }
        
        // Find the opening brace for this key's object
        size_t brace = s.find('{', q2);
        if (brace == std::string::npos) break;
        
        // Find the closing brace for this object
        size_t closeBrace = s.find('}', brace);
        if (closeBrace == std::string::npos) break;
        
        std::string objectStr = s.substr(brace, closeBrace - brace + 1);
        
        // Extract base
        size_t baseKey = objectStr.find("\"base\"");
        if (baseKey == std::string::npos) {
            pos = closeBrace + 1;
            continue;
        }
        
        size_t colon = objectStr.find(':', baseKey);
        if (colon == std::string::npos) {
            pos = closeBrace + 1;
            continue;
        }
        
        size_t bq1 = objectStr.find('"', colon);
        if (bq1 == std::string::npos) {
            pos = closeBrace + 1;
            continue;
        }
        
        size_t bq2 = objectStr.find('"', bq1 + 1);
        if (bq2 == std::string::npos) {
            pos = closeBrace + 1;
            continue;
        }
        
        std::string baseStr = objectStr.substr(bq1 + 1, bq2 - bq1 - 1);
        int base;
        try {
            base = std::stoi(baseStr);
        } catch (...) {
            pos = closeBrace + 1;
            continue;
        }
        
        // Extract value
        size_t valueKey = objectStr.find("\"value\"");
        if (valueKey == std::string::npos) {
            pos = closeBrace + 1;
            continue;
        }
        
        size_t vcolon = objectStr.find(':', valueKey);
        if (vcolon == std::string::npos) {
            pos = closeBrace + 1;
            continue;
        }
        
        size_t vq1 = objectStr.find('"', vcolon);
        if (vq1 == std::string::npos) {
            pos = closeBrace + 1;
            continue;
        }
        
        size_t vq2 = objectStr.find('"', vq1 + 1);
        if (vq2 == std::string::npos) {
            pos = closeBrace + 1;
            continue;
        }
        
        std::string valueStr = objectStr.substr(vq1 + 1, vq2 - vq1 - 1);
        
        // Convert value from specified base
        long long y;
        try { 
            y = convertFromBase(valueStr, base); 
        }
        catch (const std::exception&) { 
            std::cerr << "Conversion error for x=" << x << ", value=" << valueStr << ", base=" << base << '\n'; 
            pos = closeBrace + 1;
            continue; 
        }
        
        // Only add if we haven't seen this x value before
        if (entries.find(x) == entries.end()) {
            entries[x] = {base, valueStr};
            points.emplace_back(static_cast<double>(x), static_cast<double>(y));
        }
        
        pos = closeBrace + 1;
    }
    
    if (points.empty()) {
        std::cerr << "No valid points found in the input." << std::endl;
        return 1;
    }
    
    std::cout << std::fixed << std::setprecision(10);
    // Find the constant term (secret in Shamir's Secret Sharing)
    double constantTerm = findConstantTerm(points);
    std::cout << constantTerm << std::endl;
    
    return 0;
}
