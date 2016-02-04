// rechor project
// util.hpp

#ifndef _RHACT_RECHOR_UTIL_HPP_
#define _RHACT_RECHOR_UTIL_HPP_

#include <array>

namespace rechor {

    /* console output utility */
    template <typename T>
    std::ostream& operator<< (std::ostream& out, const std::vector<T>& v) {
        if (!v.empty()) {
            out << '[';
            std::copy(v.begin(), v.end(), std::ostream_iterator<T>(out, ", "));
            out << "\b\b]";
        }
        return out;
    }

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

    



} // namespace rechor

#endif
