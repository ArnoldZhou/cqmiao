#pragma once

#include <cstdlib>
#include <cstring>

inline std::wstring UTF8_To_Unicode(const std::string&str)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	std::unique_ptr<wchar_t[]> pwBuf(new wchar_t[len + 1]{});
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, pwBuf.get(), len);
	return std::wstring(pwBuf.get());
}

inline std::wstring ANSI_To_Unicode(const std::string&str)
{
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
	std::unique_ptr<wchar_t[]> pwBuf(new wchar_t[len + 1]{});
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, pwBuf.get(), len);
	return std::wstring(pwBuf.get());
}

inline std::string UTF8_To_ANSI(const std::string& str)
{
	int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	std::unique_ptr<wchar_t[]> pwBuf(new wchar_t[nwLen + 1]{});    //一定要加1，不然会出现尾巴 
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), pwBuf.get(), nwLen);
	int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf.get(), -1, NULL, NULL, NULL, NULL);
	std::unique_ptr<char[]> pBuf(new char[nLen + 1]{});
	WideCharToMultiByte(CP_ACP, 0, pwBuf.get(), nwLen, pBuf.get(), nLen, NULL, NULL);
	return std::string(pBuf.get());
}

inline std::string GBK_To_UTF8(const std::string& strGbk)//传入的strGbk是GBK编码  
{
	//gbk转unicode  
	auto len = MultiByteToWideChar(CP_ACP, 0, strGbk.c_str(), -1, NULL, 0);
	std::unique_ptr<wchar_t[]> strUnicode(new wchar_t[len + 1] {});
	MultiByteToWideChar(CP_ACP, 0, strGbk.c_str(), -1, strUnicode.get(), len);

	//unicode转UTF-8  
	len = WideCharToMultiByte(CP_UTF8, 0, strUnicode.get(), -1, NULL, 0, NULL, NULL);
	std::unique_ptr<char[]> strUtf8(new char[len + 1] {});
	WideCharToMultiByte(CP_UTF8, 0, strUnicode.get(), -1, strUtf8.get(), len, NULL, NULL);

	//此时的strUtf8是UTF-8编码  
	return std::string(strUtf8.get());
}

// UTF8转GBK
inline std::string UTF8_To_GBK(const std::string& strUtf8)
{
	//UTF-8转unicode  
	int len = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, NULL, 0);
	std::unique_ptr<wchar_t[]> strUnicode(new wchar_t[len + 1] {});//len = 2  
	MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, strUnicode.get(), len);

	//unicode转gbk  
	len = WideCharToMultiByte(CP_ACP, 0, strUnicode.get(), -1, NULL, 0, NULL, NULL);
	std::unique_ptr<char[]>strGbk(new char[len + 1] {});//len=3 本来为2，但是char*后面自动加上了\0  
	WideCharToMultiByte(CP_ACP, 0, strUnicode.get(), -1, strGbk.get(), len, NULL, NULL);

	//此时的strTemp是GBK编码  
	return std::string(strGbk.get());
}