#ifndef _DES_Data_Encryption_Standart_
#define _DES_Data_Encryption_Standart_


typedef unsigned char	uchar;
typedef unsigned short	ushort;
typedef unsigned long	ulong;

typedef struct _ulong64
{
	ulong	l;
	ulong	h;
}
ulong64;


void    des_create_keys(ulong64 src, ulong64 *keys);
ulong64 des_encrypt_block(ulong64 src, ulong64 *keys);
ulong64 des_decrypt_block(ulong64 src, ulong64 *keys);

// encrypting
ulong des_encrypt_ecb(uchar *src, ulong srclen, uchar *dst, ulong64 key);
ulong des_decrypt_ecb(uchar *src, ulong srclen, uchar *dst, ulong64 key);

ulong des_encrypt_cbc(uchar *src, ulong srclen, uchar *dst, ulong64 key, ulong64 iv);
ulong des_decrypt_cbc(uchar *src, ulong srclen, uchar *dst, ulong64 key, ulong64 iv);

ulong des_encrypt_cfb(uchar *src, ulong srclen, uchar *dst, ulong64 key, ulong64 iv);
ulong des_decrypt_cfb(uchar *src, ulong srclen, uchar *dst, ulong64 key, ulong64 iv);

ulong des_encrypt_ofb(uchar *src, ulong srclen, uchar *dst, ulong64 key, ulong64 iv);
ulong des_decrypt_ofb(uchar *src, ulong srclen, uchar *dst, ulong64 key, ulong64 iv);


#endif		// _DES_Data_Encryption_Standart_