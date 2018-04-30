#include "stdafx.h"
#include "timeUtil.h"

#include <cstdio>
#include <ctime>

namespace TOPLauncher
{
	namespace util
	{
		Time::Time()
		{
		}

		Time::Time(int64_t localTime, int timezoneOffset)
			: m_timeZone(timezoneOffset)
			, m_timePoint(std::chrono::milliseconds(localTime) - std::chrono::seconds(m_timeZone.offset))
		{
		}

		Time::Time(int64_t localTime, TimeZone timezone)
			: m_timeZone(timezone)
			, m_timePoint(std::chrono::milliseconds(localTime) - std::chrono::seconds(m_timeZone.offset))
		{
		}

        Time::Time(int year, int month, int day, int hour, int minute, int second, int millisecond, TimeZone timezone)
            : m_timeZone(timezone)
        {
            struct tm timeInfo = { 0 };

            timeInfo.tm_year = year;
            timeInfo.tm_mon = month;
            timeInfo.tm_mday = day;
            timeInfo.tm_hour = hour;
            timeInfo.tm_min = minute;
            timeInfo.tm_sec = second;

            timeInfo.tm_year -= 1900;
            timeInfo.tm_mon -= 1;

            time_t localTime = mktime(&timeInfo);

            int64_t timeMilliseconds = (localTime + int(GetLocalTimeZoneOffset())) * 1000 + millisecond;

            m_timePoint += std::chrono::milliseconds(timeMilliseconds);
            m_timePoint -= std::chrono::seconds(m_timeZone.offset);
        }

		Time::Time(int64_t utcTime)
			: m_timeZone(0)
			, m_timePoint(std::chrono::milliseconds(utcTime))
		{
		}

		Time::~Time()
		{
		}

		int Time::Year() const
		{
			auto localTime = m_timePoint + std::chrono::seconds(m_timeZone.offset);
			time_t time = std::chrono::system_clock::to_time_t(localTime);
			struct tm gmTime = { 0 };

			gmtime_s(&gmTime, &time);

			return gmTime.tm_year + 1900;
		}

		int Time::Month() const
		{
			auto localTime = m_timePoint + std::chrono::seconds(m_timeZone.offset);
			time_t time = std::chrono::system_clock::to_time_t(localTime);
			struct tm gmTime = { 0 };

            gmtime_s(&gmTime, &time);

			return gmTime.tm_mon + 1;
		}

		int Time::Day() const
		{
			auto localTime = m_timePoint + std::chrono::seconds(m_timeZone.offset);
			time_t time = std::chrono::system_clock::to_time_t(localTime);
			struct tm gmTime = { 0 };

            gmtime_s(&gmTime, &time);

			return gmTime.tm_mday;
		}

		int Time::DayOfWeek() const
		{
			auto localTime = m_timePoint + std::chrono::seconds(m_timeZone.offset);
			time_t time = std::chrono::system_clock::to_time_t(localTime);
			struct tm gmTime = { 0 };

            gmtime_s(&gmTime, &time);

			return gmTime.tm_wday + 1;
		}

		int Time::Hour() const
		{
			auto localTime = m_timePoint + std::chrono::seconds(m_timeZone.offset);
			time_t time = std::chrono::system_clock::to_time_t(localTime);
			struct tm gmTime = { 0 };

            gmtime_s(&gmTime, &time);

			return gmTime.tm_hour;
		}

		int Time::Minute() const
		{
			auto localTime = m_timePoint + std::chrono::seconds(m_timeZone.offset);
			time_t time = std::chrono::system_clock::to_time_t(localTime);
			struct tm gmTime = { 0 };

            gmtime_s(&gmTime, &time);

			return gmTime.tm_min;
		}

		int Time::Second() const
		{
			auto localTime = m_timePoint + std::chrono::seconds(m_timeZone.offset);
			time_t time = std::chrono::system_clock::to_time_t(localTime);
			struct tm gmTime = { 0 };

            gmtime_s(&gmTime, &time);

			return gmTime.tm_sec;
		}

		int Time::Millisecond() const
		{
			using Sec = std::chrono::seconds;
			using Ms = std::chrono::milliseconds;
			using std::chrono::duration_cast;
			auto localTime = m_timePoint + std::chrono::seconds(m_timeZone.offset);

			return int((duration_cast<Ms>(localTime.time_since_epoch()) -
				duration_cast<Sec>(localTime.time_since_epoch())).count());
		}

        int64_t Time::TotalSeconds() const
        {
            return TotalMilliseconds() / 1000;
        }

		int64_t Time::TotalMilliseconds() const
		{
			auto localTime = m_timePoint + std::chrono::seconds(m_timeZone.offset);
			return std::chrono::duration_cast<std::chrono::milliseconds>(localTime.time_since_epoch()).count();
		}

		Time::TimeZone Time::GetTimeZone() const
		{
			return m_timeZone;
		}

		void Time::SetTimeZone(TimeZone newTimeZone)
		{
			m_timeZone = newTimeZone;
		}

		Time Time::ToUTC() const
		{
			Time newTime(*this);
			newTime.SetTimeZone(TimeZone());
			return newTime;
		}

		std::wstring Time::ToTimestamp() const
		{
			// SQLite compatible format 2000-01-01T00:00:00.000+08:00
            wchar_t tsBuf[200];

            swprintf_s(tsBuf, L"%04d-%02d-%02dT%02d:%02d:%02d.%03d%c%02d:%02d",
                Year(), Month(), Day(), Hour(), Minute(), Second(), Millisecond(),
                (m_timeZone.direction() == TimeZone::Positive ? L'+' : L'-'), m_timeZone.Hour(), m_timeZone.Minute());

			return tsBuf;
		}

        std::wstring Time::ToUTCTimestamp() const
		{
			auto utcTime = m_timePoint;
			time_t time = std::chrono::system_clock::to_time_t(utcTime);
			struct tm gmTime = { 0 };
			

			// SQLite compatible format 2000-01-01T00:00:00.000+08:00
			wchar_t tsBuf[200];

            gmtime_s(&gmTime, &time);
            swprintf_s(tsBuf, L"%04d-%02d-%02dT%02d:%02d:%02d.%03d%c%02d:%02d",
				gmTime.tm_year + 1900, gmTime.tm_mon + 1, gmTime.tm_mday,
				gmTime.tm_hour, gmTime.tm_min, gmTime.tm_sec, Millisecond(),
				L'+', 0, 0);

			return tsBuf;
		}


		int Time::ToLocalTime(tm* time) const
		{
			time_t utcTime = std::chrono::system_clock::to_time_t(m_timePoint);

            return localtime_s(time, &utcTime);	
		}

		int Time::ToUTCTime(tm* time) const
		{
			time_t utcTime = std::chrono::system_clock::to_time_t(m_timePoint);

            return gmtime_s(time, &utcTime);
		}

		Time Time::FromLocalTime(int64_t localTime, int timezoneOffset)
		{
			return Time(localTime, timezoneOffset);
		}

		Time Time::FromUTCTime(int64_t utcTime)
		{
			return Time(utcTime);
		}

		Time Time::FromTimestamp(const std::wstring& timestamp)
		{
			struct tm timeInfo = { 0 };
			struct {
				int millisecond;
				wchar_t timezone_direction;
				int timezone_hours;
				int timezone_minutes;
			}exInfo = { 0 };

            int ret = swscanf_s(timestamp.c_str(), L"%04d-%02d-%02dT%02d:%02d:%02d.%03d%c%02d:%02d",
                &timeInfo.tm_year,
                &timeInfo.tm_mon,
                &timeInfo.tm_mday,
                &timeInfo.tm_hour,
                &timeInfo.tm_min,
                &timeInfo.tm_sec,
                &exInfo.millisecond,
                &exInfo.timezone_direction, 1,
                &exInfo.timezone_hours,
                &exInfo.timezone_minutes);

            if (ret == EOF || ret == 0)
            {
                return Time();
            }

			timeInfo.tm_year -= 1900;
			timeInfo.tm_mon -= 1;
			time_t localTime = mktime(&timeInfo);

            // mktime function regards time in the incoming tm struct as local time,
            // we input utc time here, so we need to apply the timezone offset set before to get the right time value.
			int64_t timeMilliseconds = (localTime + int(GetLocalTimeZoneOffset())) * 1000 + exInfo.millisecond;

			return Time(timeMilliseconds,
				TimeZone(exInfo.timezone_direction == L'+' ? TimeZone::Positive : TimeZone::Negative,
					exInfo.timezone_hours, exInfo.timezone_minutes));
		}

		Time Time::Now()
		{
			Time newTime;
			newTime.m_timePoint = std::chrono::system_clock::now();
			newTime.m_timeZone = TimeZone(int(GetLocalTimeZoneOffset()));
			return newTime;
		}

		Time Time::UTCNow()
		{
			Time newTime;
			newTime.m_timePoint = std::chrono::system_clock::now();
			return newTime;
		}

		Time::TimeZone Time::GetLocalTimeZone()
		{
			return TimeZone(int(GetLocalTimeZoneOffset()));
		}

		Time Time::GetDate(int year, int month, int day, TimeZone timezone)
		{
			assert(month > 0 && month <= 12);
			auto CheckDayCount = [year](int month, int day)
			{
				switch (month)
				{
				case 1: case 3: case 5: case 7: case 8: case 10: case 12:
					assert(day > 0 && day <= 31);
					break;
				case 4: case 6: case 9: case 11:
					assert(day > 0 && day <= 30);
					break;
				default:
					break;
				}
				if (month == 2)
				{
					if ((year - 2000) % 4 == 0)
					{
						assert(day > 0 && day <= 29);
					}
					else
					{
						assert(day > 0 && day < 28);
					}
				}
			};
			CheckDayCount(month, day);

			wchar_t tsBuf[200];

			swprintf_s(tsBuf, L"%04d-%02d-%02dT00:00:00.000%c%02d:%02d",
				year, month, day,
				timezone.direction() == TimeZone::Positive ? '+' : '-',
				timezone.Hour(), timezone.Minute());

			return Time::FromTimestamp(tsBuf);
		}

        bool Time::IsSameDay(const Time& t1, const Time& t2)
        {
            return t1.GetTimeZone() == t2.GetTimeZone() &&
                t1.Year() == t2.Year() &&
                t1.Month() == t2.Month() && 
                t1.Day() == t2.Day();
        }

		double Time::GetLocalTimeZoneOffset()
		{
			time_t now = time(NULL);
			tm gm = { 0 }, loc = { 0 };

            gmtime_s(&gm, &now);
            localtime_s(&loc, &now);

			time_t gmt = mktime(&gm);

			time_t local = mktime(&loc);

			return difftime(local, gmt);
		}
        bool operator>(const Time& t1, const Time& t2)
        {
            return t1.ToUTC().TotalMilliseconds() > t2.ToUTC().TotalMilliseconds();
        }
        bool operator>=(const Time& t1, const Time& t2)
        {
            return t1.ToUTC().TotalMilliseconds() >= t2.ToUTC().TotalMilliseconds();
        }
        bool operator<(const Time& t1, const Time& t2)
        {
            return t1.ToUTC().TotalMilliseconds() < t2.ToUTC().TotalMilliseconds();
        }
        bool operator<=(const Time& t1, const Time& t2)
        {
            return t1.ToUTC().TotalMilliseconds() <= t2.ToUTC().TotalMilliseconds();
        }
        bool operator==(const Time& t1, const Time& t2)
        {
            return t1.ToUTC().TotalMilliseconds() == t2.ToUTC().TotalMilliseconds();
        }
        bool operator!=(const Time& t1, const Time& t2)
        {
            return t1.ToUTC().TotalMilliseconds() != t2.ToUTC().TotalMilliseconds();
        }
        bool operator==(const Time::TimeZone & t1, const Time::TimeZone & t2)
        {
            return t1.offset == t2.offset;
        }
        bool operator!=(const Time::TimeZone & t1, const Time::TimeZone & t2)
        {
            return t1.offset != t2.offset;
        }
}
}
