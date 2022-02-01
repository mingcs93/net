#include "timestamp.h"
#include <chrono>
#include <ctime>
#include <sstream>

BEGIN_NS(base)

static_assert(sizeof(Timestamp) == sizeof(int64_t), "sizeof(Timestamp) error");

Timestamp::Timestamp(const int64_t microSecondsSinceEpoch)
	: m_microSecondsSinceEpoch_(microSecondsSinceEpoch)
{
}

std::string Timestamp::toString() const
{
    char buf[64] = { 0 };
    int64_t seconds = m_microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
    int64_t microseconds = m_microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
    snprintf(buf, sizeof(buf)-1, "%lld.%06lld", (long long int)seconds, (long long int)microseconds);
    return buf;
}

std::string Timestamp::toFormattedString(const bool showMicroseconds) const
{
	time_t seconds = static_cast<time_t>(m_microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
	struct tm tm_time;

#ifdef WIN32
	localtime_s(&tm_time, &seconds);
#else
	struct tm *ptm;
	ptm = localtime(&seconds);
	tm_time = *ptm;
#endif

	char buf[32] = { 0 };

	if (showMicroseconds)
	{
		int microseconds = static_cast<int>(m_microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
#ifdef WIN32
		_snprintf_s(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
			microseconds);
#else
		snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
			microseconds);
#endif
	}
	else
	{
#ifdef WIN32
		_snprintf_s(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
#else
		snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
#endif
	}
	
	
	return buf;
}

Timestamp Timestamp::now()
{
	std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds> now = 
		std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now());

	int64_t microSeconds = now.time_since_epoch().count();
	Timestamp time(microSeconds);
	return time;
}

Timestamp Timestamp::invalid()
{
	return Timestamp();
}

int64_t Timestamp::getNowPointTime()
{
	std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds> now = 
		std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now());

	return now.time_since_epoch().count();
}

END_NS(base)