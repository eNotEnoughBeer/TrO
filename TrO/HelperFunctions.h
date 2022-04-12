#pragma once
#include <atlstr.h>
#include <limits>
#include <ctime>
#include <process.h>
#include "md5.h"
#include "des.h"
#include "sha1.h"

#ifdef max
#undef max
#endif

static std::string get_utf8(const std::wstring& wstr)
{
	if (wstr.empty()) return std::string();
	int sz = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], -1, 0, 0, 0, 0);
	std::string res(sz, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], -1, &res[0], sz, 0, 0);
	return res;
}

static std::wstring get_utf16(const std::string& str)
{
	if (str.empty()) return std::wstring();
	int sz = MultiByteToWideChar(CP_UTF8, 0, &str[0], -1, 0, 0);
	std::wstring res(sz, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], -1, &res[0], sz);
	return res;
}

static CString getCurPath()
{ // на возврат идет путь к текущей arx-ине без последнего слеша. например, c:\program files\technolog
	TCHAR fName[_MAX_PATH + 1];
	GetModuleFileName(nullptr, fName, sizeof(fName) - 1);
	int i = (int)_tcslen(fName);
	while ((i > 0) && (fName[i] != _T('\\'))) i--;
	if (i)	fName[i] = _T('\0'); // убираем последний слеш.
	else	_tcscpy(fName, _T("C:"));
	return fName;
}

// функция для получения MD5 из юникодовой строки
static CString getMD5(CString strIn)
{	
	size_t outputSize = strIn.GetLength() + 1;
	char* outputString = new char[outputSize];
	size_t charsConverted = 0;
	const TCHAR* inputW = strIn.GetString();
	wcstombs_s(&charsConverted, outputString, outputSize, inputW, strIn.GetLength());
	std::string a = md5(outputString, strIn.GetLength());
	delete[] outputString;

	USES_CONVERSION;
	return CA2W(a.c_str());
}

// вспомогательная функция
static void _getKeys(CString sKey, ulong64* key, CString siv, ulong64* iv)
{
	swscanf(sKey, _T("%08X%08X"), &key->h, &key->l);
	swscanf(siv, _T("%08X%08X"), &iv->h, &iv->l);
}

// шифруем строку "str" по ключу "key"
static void encodeStr(CString cryptokey, CString& strInOut)
{
	if (cryptokey.IsEmpty() || strInOut.IsEmpty())
		return;
	_tsetlocale(LC_ALL, _T("ru_UA.UTF-8"));
	std::string res = get_utf8(strInOut.GetString());
	//return;
	unsigned char sha1[20];
	std::string keymb = get_utf8(cryptokey.GetString());
	sha1encode_str((char*)keymb.c_str(), sha1);
	//sha1encode_wcs((LPTSTR)(LPCTSTR)cryptokey, sha1);
	CString key_str; key_str.Format(_T("%02X%02X%02X%02X%02X%02X%02X%02X"), sha1[0], sha1[1], sha1[2], sha1[3], sha1[4], sha1[5], sha1[6], sha1[7]);
	CString iv_str; iv_str.Format(_T("%02X%02X%02X%02X%02X%02X%02X%02X"), sha1[19], sha1[18], sha1[17], sha1[16], sha1[15], sha1[14], sha1[13], sha1[12]);
	ulong64 key, iv;
	_getKeys(key_str, &key, iv_str, &iv);

	int	i, srclen, dstlen;
	uchar* src, * dst;
	WCHAR		szTemp[3];

	//srclen = strInOut.GetLength() + 1;
	srclen = (int)res.size() + 1;
	src = new uchar[srclen];
	strcpy((char*)src, res.c_str());
	//wcstombs((char*)src, strInOut, srclen);
	dst = (uchar*)LocalAlloc(LPTR, i = ((srclen >> 3) + 1) << 3);
	dstlen = des_encrypt_ofb(src, srclen, dst, key, iv);

	strInOut.Empty();
	for (i = 0; i < dstlen; i++)
	{
		swprintf_s(szTemp, _T("%02X"), dst[i]);
		strInOut += szTemp;
	}

	LocalFree(dst);
	delete[] src;
	_tsetlocale(LC_ALL, _T("C"));
}

// расшифровываем строку "str" по ключу "key"
static void decodeStr(CString cryptokey, CString& strInOut)
{
	//return;
	if (cryptokey.IsEmpty() || strInOut.IsEmpty())
		return;
	_tsetlocale(LC_ALL, _T("ru_UA.UTF-8"));
	unsigned char sha1[20];
	std::string keymb = get_utf8(cryptokey.GetString());
	sha1encode_str((char*)keymb.c_str(), sha1);
	//sha1encode_wcs((LPTSTR)(LPCTSTR)cryptokey, sha1);
	CString key_str; key_str.Format(_T("%02X%02X%02X%02X%02X%02X%02X%02X"), sha1[0], sha1[1], sha1[2], sha1[3], sha1[4], sha1[5], sha1[6], sha1[7]);
	CString iv_str; iv_str.Format(_T("%02X%02X%02X%02X%02X%02X%02X%02X"), sha1[19], sha1[18], sha1[17], sha1[16], sha1[15], sha1[14], sha1[13], sha1[12]);
	ulong64 key, iv;
	_getKeys(key_str, &key, iv_str, &iv);

	int			i, srclen, dstlen;
	ulong		ul;
	uchar* src, * dst;
	LPCTSTR		t;
	dstlen = strInOut.GetLength() >> 1;
	src = (uchar*)LocalAlloc(LPTR, dstlen);
	dst = (uchar*)LocalAlloc(LPTR, dstlen);
	t = strInOut;

	for (i = 0; i < dstlen; i++)
	{
		swscanf(t, _T("%02X"), &ul);
		dst[i] = (uchar)ul;
		t += 2;
	}

	srclen = des_decrypt_ofb(dst, dstlen, src, key, iv);
	std::string tmp = (char*)src;
	std::wstring res = get_utf16(tmp);
	strInOut = res.c_str();
	/*WCHAR* sz = new WCHAR[srclen << 1];

	mbstowcs(sz, (char*)src, srclen);
	strInOut.Format(_T("%s"), sz);
	delete[] sz;*/

	LocalFree(dst);
	LocalFree(src);
	_tsetlocale(LC_ALL, _T("C"));
}

static std::wstring Int2Str(int data)
{
	TCHAR res[30];
	if (data == std::numeric_limits<int>::max())
	{
		_tcscpy(res, _T(""));
		return res;
	}
	_itot(data, res, 10);
	// можно какие-то проверки значения "res"
	return res;
}

// превращает строку (data) в целое число
static int Str2Int(const TCHAR* data)
{
	if (_tcscmp(data, _T("")) == 0)
		return std::numeric_limits<int>::max();
	int res = _ttoi(data);
	// можно какие-то проверки значения "res"
	return res;
}

// превращает double (data) в строку (второй параметр кол-во знаков после запятой, третий, обрезка нулей с хвоста. 3.45000 -> 3.45)
static std::wstring Dbl2Str(double data, int prec = 3, bool autoclean = false)
{
	TCHAR res[120], buffer[10];
	try {
		if (data == std::numeric_limits<double>::max())
		{
			_tcscpy(res, _T(""));
			return res;
		}

		if (prec < 0) prec = 0;

		_tcscpy(buffer, _T("%."));
		_tcscat(buffer, Int2Str(prec).c_str());
		_tcscat(buffer, _T("f"));
		_stprintf(res, buffer, data);
		//-> чтоб нам не мешала локализованная переменная разделителя целой и дробной части, всегда ставим точку
		std::wstring numberAsString;
		numberAsString.clear();
		numberAsString.append(res);
		TCHAR buf[5];
		GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, buf, 5);

		for (int i = 0; i < (int)numberAsString.size(); i++)
		{
			if (numberAsString[i] == buf[0])
				numberAsString[i] = _T('.');
		}
		_tcscpy(res, numberAsString.c_str());
		//<-

			// можно какие-то проверки значения "res"
		if (autoclean && prec != 0)//added by Alex 
		{
			int i = (int)_tcslen(res) - 1;
			while ((i > 0) && (res[i] == _T('0'))) i--;
			if (i)
			{
				res[i + 1] = _T('\0'); // на месте последнего нуля ставим '\0'.
				if (res[i] == _T('.') || res[i] == _T(',')) res[i] = _T('\0'); // если после обрезки мы получили последним символом точку, то и ее нада угробить
			}
		}
	}
	catch (...)
	{
		return _T("");
	}
	return res;
}

// превращает строку (data) в double
static double Str2Dbl(const TCHAR* data)
{
	double res;
	try {
		if (_tcscmp(data, _T("")) == 0)
			return std::numeric_limits<double>::max();
		//-> чтоб нам не мешала локализованная переменная разделителя целой и дробной части, всегда ставим точку
		std::wstring numberAsString;
		numberAsString.clear();
		numberAsString.append(data);
		TCHAR buf[5];
		GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, buf, 5);

		for (int i = 0; i < (int)numberAsString.size(); i++)
		{
			if (numberAsString[i] == buf[0])
				numberAsString[i] = _T('.');
		}
		//<-
		res = _tstof(numberAsString.c_str());
	}
	catch (...)
	{
		return std::numeric_limits<double>::max();
	}
	// можно какие-то проверки значения "res"
	return res;
}

static CString gen_random(const int len) {
	srand((unsigned)time(NULL) * _getpid());
	static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	std::wstring tmp_s;
	tmp_s.reserve(len);

	for (int i = 0; i < len; ++i) {
		tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	return tmp_s.c_str();
}