#ifndef _RHACT_LOGGER_HPP_
#define _RHACT_LOGGER_HPP_

#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <string>
#include <iterator>

#define LOG_FUNC_GEN(name, lv, method, prefix, suffix) \
    template <typename... Args> \
    static void name(const Args&... args) { \
        if(level > LOGLEVEL::lv ) { return; } \
        std::stringstream ss; \
        stringify(ss,args...); \
        std::method << prefix << ss.str() << suffix << std::endl; \
    }

namespace rhakt {

template <typename T>
std::ostream& operator<< (std::ostream& out, const std::vector<T>& v) {
    if(!v.empty()) {
        out << '[';
        std::copy(v.begin(), v.end(), std::ostream_iterator<T>(out, ", "));
        out << "\b\b]";
    }
    return out;
}

enum struct LOGLEVEL : int {
    DEBUG = 0,
    INFO,
    WARN,
    ERR,
    LOG
};

struct logger {
private:	
    static LOGLEVEL level;
    
    static void stringify(std::stringstream& ss) {}

    template <typename T, typename... Args>
    static void stringify(std::stringstream& ss, const T& value, const Args&... args) {
        ss << value;
        stringify(ss, args...);
    }

public:
    static void setLevel(const LOGLEVEL lv){ level = lv; }

    LOG_FUNC_GEN(debug, DEBUG,  cout,   "[DEBUG] ", "")
    LOG_FUNC_GEN(info,  INFO,   cout,   "[INFO] ",  "");
    LOG_FUNC_GEN(warn,  WARN,   cout,   "[WARN] ",  "");
    LOG_FUNC_GEN(error, ERR,    cerr,   "[ERROR] ", "");
    LOG_FUNC_GEN(log,   LOG,    cout,   "", "");
};

} // namespace rhakt

#endif