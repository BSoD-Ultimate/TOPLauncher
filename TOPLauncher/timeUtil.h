#pragma once
#include <chrono>

namespace TOPLauncher
{
	namespace util
	{
		class Time
		{
			// timestamp format is compatible with SQLite: 2000-01-01T00:00:00.000+08:00
            // http://sqlite.org/lang_datefunc.html
		public:
			struct TimeSpan
			{

			};
			struct TimeZone
			{
				enum Direction
				{
					Positive, Negative
				};
				int offset; // in seconds

				TimeZone() : offset(0) {}
				// legal time zone: -12:00 ~ +14:00
				explicit TimeZone(int seconds) : offset(seconds)
				{
					assert(offset >= -43200 && offset <= 50400);
				}
				TimeZone(Direction d, int hour, int minute)
					: offset(d == Positive ? ((hour * 60 + minute) * 60) : -((hour * 60 + minute) * 60))
				{
					assert(offset >= -43200 && offset <= 50400);
				}

				Direction direction() const
				{
					return offset >= 0 ? Positive : Negative;
				}
				int Hour() const
				{
					return abs(offset / (60 * 60));
				}
				int Minute() const
				{
					return abs(offset / 60) - 60 * Hour();
				}

                friend bool operator==(const TimeZone& t1, const TimeZone& t2);
                friend bool operator!=(const TimeZone& t1, const TimeZone& t2);
			};

		public:
			Time();
			Time(int64_t localTime, int timezoneOffset);
			Time(int64_t localTime, TimeZone timezone);
            Time(int year, int month, int day, int hour, int minute, int second, int millisecond, TimeZone timezone);
			explicit Time(int64_t utcTime);
			~Time();

			// local time (with time zone offset applied comparing with UTC) 
			int Year() const;
			int Month() const; // 1 ~ 12
			int Day() const;
			int DayOfWeek() const; // Sunday is the first day , value range from 1 to 7
			int Hour() const;
			int Minute() const;
			int Second() const;
			int Millisecond() const;
            int64_t TotalSeconds() const;
			int64_t TotalMilliseconds() const;
			std::wstring ToTimestamp() const;
			std::wstring ToUTCTimestamp() const;
			int ToLocalTime(tm* time) const;
			int ToUTCTime(tm* time) const;

			// read / write 
			TimeZone GetTimeZone() const;
			void SetTimeZone(TimeZone newTimeZone);

			Time ToUTC() const;

			// local time / utc time since epoch (1970-01-01 00:00:00)
			static Time FromLocalTime(int64_t localTime, int timezoneOffset);
			static Time FromUTCTime(int64_t utcTime);
			static Time FromTimestamp(const std::wstring& timestamp);

			static Time Now();
			static Time UTCNow();

			static TimeZone GetLocalTimeZone();
			static Time GetDate(int year, int month, int day, TimeZone timezone);

            static bool IsSameDay(const Time& t1, const Time& t2);
		private:
			static double GetLocalTimeZoneOffset();
		private:
			TimeZone m_timeZone;
			std::chrono::system_clock::time_point m_timePoint; // UTC

        public:
            friend bool operator>(const Time& t1, const Time& t2);
            friend bool operator>=(const Time& t1, const Time& t2);
            friend bool operator<(const Time& t1, const Time& t2);
            friend bool operator<=(const Time& t1, const Time& t2);
            friend bool operator==(const Time& t1, const Time& t2);
            friend bool operator!=(const Time& t1, const Time& t2);

            // apply a period of time on current time point
            template <class Rep, class Period>
            friend Time operator+(const Time& t, std::chrono::duration<Rep, Period> duration)
            {
                int64_t point = t.TotalMilliseconds();
                std::chrono::milliseconds newPoint = std::chrono::milliseconds(point) + duration;
                return Time(newPoint.count(), t.GetTimeZone());
            }
            template <class Rep, class Period>
            friend Time operator-(const Time& t, std::chrono::duration<Rep, Period> duration)
            {
                int64_t point = t.TotalMilliseconds();
                std::chrono::milliseconds newPoint = std::chrono::milliseconds(point) - duration;
                return Time(newPoint.count(), t.GetTimeZone());
            }
            template <class Rep, class Period>
            Time& operator+=(std::chrono::duration<Rep, Period> duration)
            {
                *this = *this + duration;
                return *this;
            }
            template <class Rep, class Period>
            Time& operator-=(std::chrono::duration<Rep, Period> duration)
            {
                *this = *this - duration;
                return *this;
            }

            // calculate time span
            //template <class Rep, class Period>
            //friend std::chrono::duration<Rep, Period> operator-(const Time& t1, const Time& t2)
            //{
            //    return t1.m_timePoint - t2.m_timePoint;
            //}

		};



        

	}
}
