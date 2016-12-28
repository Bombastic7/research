#pragma once

#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdarg>
#include <cstdlib>
#include <cstddef>
#include <limits>
#include <type_traits>

#include <execinfo.h>

#define DEBUGNONE 0
#define DEBUGFAST 1
#define DEBUGSLOW 2
#define DEBUGGLACIAL 3


#ifndef DEBUGLEVEL
#define DEBUGLEVEL DEBUGFAST
#endif

#if DEBUGLEVEL==0
#pragma message "no asserts"
#endif

#if DEBUGLEVEL>=DEBUGFAST
#define fast_assert(cond,...) (cond) ? ((void)0) : ::mjon661::debugimpl::assertfailimpl(__FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__)
#pragma message "fast_assert"
#else
#define fast_assert(cond,...)  while(false) { (void)(cond); }
#endif

#if DEBUGLEVEL>=DEBUGSLOW
#define slow_assert(cond,...) (cond) ? ((void)0) : ::mjon661::debugimpl::assertfailimpl(__FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__)
#pragma message "slow_assert"
#else
#define slow_assert(cond,...) while(false) { (void)(cond); }
#endif

#if DEBUGLEVEL>=DEBUGGLACIAL
#define glacial_assert(cond,...) (cond) ? ((void)0) : ::mjon661::debugimpl::assertfailimpl(__FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__)
#pragma message "glacial_assert"
#else
#define glacial_assert(cond,...) while(false) { (void)(cond); }
#endif

#define gen_assert(cond,...) (cond) ? ((void)0) : ::mjon661::debugimpl::assertfailimpl(__FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__)



namespace mjon661 {
	enum {
		DebugNone = DEBUGNONE,
		DebugFast = DEBUGFAST,
		DebugSlow = DEBUGSLOW,
		DebugGlacial = DEBUGGLACIAL,
		DebugLevel = DEBUGLEVEL
	};
	
	class AssertException : public std::runtime_error {
		public:
		AssertException(const std::string& str) : runtime_error(str) {}
	};
	
	template<typename T>
	void logDebug(T const& str) {
		std::cerr << str << std::endl;
	}

	namespace debugimpl {
		template<typename = void>
		void assertfailimpl(const char* file, unsigned line, const char* func) {
			std::string filestr(file), shortfilestr;
			
			size_t p = filestr.find_last_of('/');
			shortfilestr = std::string(file).substr(p+1);
			std::string msgstr = shortfilestr + " : " + std::to_string(line) + " : " + func + "\n\n";
			
			//Include info from backtrace()
			void* stktrcfuncbuf[50];
			int stktrcsz = backtrace(stktrcfuncbuf, 50);
			char** stktrcsyms = backtrace_symbols(stktrcfuncbuf, stktrcsz);
			for(int i=0; i<stktrcsz; i++) {
				msgstr += stktrcsyms[i];
				msgstr += "\n";
			}
			free(stktrcsyms);
			
			throw AssertException(msgstr);
		}
		
		template<typename = void>
		void assertfailimpl(const char* file, unsigned line, const char* func, const char* fmt, ...) {
			std::string filestr(file), shortfilestr;
			
			size_t p = filestr.find_last_of('/');
			shortfilestr = std::string(file).substr(p+1);
			
			char msgbuf[128];
			va_list args;
			va_start (args, fmt);
			vsnprintf(msgbuf, 128, fmt, args);
			va_end (args);

			std::string msgstr = shortfilestr + " : " + std::to_string(line) + " : " + func + " : " + msgbuf + "\n\n";
			
			//Include info from backtrace()
			void* stktrcfuncbuf[50];
			int stktrcsz = backtrace(stktrcfuncbuf, 50);
			char** stktrcsyms = backtrace_symbols(stktrcfuncbuf, stktrcsz);
			for(int i=0; i<stktrcsz; i++) {
				msgstr += stktrcsyms[i];
				msgstr += "\n";
			}
			free(stktrcsyms);
			
			throw AssertException(msgstr);
		}
	}
	
	namespace overflow {
		template<typename T>
		inline bool mul(T a, T b) {
			T result;
			return __builtin_mul_overflow(a, b, &result); //gcc builtin
		}
		
		template<typename T>
		inline bool sub(T a, T b) {
			T result;
			return __builtin_sub_overflow(a, b, &result);
		}
		
		template<typename S, typename T>
		constexpr bool trunc(S);
		
		template<>
		constexpr bool trunc<size_t, unsigned>(size_t src) {
			return src >= std::numeric_limits<unsigned>::max();
		}
		
		template<>
		constexpr bool trunc<size_t, uint64_t>(size_t src) {
			return src >= std::numeric_limits<uint64_t>::max();
		}

		template<typename T>
		constexpr bool static_mul(T, T);
		//builtin is probably faster at runtime, so both implementations included.
		
		template<>
		constexpr bool static_mul<int>(int a, int b) {
			static_assert(sizeof(int) * 2 <= sizeof(long), "");
			return a * b != static_cast<long>(a) * b;
		}
		
		template<>
		constexpr bool static_mul<unsigned>(unsigned a, unsigned b) {
			static_assert(sizeof(int) * 2 <= sizeof(long), "");
			return a * b != static_cast<unsigned long>(a) * b;
		}
		
		template<typename T>
		constexpr bool static_pow(T, T);
		
		template<typename T>
		constexpr bool static_pow_impl(T, T, unsigned, T);
		
		template<>
		constexpr bool static_pow_impl<unsigned>(unsigned tot, unsigned b, unsigned n, unsigned e) {
			return n == e+1 ? true : static_mul(tot, b) && static_pow_impl(tot*b, b, n+1, e);
		}
		
		template<>
		constexpr bool static_pow<unsigned>(unsigned a, unsigned b) {
			return static_pow_impl<unsigned>(1, a, 0, b);
		}
	}	
}
