#include "stdafx.h"
#include "DateTime.hpp"

DateTime::DateTime()
{
	t = time(nullptr);
	aTm = *localtime(&t);
}

int DateTime::GetSeconds() const
{
	return aTm.tm_sec;
}

int DateTime::GetMinutes() const
{
	return aTm.tm_min;
}

int DateTime::GetHours() const
{
	return aTm.tm_hour;
}

int DateTime::GetDay() const
{
	return aTm.tm_mday;
}

int DateTime::GetMonth() const
{
	return aTm.tm_mon + 1;
}

int DateTime::GetYear() const
{
	return aTm.tm_year + 1900;
}

DateTime::string DateTime::GetSecondsString() const
{
	return (GetSeconds() < 10) ? "0" + xr_string::ToString(GetSeconds()) : xr_string::ToString(GetSeconds());
}

DateTime::string DateTime::GetMinutesString() const
{
	return (GetMinutes() < 10) ? "0" + xr_string::ToString(GetMinutes()) : xr_string::ToString(GetMinutes());
}

DateTime::string DateTime::GetHoursString() const
{
	return (GetHours() < 10) ? "0" + xr_string::ToString(GetHours()) : xr_string::ToString(GetHours());
}

DateTime::string DateTime::GetDayString() const
{
	return (GetDay() < 10) ? "0" + xr_string::ToString(GetDay()) : xr_string::ToString(GetDay());
}

DateTime::string DateTime::GetMonthString() const
{
	return (GetMonth() < 10) ? "0" + xr_string::ToString(GetMonth()) : xr_string::ToString(GetMonth());
}

DateTime::string DateTime::GetYearString() const
{
	return xr_string::ToString(GetYear());
}