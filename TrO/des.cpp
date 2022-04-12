// ============================================================
// DES - Data Encryption Standart
//
// tiger (13angel@inbox.lv)
//
#include "pch.h"
#include "des.h"

#include "globals.h"

ulong64 pc1(ulong64 v);
ulong64 pc2(ulong64 v);
ulong64 ip(ulong64 v);
ulong64 rip(ulong64 v);

ulong64 ep(ulong v);
ulong	s_matrix(ulong64 v);
ulong	p(ulong v);

ulong64 fk(ulong64 v, ulong64 key);

// ------------------------------------------------------------
// calculate 16 DES subkeys
//
void des_create_keys(ulong64 src, ulong64 *key)
{
	uchar		ss[] = {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1};
	uchar		s;
	ulong64		t;
	int			i;

	TRACE(L"key - %08X%08X\n", src.h, src.l);

	t = pc1(src);

//	TRACE(L"pc1 - %08X%08X\n", t.h, t.l);

	for (i = 0; i < 16; i++)
	{
		s = ss[i];

		t.l = (0x0FFFFFFF & (t.l << s)) | (t.l >> (28 - s));
		t.h = (0x0FFFFFFF & (t.h << s)) | (t.h >> (28 - s));

//		TRACE(L"rol - %08X%08X\n", t.h, t.l);

		key[i] = pc2(t);

//		TRACE(L"pc2 - %08X%08X\n", key[i].h, key[i].l);
	}
}



// ------------------------------------------------------------
// DES encrypt block
//
ulong64 des_encrypt_block(ulong64 src, ulong64 *keys)
{
	ulong64	r = {0, 0};
	ulong	t;

//	TRACE(L"crypt - %08X%08X\n", src.h, src.l);

	// initial permutation
	r = ip(src);

//	TRACE(L"IP - %08X%08X\n", r.h, r.l);

	// 16 crypting rounds
	r = fk(r, keys[0]);
	r = fk(r, keys[1]);
	r = fk(r, keys[2]);
	r = fk(r, keys[3]);
	r = fk(r, keys[4]);
	r = fk(r, keys[5]);
	r = fk(r, keys[6]);
	r = fk(r, keys[7]);
	r = fk(r, keys[8]);
	r = fk(r, keys[9]);
	r = fk(r, keys[10]);
	r = fk(r, keys[11]);
	r = fk(r, keys[12]);
	r = fk(r, keys[13]);
	r = fk(r, keys[14]);
	r = fk(r, keys[15]);

	// swap
	t	= r.h;
	r.h	= r.l;
	r.l = t;

	// reverse to initial permutation
	r = rip(r);

//	TRACE(L"RIP - %08X%08X\n", r.h, r.l);

	TRACE(L"encrypted %08X%08X -> %08X%08X\n", src.h, src.l, r.h, r.l);

	return r;
}



// ------------------------------------------------------------
// DES decrypt block
//
ulong64 des_decrypt_block(ulong64 src, ulong64 *keys)
{
	ulong64	r = {0, 0};
	ulong	t;

//	TRACE(L"crypt - %08X%08X\n", src.h, src.l);

	// initial permutation
	r = ip(src);

//	TRACE(L"IP - %08X%08X\n", r.h, r.l);

	// 16 crypting rounds
	r = fk(r, keys[15]);
	r = fk(r, keys[14]);
	r = fk(r, keys[13]);
	r = fk(r, keys[12]);
	r = fk(r, keys[11]);
	r = fk(r, keys[10]);
	r = fk(r, keys[9]);
	r = fk(r, keys[8]);
	r = fk(r, keys[7]);
	r = fk(r, keys[6]);
	r = fk(r, keys[5]);
	r = fk(r, keys[4]);
	r = fk(r, keys[3]);
	r = fk(r, keys[2]);
	r = fk(r, keys[1]);
	r = fk(r, keys[0]);

	// swap
	t	= r.h;
	r.h	= r.l;
	r.l = t;

	// reverse to initial permutation
	r = rip(r);

//	TRACE(L"RIP - %08X%08X\n", r.h, r.l);

	TRACE(L"decrypted %08X%08X -> %08X%08X\n", src.h, src.l, r.h, r.l);

	return r;
}



// ------------------------------------------------------------
// main transformation function
//
ulong64 fk(ulong64 v, ulong64 key)
{
	ulong64	r = {0, 0};

	// extending permutation
	r = ep(v.l);

//	TRACE(L"EP - %08X%08X\n", r.h, r.l);

	// XOR with a key
	r.h ^= key.h;
	r.l ^= key.l;

//	TRACE(L"XOR - %08X%08X\n", r.h, r.l);

	// S matrix
	r.l = s_matrix(r);

//	TRACE(L"S Matrix - %08X\n", r.l);

	// just a permutation
	r.l = p(r.l);

//	TRACE(L"P - %08X\n", r.l);

	// XOR with source right
	r.l ^= v.h;

	// put source left to the right
	r.h = v.l;

	return r;
}



// ------------------------------------------------------------
// DES encrypt - ECB mode (Electronic Codebook)
//
ulong des_encrypt_ecb(uchar *src, ulong srclen, uchar *dst, ulong64 key)
{
	uchar		buff[8];
	ulong64		keys[16];
	ulong64		r;
	uchar		*t = dst;

	if (srclen == 0)
		return 0;

	// prepare keys
	des_create_keys(key, keys);

	if (srclen >= 8)
	{
		do
		{
			// get 64 bits
			decode_rev(&r.h, src, 4);
			src += 4;
			decode_rev(&r.l, src, 4);
			src += 4;

			// encrypt
			r = des_encrypt_block(r, keys);

			// put 64 bits
			encode_rev(dst, &r.h, 4);
			dst += 4;
			encode_rev(dst, &r.l, 4);
			dst += 4;

			// 
			srclen -= 8;
		}
		while (srclen >= 8);
	}

	// crypt the rest
	if (srclen != 0)
	{
		memset(buff, 0, sizeof(buff));
		memcpy(buff, src, srclen);

		// get 64 bits
		decode_rev(&r.h, buff, 4);
		decode_rev(&r.l, buff + 4, 4);

		// encrypt
		r = des_encrypt_block(r, keys);

		// put 64 bits
		encode_rev(dst, &r.h, 4);
		dst += 4;
		encode_rev(dst, &r.l, 4);
		dst += 4;
	}

	return (dst - t);
}



// ------------------------------------------------------------
// DES decrypt - ECB mode (Electronic Codebook)
//
ulong des_decrypt_ecb(uchar *src, ulong srclen, uchar *dst, ulong64 key)
{
	ulong64		keys[16];
	ulong64		r;
	uchar		*t = dst;

	if (srclen < 8)
		return 0;

	// prepare keys
	des_create_keys(key, keys);

	do
	{
		// get 64 bits
		decode_rev(&r.h, src, 4);
		src += 4;
		decode_rev(&r.l, src, 4);
		src += 4;

		// encrypt
		r = des_decrypt_block(r, keys);

		// put 64 bits
		encode_rev(dst, &r.h, 4);
		dst += 4;
		encode_rev(dst, &r.l, 4);
		dst += 4;

		// 
		srclen -= 8;
	}
	while (srclen >= 8);

	return (dst - t);
}



// ------------------------------------------------------------
// DES encrypt - CBC mode (Cipher Block Chaining)
//
ulong des_encrypt_cbc(uchar *src, ulong srclen, uchar *dst, ulong64 key, ulong64 iv)
{
	uchar		buff[8];
	ulong64		keys[16];
	ulong64		r;
	uchar		*t = dst;

	if (srclen == 0)
		return 0;

	// prepare keys
	des_create_keys(key, keys);

	if (srclen >= 8)
	{
		do
		{
			// get 64 bits
			decode_rev(&r.h, src, 4);
			src += 4;
			decode_rev(&r.l, src, 4);
			src += 4;

			// XOR with IV
			r.h ^= iv.h;
			r.l ^= iv.l;

			// encrypt
			r = des_encrypt_block(r, keys);

			iv = r;

			// put 64 bits
			encode_rev(dst, &r.h, 4);
			dst += 4;
			encode_rev(dst, &r.l, 4);
			dst += 4;

			// 
			srclen -= 8;
		}
		while (srclen >= 8);
	}

	// crypt the rest
	if (srclen != 0)
	{
		memset(buff, 0, sizeof(buff));
		memcpy(buff, src, srclen);

		// get 64 bits
		decode_rev(&r.h, buff, 4);
		decode_rev(&r.l, buff + 4, 4);

		// XOR with IV
		r.h ^= iv.h;
		r.l ^= iv.l;

		// encrypt
		r = des_encrypt_block(r, keys);

		// put 64 bits
		encode_rev(dst, &r.h, 4);
		dst += 4;
		encode_rev(dst, &r.l, 4);
		dst += 4;
	}

	return (dst - t);
}



// ------------------------------------------------------------
// DES decrypt - CBC mode (Cipher Block Chaining)
//
ulong des_decrypt_cbc(uchar *src, ulong srclen, uchar *dst, ulong64 key, ulong64 iv)
{
	ulong64		keys[16];
	ulong64		r, c;
	uchar		*t = dst;

	if (srclen < 8)
		return 0;

	// prepare keys
	des_create_keys(key, keys);

	do
	{
		// get 64 bits
		decode_rev(&c.h, src, 4);
		src += 4;
		decode_rev(&c.l, src, 4);
		src += 4;

		// encrypt
		r = des_decrypt_block(c, keys);

		// XOR with IV
		r.h ^= iv.h;
		r.l ^= iv.l;

		iv = c;

		// put 64 bits
		encode_rev(dst, &r.h, 4);
		dst += 4;
		encode_rev(dst, &r.l, 4);
		dst += 4;

		// 
		srclen -= 8;
	}
	while (srclen >= 8);

	return (dst - t);
}



// ------------------------------------------------------------
// DES encrypt - CFB mode (Cipher Feedback)
//
ulong des_encrypt_cfb(uchar *src, ulong srclen, uchar *dst, ulong64 key, ulong64 iv)
{
	ulong64		keys[16];
	ulong64		r;
	uchar		*t = dst, c;

	if (srclen == 0)
		return 0;

	// prepare keys
	des_create_keys(key, keys);

	r = iv;

	do
	{
		// encrypt vector
		r = des_encrypt_block(r, keys);

		// crypted char
		c = (uchar) (r.h >> 24) ^ *src++;

		// put it to the output
		*dst++ = c;

		// modify vector
		r.h <<= 8;
		r.h |= (r.l >> 24);
		r.l <<= 8;
		r.l |= c;
	}
	while (--srclen != 0);

	return (dst - t);
}



// ------------------------------------------------------------
// DES decrypt - CFB mode (Cipher Feedback)
//
ulong des_decrypt_cfb(uchar *src, ulong srclen, uchar *dst, ulong64 key, ulong64 iv)
{
	ulong64		keys[16];
	ulong64		r;
	uchar		*t = dst, c;

	if (srclen == 8)
		return 0;

	// prepare keys
	des_create_keys(key, keys);

	r = iv;

	do
	{
		// encrypt vector
		r = des_encrypt_block(r, keys);

		// crypted char
		c = *src++;

		// decrypt it and put to the output
		*dst++ = (uchar) (r.h >> 24) ^ c;

		// modify vector
		r.h <<= 8;
		r.h |= (r.l >> 24);
		r.l <<= 8;
		r.l |= c;
	}
	while (--srclen != 0);

	return (dst - t);
}



// ------------------------------------------------------------
// DES encrypt - OFB mode (Output Feedback)
//
ulong des_encrypt_ofb(uchar *src, ulong srclen, uchar *dst, ulong64 key, ulong64 iv)
{
	ulong64		keys[16];
	ulong64		r;
	uchar		*t = dst, h;

	if (srclen == 0)
		return 0;

	// prepare keys
	des_create_keys(key, keys);

	r = iv;

	do
	{
		// encrypt vector
		r = des_encrypt_block(r, keys);

		// get the right most 8 bits
		h = (uchar) (r.h >> 24);

		// crypted a char and put it to the output
		*dst++ = h ^ *src++;

		// modify vector
		r.h <<= 8;
		r.h |= (r.l >> 24);
		r.l <<= 8;
		r.l |= h;
	}
	while (--srclen != 0);

	return (dst - t);
}



// ------------------------------------------------------------
// DES encrypt - OFB mode (Output Feedback)
//
ulong des_decrypt_ofb(uchar *src, ulong srclen, uchar *dst, ulong64 key, ulong64 iv)
{
	ulong64		keys[16];
	ulong64		r;
	uchar		*t = dst, h;

	if (srclen == 8)
		return 0;

	// prepare keys
	des_create_keys(key, keys);

	r = iv;

	do
	{
		// encrypt vector
		r = des_encrypt_block(r, keys);

		// get the right most 8 bits
		h = (uchar) (r.h >> 24);

		// decrypt it and put to the output
		*dst++ = h ^ *src++;;

		// modify vector
		r.h <<= 8;
		r.h |= (r.l >> 24);
		r.l <<= 8;
		r.l |= h;
	}
	while (--srclen != 0);

	return (dst - t);
}






// ============================================================
// permutations and helper functions
// ============================================================

// ------------------------------------------------------------
// permuted choice 1 (choice from a permutation)
//
ulong64 pc1(ulong64 v)
{
	uchar	p1[] = {
					60, 52, 44, 36, 59, 51, 43,
					35, 27, 19, 11,  3, 58, 50,
					42, 34, 26, 18, 10,  2, 57,
					49, 41, 33, 25, 17,  9,  1};

	uchar	p2[] = {
					28, 20, 12,  4, 61, 53, 45,
					37, 29, 21, 13,  5, 62, 54,
					46, 38, 30, 22, 14,  6, 63,
					55, 47, 39, 31, 23, 15,  7};
	ulong64	r = {0, 0};
	ulong	t;
	uchar	s;
	int		i;

	// 28
	for (i = 27; i >= 0; i--)
	{
		t = v.l;
		s = p1[i];

		if (s > 31)
		{
			t = v.h;
			s -= 32;
		}
		r.l |= ((t >> s) & 1) << i;
	}
	// 28
	for (i = 27; i >= 0; i--)
	{
		t = v.l;
		s = p2[i];

		if (s > 31)
		{
			t = v.h;
			s -= 32;
		}
		r.h |= ((t >> s) & 1) << i;
	}
	return r;
}



// ------------------------------------------------------------
// permuted choice 1 (choice from a permutation)
//
ulong64 pc2(ulong64 v)
{
	uchar	p1[] = {
					24, 27, 20,  6, 14, 10,
					 3, 22,  0, 17,  7, 12,
					 8, 23, 11,  5, 16, 26,
					 1,  9, 19, 25,  4, 15,
					54, 43, 36, 29, 49, 40,
					48, 30};
	uchar	p2[] = {
					52, 44, 37, 33,
					46, 35, 50, 41, 28, 53,
					51, 55, 32, 45, 39, 42};

	ulong64	r = {0, 0};
	ulong	t;
	uchar	s;
	int		i;

	// 32
	for (i = 31; i >= 0; i--)
	{
		t = v.l;
		s = p1[i];

		if (s > 27)
		{
			t = v.h;
			s -= 28;
		}
		r.l |= ((t >> s) & 1) << i;
	}
	// 16
	for (i = 15; i >= 0; i--)
	{
		t = v.l;
		s = p2[i];

		if (s > 27)
		{
			t = v.h;
			s -= 28;
		}
		r.h |= ((t >> s) & 1) << i;
	}
	return r;
}



// ------------------------------------------------------------
// initial permutation
//
ulong64 ip(ulong64 v)
{
	uchar	p1[] = {
					57, 49, 41, 33, 25, 17,  9,  1,
					59, 51, 43, 35, 27, 19, 11,  3,
					61, 53, 45, 37, 29, 21, 13,  5,
					63, 55, 47, 39, 31, 23, 15,  7};
	uchar	p2[] = {
					56, 48, 40, 32, 24, 16,  8,  0,
					58, 50, 42, 34, 26, 18, 10,  2,
					60, 52, 44, 36, 28, 20, 12,  4,
					62, 54, 46, 38, 30, 22, 14,  6};
	ulong64	r = {0, 0};
	ulong	t;
	uchar	s;
	int		i;

	// 32
	for (i = 31; i >= 0; i--)
	{
		t = v.l;
		s = p1[i];

		if (s > 31)
		{
			t = v.h;
			s -= 32;
		}
		r.l |= ((t >> s) & 1) << i;
	}
	// 32
	for (i = 31; i >= 0; i--)
	{
		t = v.l;
		s = p2[i];

		if (s > 31)
		{
			t = v.h;
			s -= 32;
		}
		r.h |= ((t >> s) & 1) << i;
	}
	return r;
}



// ------------------------------------------------------------
// reverse to initial permutation
//
ulong64 rip(ulong64 v)
{
	uchar	p1[] = {
					39,  7, 47, 15, 55, 23, 63, 31,
					38,  6, 46, 14, 54, 22, 62, 30,
					37,  5, 45, 13, 53, 21, 61, 29,
					36,  4, 44, 12, 52, 20, 60, 28};
	uchar	p2[] = {
					35,  3, 43, 11, 51, 19, 59, 27,
					34,  2, 42, 10, 50, 18, 58, 26,
					33,  1, 41,  9, 49, 17, 57, 25,
					32,  0, 40,  8, 48, 16, 56, 24};
	ulong64	r = {0, 0};
	ulong	t;
	uchar	s;
	int		i;

	// 32
	for (i = 31; i >= 0; i--)
	{
		t = v.l;
		s = p1[i];

		if (s > 31)
		{
			t = v.h;
			s -= 32;
		}
		r.l |= ((t >> s) & 1) << i;
	}
	// 32
	for (i = 31; i >= 0; i--)
	{
		t = v.l;
		s = p2[i];

		if (s > 31)
		{
			t = v.h;
			s -= 32;
		}
		r.h |= ((t >> s) & 1) << i;
	}
	return r;
}



// ------------------------------------------------------------
// extension permutation
//
ulong64 ep(ulong v)
{
	uchar	p1[] = {
					31,  0,  1,  2,  3,  4,
					 3,  4,  5,  6,  7,  8,
					 7,  8,  9, 10, 11, 12,
					11, 12, 13, 14, 15, 16,
					15, 16, 17, 18, 19, 20,
					19, 20};
	uchar	p2[] = {
					21, 22, 23, 24,
					23, 24, 25, 26, 27, 28,
					27, 28, 29, 30, 31,  0};
	ulong64	r = {0, 0};
	int		i;

	// 32
	for (i = 31; i >= 0; i--)
	{
		r.l |= ((v >> p1[i]) & 1) << i;
	}
	// 16
	for (i = 15; i >= 0; i--)
	{
		r.h |= ((v >> p2[i]) & 1) << i;
	}
	return r;
}



// ------------------------------------------------------------
// extension permutation
//
ulong s_matrix(ulong64 v)
{
	static ulong	s1[] = {
				14 << 28,  4 << 28, 13 << 28,  1 << 28,  2 << 28, 15 << 28, 11 << 28,  8 << 28,  3 << 28, 10 << 28,  6 << 28, 12 << 28,  5 << 28,  9 << 28,  0 << 28,  7 << 28,
				 0 << 28, 15 << 28,  7 << 28,  4 << 28, 14 << 28,  2 << 28, 13 << 28,  1 << 28, 10 << 28,  6 << 28, 12 << 28, 11 << 28,  9 << 28,  5 << 28,  3 << 28,  8 << 28,
				 4 << 28,  1 << 28, 14 << 28,  8 << 28, 13 << 28,  6 << 28,  2 << 28, 11 << 28, 15 << 28, 12 << 28,  9 << 28,  7 << 28,  3 << 28, 10 << 28,  5 << 28,  0 << 28,
				15 << 28, 12 << 28,  8 << 28,  2 << 28,  4 << 28,  9 << 28,  1 << 28,  7 << 28,  5 << 28, 11 << 28,  3 << 28, 14 << 28, 10 << 28,  0 << 28,  6 << 28, 13 << 28
			};
	static ulong	s2[] = {
				15 << 24,  1 << 24,  8 << 24, 14 << 24,  6 << 24, 11 << 24,  3 << 24,  4 << 24,  9 << 24,  7 << 24,  2 << 24, 13 << 24, 12 << 24,  0 << 24,  5 << 24, 10 << 24,
				 3 << 24, 13 << 24,  4 << 24,  7 << 24, 15 << 24,  2 << 24,  8 << 24, 14 << 24, 12 << 24,  0 << 24,  1 << 24, 10 << 24,  6 << 24,  9 << 24, 11 << 24,  5 << 24,
				 0 << 24, 14 << 24,  7 << 24, 11 << 24, 10 << 24,  4 << 24, 13 << 24,  1 << 24,  5 << 24,  8 << 24, 12 << 24,  6 << 24,  9 << 24,  3 << 24,  2 << 24, 15 << 24,
				13 << 24,  8 << 24, 10 << 24,  1 << 24,  3 << 24, 15 << 24,  4 << 24,  2 << 24, 11 << 24,  6 << 24,  7 << 24, 12 << 24,  0 << 24,  5 << 24, 14 << 24,  9 << 24
			};
	static ulong	s3[] = {
				10 << 20,  0 << 20,  9 << 20, 14 << 20,  6 << 20,  3 << 20, 15 << 20,  5 << 20,  1 << 20, 13 << 20, 12 << 20,  7 << 20, 11 << 20,  4 << 20,  2 << 20,  8 << 20,
				13 << 20,  7 << 20,  0 << 20,  9 << 20,  3 << 20,  4 << 20,  6 << 20, 10 << 20,  2 << 20,  8 << 20,  5 << 20, 14 << 20, 12 << 20, 11 << 20, 15 << 20,  1 << 20,
				13 << 20,  6 << 20,  4 << 20,  9 << 20,  8 << 20, 15 << 20,  3 << 20,  0 << 20, 11 << 20,  1 << 20,  2 << 20, 12 << 20,  5 << 20, 10 << 20, 14 << 20,  7 << 20,
				 1 << 20, 10 << 20, 13 << 20,  0 << 20,  6 << 20,  9 << 20,  8 << 20,  7 << 20,  4 << 20, 15 << 20, 14 << 20,  3 << 20, 11 << 20,  5 << 20,  2 << 20, 12 << 20
			};
	static ulong	s4[] = {
				 7 << 16, 13 << 16, 14 << 16,  3 << 16,  0 << 16,  6 << 16,  9 << 16, 10 << 16,  1 << 16,  2 << 16,  8 << 16,  5 << 16, 11 << 16, 12 << 16,  4 << 16, 15 << 16,
				13 << 16,  8 << 16, 11 << 16,  5 << 16,  6 << 16, 15 << 16,  0 << 16,  3 << 16,  4 << 16,  7 << 16,  2 << 16, 12 << 16,  1 << 16, 10 << 16, 14 << 16,  9 << 16,
				10 << 16,  6 << 16,  9 << 16,  0 << 16, 12 << 16, 11 << 16,  7 << 16, 13 << 16, 15 << 16,  1 << 16,  3 << 16, 14 << 16,  5 << 16,  2 << 16,  8 << 16,  4 << 16,
				 3 << 16, 15 << 16,  0 << 16,  6 << 16, 10 << 16,  1 << 16, 13 << 16,  8 << 16,  9 << 16,  4 << 16,  5 << 16, 11 << 16, 12 << 16,  7 << 16,  2 << 16, 14 << 16
			};
	static ulong	s5[] = {
				 2 << 12, 12 << 12,  4 << 12,  1 << 12,  7 << 12, 10 << 12, 11 << 12,  6 << 12,  8 << 12,  5 << 12,  3 << 12, 15 << 12, 13 << 12,  0 << 12, 14 << 12,  9 << 12,
				14 << 12, 11 << 12,  2 << 12, 12 << 12,  4 << 12,  7 << 12, 13 << 12,  1 << 12,  5 << 12,  0 << 12, 15 << 12, 10 << 12,  3 << 12,  9 << 12,  8 << 12,  6 << 12,
				 4 << 12,  2 << 12,  1 << 12, 11 << 12, 10 << 12, 13 << 12,  7 << 12,  8 << 12, 15 << 12,  9 << 12, 12 << 12,  5 << 12,  6 << 12,  3 << 12,  0 << 12, 14 << 12,
				11 << 12,  8 << 12, 12 << 12,  7 << 12,  1 << 12, 14 << 12,  2 << 12, 13 << 12,  6 << 12, 15 << 12,  0 << 12,  9 << 12, 10 << 12,  4 << 12,  5 << 12,  3 << 12
			};
	static ulong	s6[] = {
				12 << 8,  1 << 8, 10 << 8, 15 << 8,  9 << 8,  2 << 8,  6 << 8,  8 << 8,  0 << 8, 13 << 8,  3 << 8,  4 << 8, 14 << 8,  7 << 8,  5 << 8, 11 << 8,
				10 << 8, 15 << 8,  4 << 8,  2 << 8,  7 << 8, 12 << 8,  9 << 8,  5 << 8,  6 << 8,  1 << 8, 13 << 8, 14 << 8,  0 << 8, 11 << 8,  3 << 8,  8 << 8,
				 9 << 8, 14 << 8, 15 << 8,  5 << 8,  2 << 8,  8 << 8, 12 << 8,  3 << 8,  7 << 8,  0 << 8,  4 << 8, 10 << 8,  1 << 8, 13 << 8, 11 << 8,  6 << 8,
				 4 << 8,  3 << 8,  2 << 8, 12 << 8,  9 << 8,  5 << 8, 15 << 8, 10 << 8, 11 << 8, 14 << 8,  1 << 8,  7 << 8,  6 << 8,  0 << 8,  8 << 8, 13 << 8
			};
	static ulong	s7[] = {
				 4 << 4, 11 << 4,  2 << 4, 14 << 4, 15 << 4,  0 << 4,  8 << 4, 13 << 4,  3 << 4, 12 << 4,  9 << 4,  7 << 4,  5 << 4, 10 << 4,  6 << 4,  1 << 4,
				13 << 4,  0 << 4, 11 << 4,  7 << 4,  4 << 4,  9 << 4,  1 << 4, 10 << 4, 14 << 4,  3 << 4,  5 << 4, 12 << 4,  2 << 4, 15 << 4,  8 << 4,  6 << 4,
				 1 << 4,  4 << 4, 11 << 4, 13 << 4, 12 << 4,  3 << 4,  7 << 4, 14 << 4, 10 << 4, 15 << 4,  6 << 4,  8 << 4,  0 << 4,  5 << 4,  9 << 4,  2 << 4,
				 6 << 4, 11 << 4, 13 << 4,  8 << 4,  1 << 4,  4 << 4, 10 << 4,  7 << 4,  9 << 4,  5 << 4,  0 << 4, 15 << 4, 14 << 4,  2 << 4,  3 << 4, 12 << 4
			};
	static ulong	s8[] = {
				13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
				 1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
				 7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
				 2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
			};

	v.h <<= 2;
	v.h |= (v.l >> 30);

	return
		s1[(((v.h >> 12) & 0x20) | ((v.h >>  8) & 0x10)) + ((v.h >> 13) & 0xF)] +
		s2[(((v.h >>  6) & 0x20) | ((v.h >>  2) & 0x10)) + ((v.h >>  7) & 0xF)] +
		s3[(((v.h      ) & 0x20) | ((v.h <<  4) & 0x10)) + ((v.h >>  1) & 0xF)] +
		s4[(((v.l >> 24) & 0x20) | ((v.l >> 20) & 0x10)) + ((v.l >> 25) & 0xF)] +
		s5[(((v.l >> 18) & 0x20) | ((v.l >> 14) & 0x10)) + ((v.l >> 19) & 0xF)] +
		s6[(((v.l >> 12) & 0x20) | ((v.l >>  8) & 0x10)) + ((v.l >> 13) & 0xF)] +
		s7[(((v.l >>  6) & 0x20) | ((v.l >>  2) & 0x10)) + ((v.l >>  7) & 0xF)] +
		s8[(((v.l      ) & 0x20) | ((v.l <<  4) & 0x10)) + ((v.l >>  1) & 0xF)];

/*
	uchar	s1[] = {
				14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
				 0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
				 4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
				15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13
			};
	uchar	s2[] = {
				15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
				 3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
				 0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
				13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9
			};
	uchar	s3[] = {
				10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
				13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
				13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
				 1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12
			};
	uchar	s4[] = {
				 7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
				13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
				10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
				 3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14
			};
	uchar	s5[] = {
				 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
				14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
				 4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
				11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3
			};
	uchar	s6[] = {
				12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
				10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
				 9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
				 4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13
			};
	uchar	s7[] = {
				 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
				13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
				 1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
				 6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12
			};
	uchar	s8[] = {
				13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
				 1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
				 7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
				 2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
			};

	v.h <<= 2;
	v.h |= (v.l >> 30);

	return
		(s1[((((v.h >> 16) & 2) | ((v.h >> 12) & 1)) << 4) + ((v.h >> 13) & 0xF)] << 28) +
		(s2[((((v.h >> 10) & 2) | ((v.h >>  6) & 1)) << 4) + ((v.h >>  7) & 0xF)] << 24) +
		(s3[((((v.h >>  4) & 2) | ((v.h      ) & 1)) << 4) + ((v.h >>  1) & 0xF)] << 20) +
		(s4[((((v.l >> 28) & 2) | ((v.l >> 24) & 1)) << 4) + ((v.l >> 25) & 0xF)] << 16) +
		(s5[((((v.l >> 22) & 2) | ((v.l >> 18) & 1)) << 4) + ((v.l >> 19) & 0xF)] << 12) +
		(s6[((((v.l >> 16) & 2) | ((v.l >> 12) & 1)) << 4) + ((v.l >> 13) & 0xF)] <<  8) +
		(s7[((((v.l >> 10) & 2) | ((v.l >>  6) & 1)) << 4) + ((v.l >>  7) & 0xF)] <<  4) +
		(s8[((((v.l >>  4) & 2) | ((v.l      ) & 1)) << 4) + ((v.l >>  1) & 0xF)]     );
*/
}



// ------------------------------------------------------------
// permutation
//
ulong p(ulong v)
{
	uchar	p1[] = {
					 7, 28, 21, 10, 26,  2, 19, 13,
					23, 29,  5,  0, 18,  8, 24, 30,
					22,  1, 14, 27,  6,  9, 17, 31,
					15,  4, 20,  3, 11, 12, 25, 16};
	ulong	r = 0;
	int		i;

	// 32
	for (i = 31; i >= 0; i--)
	{
		r |= ((v >> p1[i]) & 1) << i;
	}
	return r;
}
