#ifndef __TIMESTAMP_H
#define __TIMESTAMP_H

#include <string>
#include "common.h"

BEGIN_NS(base)

class BASE_API Timestamp
{
public:
	///
	/// Constucts an invalid Timestamp.
	///
	Timestamp() : m_microSecondsSinceEpoch_(Timestamp::getNowPointTime())
	{
	}

	///
	/// Constucts a Timestamp at specific time
	///
	/// @param microSecondsSinceEpoch
	explicit Timestamp(const int64_t microSecondsSinceEpoch);

	Timestamp& operator+=(Timestamp lhs)
	{
		this->m_microSecondsSinceEpoch_ += lhs.m_microSecondsSinceEpoch_;
		return *this;
	}

	Timestamp& operator+=(const int64_t lhs)
	{
		this->m_microSecondsSinceEpoch_ += lhs;
		return *this;
	}

	Timestamp& operator-=(const Timestamp& lhs)
	{
		this->m_microSecondsSinceEpoch_ -= lhs.m_microSecondsSinceEpoch_;
		return *this;
	}

	Timestamp& operator-=(const int64_t lhs)
	{
		this->m_microSecondsSinceEpoch_ -= lhs;
		return *this;
	}

	void swap(Timestamp& that)
	{
		std::swap(m_microSecondsSinceEpoch_, that.m_microSecondsSinceEpoch_);
	}

	// default copy/assignment/dtor are Okay

	std::string toString() const;
	std::string toFormattedString(const bool showMicroseconds = true) const;

	bool valid() const { return m_microSecondsSinceEpoch_ > 0; }

	// for internal usage.
	int64_t microSecondsSinceEpoch() const { return m_microSecondsSinceEpoch_; }
	time_t secondsSinceEpoch() const
	{
		return static_cast<time_t>(m_microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
	}

	///
	/// Get time of now.
	///
	static Timestamp now();
	static Timestamp invalid();

	static int64_t getNowPointTime();

	static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
	int64_t     m_microSecondsSinceEpoch_;
};

inline bool operator<(const Timestamp& lhs, const Timestamp& rhs)
{
	return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator>(const Timestamp& lhs, const Timestamp& rhs)
{
	return rhs < lhs;
}

inline bool operator<=(const Timestamp& lhs, const Timestamp& rhs)
{
	return !(lhs > rhs);
}

inline bool operator>=(const Timestamp& lhs, const Timestamp& rhs)
{
	return !(lhs < rhs);
}

inline bool operator==(const Timestamp& lhs, const Timestamp& rhs)
{
	return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

inline bool operator!=(const Timestamp& lhs, const Timestamp& rhs)
{
	return !(lhs == rhs);
}

	///
	/// Gets time difference of two timestamps, result in seconds.
	///
	/// @param high, low
	/// @return (high-low) in seconds
	/// @c double has 52-bit precision, enough for one-microsecond
	/// resolution for next 100 years.
inline double timeDifference(const Timestamp& high, const Timestamp& low)
{
	int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
	return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

	///
	/// Add @c seconds to given timestamp.
	///
	/// @return timestamp+seconds as Timestamp
	///
inline Timestamp addTime(const Timestamp& timestamp, const int64_t microseconds)
{
	//int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
	return Timestamp(timestamp.microSecondsSinceEpoch() + microseconds);
}

END_NS(base)
using namespace base;
#endif