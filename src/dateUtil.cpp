
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"    

namespace datetime {

// -----------------------------------------------------------------------

int getYear() 
{
  return boost::gregorian::day_clock::local_day().year();
}

// -----------------------------------------------------------------------

int getMonth()
{
  return boost::gregorian::day_clock::local_day().month();
}

// -----------------------------------------------------------------------

int getDay()
{
  return boost::gregorian::day_clock::local_day().day();
}

// -----------------------------------------------------------------------

int getDayOfWeek()
{
  return boost::gregorian::day_clock::local_day().day_of_week();
}

// -----------------------------------------------------------------------

int getHour()
{
  return boost::posix_time::second_clock::local_time().time_of_day().hours();
}

// -----------------------------------------------------------------------

int getMinute()
{
  return boost::posix_time::second_clock::local_time().time_of_day().minutes();
}

// -----------------------------------------------------------------------

int getSecond()
{
  return boost::posix_time::second_clock::local_time().time_of_day().seconds();
}

// -----------------------------------------------------------------------

int getMs()
{
  return boost::posix_time::second_clock::local_time().time_of_day().fractional_seconds();
}

}
