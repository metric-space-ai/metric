// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TIME_H
#define METRIC_NUMERIC_UTIL_TIME_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#if defined(_MSC_VER) || defined(__MINGW64_VERSION_MAJOR) || defined(__MINGW32__)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <sys/timeb.h>
#include <time.h>
#include <windows.h>
#include <winsock.h>
#else
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#endif
#include <ctime>
#include <string>

namespace mtrc::numeric {

//=================================================================================================
//
//  TIME FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name Time functions */
//@{
inline std::string getDate();
inline std::string getTime();
inline double getWcTime();
inline double getCpuTime();
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Creating a formated date string in the form YYYY-MM-DD
// \ingroup util
//
// \return Formated date string
*/
inline std::string getDate()
{
	std::time_t t;
	std::tm *localTime;
	char c[50];

	std::time(&t);
	localTime = std::localtime(&t);
	std::strftime(c, 50, "%Y-%m-%d", localTime);

	return std::string(c);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Creating a formated time and date string
// \ingroup util
//
// \return Formated time and date string in the format WEEKDAY DAY.MONTH YEAR, HOUR:MINUTES
*/
inline std::string getTime()
{
	std::time_t t;
	std::tm *localTime;
	char c[50];

	std::time(&t);
	localTime = std::localtime(&t);
	std::strftime(c, 50, "%A, %d.%B %Y, %H:%M", localTime);

	return std::string(c);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the current wall clock time in seconds.
// \ingroup util
//
// \return The current wall clock time in seconds.
*/
inline double getWcTime()
{
#ifdef WIN32
	struct _timeb timeptr;
	_ftime64_s(&timeptr);
	return (static_cast<double>(timeptr.time) + static_cast<double>(timeptr.millitm) / 1E3);
#else
	struct timeval tp;
	gettimeofday(&tp, nullptr);
	return (static_cast<double>(tp.tv_sec) + static_cast<double>(tp.tv_usec) / 1E6);
#endif
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the current CPU time in seconds.
// \ingroup util
//
// \return The current CPU time in seconds.
*/
inline double getCpuTime()
{
#ifdef WIN32
	FILETIME CreateTime, ExitTime, KernelTime, UserTime;
	SYSTEMTIME SysTime;

	if (GetProcessTimes(GetCurrentProcess(), &CreateTime, &ExitTime, &KernelTime, &UserTime) != TRUE) {
		return 0.0;
	} else {
		FileTimeToSystemTime(&UserTime, &SysTime);
		return (static_cast<double>(SysTime.wSecond) + static_cast<double>(SysTime.wMilliseconds) / 1E3);
	}
#else
	struct rusage ruse;
	getrusage(RUSAGE_SELF, &ruse);
	return (static_cast<double>(ruse.ru_utime.tv_sec) + static_cast<double>(ruse.ru_utime.tv_usec) / 1E6);
#endif
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
