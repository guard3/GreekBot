#include "Utils.h"
#include <array>
#include <cstdio>
#include <fstream>
#include <thread>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <zlib.h>
#if   defined _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif defined __APPLE__
#include <mach-o/dyld.h>
#endif
/* ================================================================================================================== */
namespace net = boost::asio;
/* ========== A custom io_context that runs on one strand and never runs out of work ================================ */
static auto printer_ioc = [] {
	class _ : public net::io_context {
		net::executor_work_guard<net::io_context::executor_type> m_work_guard;
		std::thread m_work_thread;
	public:
		_() : net::io_context(1), m_work_guard(net::make_work_guard(*this)), m_work_thread([this] { run(); }) {}
		~_() {
			m_work_guard.reset();
			m_work_thread.join();
		}
	};
	return _();
}();
/* ========== Random engine stuff =================================================================================== */
static std::random_device g_rd;
std::mt19937    cUtils::ms_gen(g_rd());
std::mt19937_64 cUtils::ms_gen64(g_rd());
/* ================================================================================================================== */
std::uint32_t
cUtils::CRC32(std::uint32_t hash, std::string_view str) noexcept {
	return crc32(hash, reinterpret_cast<const Byte*>(str.data()), str.size());
}
/* ================================================================================================================== */
struct printer {
	std::FILE  *m_strm;
	const char *m_tag;
	std::string m_str;
	char        m_nl;
	bool        m_bWriteToLogFile;
	void operator()() const noexcept {
		/* Retrieve the current local time; This should ideally use C++20 extensions for chrono but THAT'S STILL NOT AVAILABLE, LIKE HOLY SHIT BRUH */
		char datetime[128];
		const std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		const std::tm* p = std::localtime(&t);
		if (!p || 0 == std::strftime(datetime, 128, "%d/%b/%Y %T [", p))
			std::strcpy(datetime, "                     [");
		/* Print supplied datetime and strings */
		std::fputs(datetime,      m_strm);
		std::fputs(m_tag,         m_strm);
		std::fputs("] ",          m_strm);
		std::fputs(m_str.c_str(), m_strm);
		std::fputc(m_nl,          m_strm);
		/* If the tag is "LOG", print to the log file too */
		if (m_bWriteToLogFile) {
			/* Make sure the log file is open */
			static std::ofstream logfile;
			if (!logfile.is_open()) {
				logfile.open(cUtils::GetExecutablePath().replace_extension("log"), std::ios::app);
				if (!logfile)
					return;
			}
			/* Print; std::endl flushes the stream to file */
			logfile << datetime << m_tag << "] " << m_str << std::endl;
			if (!logfile)
				logfile.close();
		}
	}
};
/* ================================================================================================================== */
void
cUtils::print_err(std::string str, char nl) noexcept try {
	net::post(printer_ioc, printer{ stderr, "ERR", std::move(str), nl, true });
} catch (...) {}
void
cUtils::print_log(std::string str, char nl) noexcept try {
	net::post(printer_ioc, printer{ stdout, "LOG", std::move(str), nl, true });
} catch (...) {}
void
cUtils::print_msg(std::string str, char nl) noexcept try {
	net::post(printer_ioc, printer{ stdout, "MSG", std::move(str), nl, false });
} catch (...) {}
void
cUtils::print_dbg(std::string str, char nl) noexcept try {
	net::post(printer_ioc, printer{ stdout, "DBG", std::move(str), nl, false });
} catch (...) {}
/* ================================================================================================================== */
std::string
cUtils::PercentEncode(std::string_view sv) {
	/* A lookup table to check if a character is unreserved or not */
	static const bool unreserved_char[256] {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	/* Reserve at most 3 times as many characters for the output string */
	std::string result;
	result.reserve(sv.size() * 3);
	/* Parse the input */
	for (unsigned char c : sv) {
		if (unreserved_char[c])
			result.push_back(static_cast<char>(c));
		else {
			static const char hex[] = "0123456789ABCDEF";
			result.push_back('%');
			result.push_back(hex[c >> 4]);
			result.push_back(hex[c & 15]);
		}
	}
	/* Return result */
	result.shrink_to_fit();
	return result;
}
/* ================================================================================================================== */
[[noreturn]]
static void throw_exception() {
	throw std::invalid_argument("Input string is not a valid ISO 8601 timestamp");
}
/* ================================================================================================================== */
std::chrono::sys_time<std::chrono::milliseconds>
cUtils::ParseISOTimestamp(std::string_view sv) {
	using namespace std::chrono;
	/* Check that input string is at least as long as 'YYYYMMDDThhmmss' */
	if (sv.size() < 15) throw_exception();
	/* Variables */
	uint8_t dgt[9]{};                  // An array of digits converted from string chars
	uint8_t i = 0;                     // A temporary digit for when we don't want to modify the array
	const char* str = sv.data();       // The beginning of the input string or substring
	const char* end = str + sv.size(); // Τhe end of the input string
	unsigned h = 0, m = 0, s = 0;      // The hours, minutes and seconds as integers
	size_t len = 0;                    // The length of the remaining string
	bool bExtendedDate = false;        // Is the date in extended YYYY-MM-DD format?
	bool bExtendedTime = false;        // Is the time in extended hh:mm:ss format?
	bool bPlus         = false;        // Is the timezone section + or - ?
	/* Parse year as digits */
	if ((dgt[0] = str[0] - '0') > 9 ||
	    (dgt[1] = str[1] - '0') > 9 ||
	    (dgt[2] = str[2] - '0') > 9 ||
	    (dgt[3] = str[3] - '0') > 9  ) throw_exception();
	/* Parse the rest of the date */
	if (str[4] == '-') {
		bExtendedDate = true;
		if ((dgt[4] = str[5] - '0') > 9 ||
		    (dgt[5] = str[6] - '0') > 9 || str[7] != '-' ||
		    (dgt[6] = str[8] - '0') > 9 ||
		    (dgt[7] = str[9] - '0') > 9  ) throw_exception();
		str += 10;
	} else if ((dgt[4] = str[4] - '0') > 9 ||
	           (dgt[5] = str[5] - '0') > 9 ||
	           (dgt[6] = str[6] - '0') > 9 ||
	           (dgt[7] = str[7] - '0') > 9  ) throw_exception();
	else str += 8;
	/* Create a date from digits */
	year_month_day ymd {
		 year(dgt[3] + 10 * dgt[2] + 100 * dgt[1] + 1000 * dgt[0]),
		month(dgt[5] + 10 * dgt[4]),
		  day(dgt[7] + 10 * dgt[6])
	};
	/* Check that the date is valid and that the T separator exists */
	if (!ymd.ok() || *str++ != 'T') throw_exception();
	/* Parse hour */
	if ((dgt[0] = str[0] - '0') > 9 ||
	    (dgt[1] = str[1] - '0') > 9  ) throw_exception();
	/* Parse the rest of the time */
	len = end - str;
	dgt[6] = dgt[7] = 0;
	if (str[2] == ':' && len >= 8) {
		bExtendedTime = true;
		if ((dgt[2] = str[3] - '0') > 9 ||
		    (dgt[3] = str[4] - '0') > 9 || str[5] != ':' ||
		    (dgt[4] = str[6] - '0') > 9 ||
		    (dgt[5] = str[7] - '0') > 9  ) throw_exception();
		str += 8;
		/* Check if there are decimal digits and parse milliseconds */
		if (end - str > 1) {
			if (str[0] == '.' && (i = str[1] - '0') < 10) {
				dgt[6] = i;
				str += 2;
				if (end - str > 0) {
					if ((i = *str - '0') < 10) {
						dgt[7] = i;
						str++;
						if (end - str > 0) {
							if ((i = *str - '0') < 10) {
								dgt[8] = i;
								/* Advance str to skip any remaining digits */
								while (++str < end)
									if ((i = *str - '0') > 9)
										break;
		}	}	}	}	}	}
	} else if (len >= 6) {
		if ((dgt[2] = str[2] - '0') > 9 ||
		    (dgt[3] = str[3] - '0') > 9 ||
		    (dgt[4] = str[4] - '0') > 9 ||
		    (dgt[5] = str[5] - '0') > 9  ) throw_exception();
		str += 6;
	} else {
		throw_exception();
	}
	/* Make sure that the parsed date and time use the same format */
	if (bExtendedDate != bExtendedTime) throw_exception();
	/* Check the range of hours, minutes and seconds */
	if ((h = 10 * dgt[0] + dgt[1]) > 23 ||
	    (m = 10 * dgt[2] + dgt[3]) > 59 ||
	    (s = 10 * dgt[4] + dgt[5]) > 59  ) throw_exception();
	/* Save the time point so far */
	auto result = sys_days(ymd) + hours(h) + minutes(m) + seconds(s) + milliseconds(100 * dgt[6] + 10 * dgt[7] + dgt[8]);
	/* Reset digits for hours and minutes */
	dgt[0] = dgt[1] = dgt[2] = dgt[3] = 0;
	/* Parse optional timezone */
	len = end - str;
	if (len > 0) {
		switch (*str) {
			case 'Z':
				if (len == 1) break;
				throw_exception();
			case '+':
				bPlus = true;
			case '-':
				if (bExtendedTime) {
					if (len == 6)
						if ((dgt[0] = str[1] - '0') < 10 &&
						    (dgt[1] = str[2] - '0') < 10 && str[3] == ':' &&
						    (dgt[2] = str[4] - '0') < 10 &&
						    (dgt[3] = str[5] - '0') < 10  ) break;
				} else {
					if (len == 5) {
						if ((dgt[2] = str[3] - '0') > 9 ||
						    (dgt[3] = str[4] - '0') > 9  ) throw_exception();
						len = 3;
					}
					if (len == 3)
						if ((dgt[0] = str[1] - '0') < 10 &&
						    (dgt[1] = str[2] - '0') < 10  ) break;
				}
			default:
				throw_exception();
		}
	}
	/* Save timezone offset */
	if ((h = 10 * dgt[0] + dgt[1]) > 23 ||
	    (m = 10 * dgt[2] + dgt[3]) > 59  ) throw_exception();
	auto tz = hours(h) + minutes(m);
	/* Return final result */
	return bPlus ? result - tz : result + tz;
}
/* ================================================================================================================== */
std::string_view
cUtils::GetOS() noexcept {
#ifdef _WIN32
	return "Windows";
#elif defined __APPLE__ || defined __MACH__
	return "macOS";
#elif defined __linux__
	return "Linux";
#elif defined __FreeBSD__
	return "FreeBSD";
#else
	return "Unix";
#endif
}
/* ================================================================================================================== */
std::filesystem::path
cUtils::GetExecutablePath() {
#if defined _WIN32
	std::vector<WCHAR> szPath(MAX_PATH);
	for (DWORD dwLen;;) {
		/* Retrieve the fully qualified path of the executable */
		dwLen = GetModuleFileNameW(NULL, szPath.data(), (DWORD)szPath.size());
		/* If the length is 0, an error occured */
		if (dwLen == 0) {
			const DWORD dwMsg = GetLastError();
			LPSTR lpszMsg = NULL;
			try {
				constexpr DWORD FORMAT_FLAGS = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
				if (0 == FormatMessageA(FORMAT_FLAGS, NULL, dwMsg, 0, (LPSTR)&lpszMsg, 256, NULL))
					throw std::runtime_error("Cannot find a suitable filepath.");
				throw std::runtime_error(lpszMsg);
			} catch (...) {
				LocalFree(lpszMsg);
				throw;
			}
		}
		/* If the length is smaller than the total vector size, then the full path has been retrieved */
		if (dwLen < szPath.size()) {
			szPath.resize(dwLen);
			break;
		}
		/* Otherwise, increase the vector size and retry */
		szPath.resize(dwLen * 2);
	}
	return std::filesystem::path(szPath.begin(), szPath.end());
#elif defined __APPLE__
	std::vector<char> path(256);
	/* Retrieve a path to the executable. */
	if (uint32_t size = 256; _NSGetExecutablePath(path.data(), &size) < 0) {
		/* If the path vector isn't large enough, 'size' is updated to the required size */
		path.resize(size);
		/* Retry */
		_NSGetExecutablePath(path.data(), &size);
	}
	return std::filesystem::canonical(path.data());
#else
	return std::filesystem::canonical("/proc/self/exe");
#endif
}