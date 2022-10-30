#include "Utils.h"

std::string
cUtils::Base64Encode(const void* data, size_t num) {
	/* Lookup table */
	static const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	/* Calculate final string size (closest multiple of 4) */
	size_t size = num;
	if (size_t r = size % 3)
		size += 3 - r;
	size = size * 8 / 6;
	/* Create result string filled with padding */
	std::string result(size, '=');
	/* Pointers */
	auto s = result.data();
	auto p = (const uint8_t*)data;
	/* Chars */
	uint8_t a, b, c;
	while (num > 2) {
		a = *p++;
		b = *p++;
		c = *p++;
		num -= 3;
		*s++ = b64chars[a >> 2];
		*s++ = b64chars[0x3F & ((a << 4) | (b >> 4))];
		*s++ = b64chars[0x3F & ((b << 2) | (c >> 6))];
		*s++ = b64chars[0x3F & c];
	}
	if (num) {
		a = *p++;
		*s++ = b64chars[a >> 2];
		if (num == 1)
			*s = b64chars[0x3F & (a << 4)];
		else {
			b = *p;
			*s++ = b64chars[0x3F & ((a << 4) | (b >> 4))];
			*s   = b64chars[0x3F & (b << 2)];
		}
	}
	/* Construct result string */
	return result;
}

std::vector<uint8_t>
cUtils::base64_decode(const char* s, size_t len) {
	/* Lookup table */
	static const int8_t b64values[256] {
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
		-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
		-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
	};
	/* If string is empty, just return */
	if (len == 0)
		return {};
	/* Reserve enough space on result vector */
	std::vector<uint8_t> result;
	result.reserve(len * 6 / 8);
	/* Decode */
	int8_t a, b, c, d;
	for (uint32_t n; len > 4; len -= 4) {
		/* Consume next 4 characters */
		a = b64values[(uint8_t)*s++];
		b = b64values[(uint8_t)*s++];
		c = b64values[(uint8_t)*s++];
		d = b64values[(uint8_t)*s++];
		/* Check for errors */
		if ((a | b | c | d) < 0)
			throw std::exception();
		/* Decode bytes */
		n = (a << 18) | (b << 12) | (c << 6) | d;
		result.emplace_back(        n >> 16);
		result.emplace_back(0xFF & (n >> 8));
		result.emplace_back(0xFF &  n      );
	}
	/* If there's one remaining char, this is invalid */
	if (len == 1)
		throw std::exception();
	/* Load final chars, add padding if missing */
	a = b64values[(uint8_t)*s++];
	b = b64values[(uint8_t)*s++];
	switch (len) {
		case 2:
			c = d = '=';
			break;
		case 3:
			c = *s;
			d = '=';
			break;
		default:
			c = *s++;
			d = *s;
	}
	result.emplace_back((a << 2) | (b >> 4));
	if ((c & d) == '=') {
		if ((a | b) < 0)
			throw std::exception();
		return result;
	}
	c = b64values[(uint8_t)c];
	result.emplace_back(0xFF & ((b << 4) | (c >> 2)));
	if (d == '=') {
		if ((a | b | c) < 0)
			throw std::exception();
		return result;
	}
	d = b64values[(uint8_t)d];
	result.emplace_back(0xFF & ((c << 6) | d));
	if ((a | b | c | d) < 0)
		throw std::exception();
	return result;
}