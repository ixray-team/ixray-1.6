#pragma once

/*
* 
* Author: wh1t3lord
* Description: fast-stack char/wchar string in one class with std::string API reference implementation
* Date: 16.01.2025
* 
*/

#include <string.h>
#include <iterator>

template<typename char_t, unsigned int _kStringLength>
class stack_string
{
public:

	using iterator_category = std::random_access_iterator_tag;
	using value_type = char_t;
	using pointer = char_t*;
	using reference = char_t&;

public:
	stack_string()
	{
		static_assert(decltype(char) == decltype(char_t) || decltype(wchar_t) == decltype(char_t), "unsupported char format, report to developers (maybe you need it, but at least write your problem)");
		static_assert(_kStringLength != decltype(_kStringLegnth)(-1), "you can't pass a negative value for instatiation!");
		static_assert(_kStringLength > 0, "you can't make a arr as zero lol");

		// if you do {} <= initializes like memset(buf,0,sizeof(buf)) not fast, this is faster initialization!
		// https://godbolt.org/z/9cc833e1W
		m_buffer[0] = char_t(0);
	}

	~stack_string() {}


	inline pointer c_str(void) const { return m_buffer; }
	inline constexpr unsigned int max_size(void) const { return sizeof(m_buffer) / sizeof(char_t); }
	inline unsigned int size(void)
	{
		if constexpr (decltype(char) == decltype(char_t))
		{
			return strlen(m_buffer);
		}
		
		if constexpr (decltype(wchar_t) == decltype(char_t))
		{
			return wcslen(m_buffer);
		}
	}

private:
	char_t m_buffer[_kStringLength];
};

static_assert(sizeof(stack_string<char, 1>) == sizeof(char[1]), "you can't add any additional field to this class! pure buffer on stack... (there's no point in reducing counting operations and caching like size of buffer and etc)");