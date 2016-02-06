// rechor project
// util.hpp

#ifndef _RHACT_UTIL_HPP_
#define _RHACT_UTIL_HPP_

#include <array>
#include <fstream>

namespace rhakt {
namespace util {
    
    // make array
    template<typename T, typename ...Args>
    constexpr std::array<T, sizeof...(Args)> make_array(Args&&... args) {
        return std::array<T, sizeof...(Args)>{ static_cast<Args&&>(args)... };
    }

    // get size of std::array
    template<typename T, std::size_t N>
    constexpr std::size_t array_size(const std::array<T, N>&) { return N; }

    // get size of array[N]
    template<typename T, std::size_t N>
    constexpr std::size_t array_size(T(&)[N]) { return N; }
    
    
    /* noncopyable */
    class Noncopyable {
    protected:
        Noncopyable() = default;
        ~Noncopyable() = default;
    private:
    void operator =(const Noncopyable& src) = delete;
        Noncopyable(const Noncopyable& src) = delete;
    };

    /* load file */
    inline bool loadfile(const std::string& name, bool binary, std::string& buf) {
        std::ifstream ifs(name, binary ? std::ifstream::binary : std::ifstream::in);
        if(!ifs.is_open()){ return false; };
        if(binary) {
            ifs.seekg(0, std::ios::end);
            buf.resize(static_cast<size_t>(ifs.tellg()));
            ifs.seekg(0, std::ios::beg);
            ifs.read(&buf[0], buf.size());
        } else {
            std::ostringstream oss;
            oss << ifs.rdbuf();
            buf = oss.str();
        }
        return !ifs.bad();
    }

    /* save file */
    inline bool savefile(const std::string& name, bool binary, const char* buf, const size_t len) {
        std::ofstream ofs(name, binary ? std::ofstream::binary : std::ofstream::out);
        if(!ofs.is_open()){ return false; };
        ofs.write(buf, len);
        return !ofs.bad();
    }

    inline bool savefile(const std::string& name, bool binary, const std::string& buf) {
        return savefile(name, binary, buf.c_str(), buf.size());
    }
    


}} // namespace rhakt::util

#endif
