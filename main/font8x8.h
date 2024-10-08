/*
 * font8x8.h
 *
 *	Created on: 2017/05/03
 *		Author: yanbe
 */

#ifndef MAIN_FONT8X8_H_
#define MAIN_FONT8X8_H_

/*
   Constant: font8x8.c
   Contains an 90 digree transposed 8x8 font map for unicode points 
   U+0000 - U+007F (basic latin)
   
   This constant has been rotated 90 degrees to make the API easier to use.
   The original version written by darkrose is availble at:
   https://opengameart.org/content/8x8-ascii-bitmap-font-with-c-source
   Note the font.c is licensed GPLv3 or later only.

   Conversion is done via following procedure:
   
int main() {
	for (int code = 0; code < 128; code++) {
		uint64_t f=font[code];
		//printf("0x%lx\n",f);
		uint64_t mask = 0xFF00000000000000;
		uint16_t bits = 56;

		uint8_t basic[8];
		for(int w=0;w<8;w++) {
			//printf("%lx\n",mask);
			uint64_t u0 = f & mask;
			basic[w] = u0 >> bits;
			//printf("%lx %02x\n",u0, basic[w]);
			mask = mask >> 8;
			bits = bits - 8;
		}

		uint8_t trans[8];
		for (int w = 0; w < 8; w++) {
			trans[w] = 0x00;
			for (int b = 0; b < 8; b++) {
				trans[w] |= ((basic[b] & (1 << w)) >> w) << b;
			}
		}
		for (int w = 0; w < 8; w++) {
			if (w == 0) { printf("	  "); }
			printf("0x%.2X", trans[7-w]);
			if (w < 7) { printf(", "); }
			if (w == 7) {
				if (code < 0x20) {
					printf(",	// U+00%.2X\n", code);
				} else {
					printf(",	// U+00%.2X (%c)\n", code, code);
				}
			}
		}
	}
}
*/

static uint8_t font8x8[] = {
	0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+0000 (nul)
	0x00, 0x04, 0x02, 0xFF, 0x02, 0x04, 0x00, 0x00,   // U+0001 (Up Allow)
	0x00, 0x20, 0x40, 0xFF, 0x40, 0x20, 0x00, 0x00,   // U+0002 (Down Allow)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+0003
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+0004
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+0005
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+0006
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+0007
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+0008
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+0009
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+000A
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+000B
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+000C
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+000D
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+000E
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+000F
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+0010
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+0011
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+0012
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+0013
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+0014
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+0015
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+0016
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+0017
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+0018
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+0019
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+001A
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+001B
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+001C
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+001D
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+001E
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+001F
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+0020 (space)
	0x00, 0x00, 0x00, 0x00, 0x2F, 0x00, 0x00, 0x00,   // U+0021 (!)
/*
	00000000
	00000000
	00000000
	00000000
	00101111
	00000000
	00000000
	00000000
*/
	0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00,   // U+0022 (")
	0x00, 0x14, 0x3E, 0x14, 0x3E, 0x14, 0x00, 0x00,   // U+0023 (#)
	0x00, 0x00, 0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00,   // U+0024 ($)
	0x02, 0x05, 0x15, 0x0A, 0x14, 0x2A, 0x28, 0x10,   // U+0025 (%)
	0x00, 0x00, 0x16, 0x29, 0x29, 0x11, 0x10, 0x28,   // U+0026 (&)
	0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,   // U+0027 (')
	0x00, 0x00, 0x0C, 0x12, 0x21, 0x00, 0x00, 0x00,   // U+0028 (()
	0x00, 0x00, 0x00, 0x21, 0x12, 0x0C, 0x00, 0x00,   // U+0029 ())
	0x00, 0x00, 0x15, 0x0E, 0x1F, 0x0E, 0x15, 0x00,   // U+002A (*)
	0x00, 0x00, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00,   // U+002B (+)
	0x00, 0x00, 0x00, 0x40, 0x20, 0x00, 0x00, 0x00,   // U+002C (,)
	0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00,   // U+002D (-)
	0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,   // U+002E (.)
	0x00, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00,   // U+002F (/)
	0x00, 0x0C, 0x12, 0x21, 0x21, 0x12, 0x0C, 0x00,   // U+0030 (0)
	0x00, 0x00, 0x00, 0x22, 0x3F, 0x20, 0x00, 0x00,   // U+0031 (1)
	0x00, 0x22, 0x31, 0x29, 0x29, 0x25, 0x22, 0x00,   // U+0032 (2)
	0x00, 0x12, 0x21, 0x29, 0x29, 0x25, 0x12, 0x00,   // U+0033 (3)
	0x00, 0x18, 0x14, 0x12, 0x3F, 0x10, 0x00, 0x00,   // U+0034 (4)
	0x00, 0x17, 0x25, 0x25, 0x25, 0x25, 0x19, 0x00,   // U+0035 (5)
	0x00, 0x1E, 0x25, 0x25, 0x25, 0x25, 0x18, 0x00,   // U+0036 (6)
	0x00, 0x21, 0x11, 0x09, 0x05, 0x03, 0x01, 0x00,   // U+0037 (7)
	0x00, 0x1A, 0x25, 0x25, 0x25, 0x25, 0x1A, 0x00,   // U+0038 (8)
	0x00, 0x06, 0x29, 0x29, 0x29, 0x29, 0x1E, 0x00,   // U+0039 (9)
	0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,   // U+003A (:)
	0x00, 0x00, 0x00, 0x40, 0x24, 0x00, 0x00, 0x00,   // U+003B (;)
	0x00, 0x08, 0x08, 0x14, 0x14, 0x22, 0x22, 0x00,   // U+003C (<)
	0x00, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x00,   // U+003D (=)
	0x00, 0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x00,   // U+003E (>)
	0x00, 0x02, 0x01, 0x29, 0x09, 0x06, 0x00, 0x00,   // U+003F (?)
	0x18, 0x24, 0x42, 0xBA, 0xAA, 0xBE, 0x00, 0x00,   // U+0040 (@)
	0x00, 0x30, 0x0C, 0x0B, 0x0B, 0x0C, 0x30, 0x00,   // U+0041 (A)
	0x00, 0x3F, 0x25, 0x25, 0x25, 0x1A, 0x00, 0x00,   // U+0042 (B)
	0x0C, 0x12, 0x21, 0x21, 0x21, 0x12, 0x00, 0x00,   // U+0043 (C)
	0x00, 0x3F, 0x21, 0x21, 0x21, 0x1E, 0x00, 0x00,   // U+0044 (D)
	0x00, 0x3F, 0x25, 0x25, 0x25, 0x21, 0x00, 0x00,   // U+0045 (E)
	0x00, 0x3F, 0x05, 0x05, 0x05, 0x01, 0x00, 0x00,   // U+0046 (F)
	0x0C, 0x12, 0x21, 0x29, 0x29, 0x1A, 0x00, 0x00,   // U+0047 (G)
	0x00, 0x3F, 0x04, 0x04, 0x04, 0x04, 0x3F, 0x00,   // U+0048 (H)
	0x00, 0x00, 0x21, 0x21, 0x3F, 0x21, 0x21, 0x00,   // U+0049 (I)
	0x00, 0x10, 0x20, 0x21, 0x21, 0x1F, 0x00, 0x00,   // U+004A (J)
	0x00, 0x3F, 0x08, 0x0C, 0x12, 0x21, 0x00, 0x00,   // U+004B (K)
	0x00, 0x3F, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00,   // U+004C (L)
	0x00, 0x3F, 0x02, 0x04, 0x08, 0x04, 0x02, 0x3F,   // U+004D (M)
	0x00, 0x3F, 0x02, 0x04, 0x08, 0x10, 0x3F, 0x00,   // U+004E (N)
	0x00, 0x00, 0x1E, 0x21, 0x21, 0x21, 0x1E, 0x00,   // U+004F (O)
	0x00, 0x3F, 0x05, 0x05, 0x05, 0x02, 0x00, 0x00,   // U+0050 (P)
	0x00, 0x00, 0x1E, 0x21, 0x21, 0x21, 0x5E, 0x00,   // U+0051 (Q)
	0x00, 0x3F, 0x05, 0x0D, 0x15, 0x22, 0x00, 0x00,   // U+0052 (R)
	0x00, 0x00, 0x12, 0x25, 0x29, 0x29, 0x12, 0x00,   // U+0053 (S)
	0x00, 0x01, 0x01, 0x01, 0x3F, 0x01, 0x01, 0x01,   // U+0054 (T)
	0x00, 0x1F, 0x20, 0x20, 0x20, 0x20, 0x1F, 0x00,   // U+0055 (U)
	0x01, 0x06, 0x18, 0x20, 0x20, 0x18, 0x06, 0x01,   // U+0056 (V)
	0x00, 0x3F, 0x10, 0x08, 0x04, 0x08, 0x10, 0x3F,   // U+0057 (W)
	0x00, 0x21, 0x12, 0x0C, 0x0C, 0x12, 0x21, 0x00,   // U+0058 (X)
	0x00, 0x01, 0x02, 0x04, 0x38, 0x04, 0x02, 0x01,   // U+0059 (Y)
	0x00, 0x21, 0x31, 0x29, 0x25, 0x23, 0x21, 0x00,   // U+005A (Z)
/*
	00000000
	00100001
	00110001
	00101001
	00100101
	00100011
	00100001
	00000000
*/
	0x00, 0x00, 0x3F, 0x21, 0x21, 0x00, 0x00, 0x00,   // U+005B ([)
	0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x00,   // U+005C (\)
	0x00, 0x00, 0x21, 0x21, 0x3F, 0x00, 0x00, 0x00,   // U+005D (])
	0x00, 0x00, 0x02, 0x01, 0x02, 0x00, 0x00, 0x00,   // U+005E (^)
	0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00,   // U+005F (_)
	0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00,   // U+0060 (`)
	0x00, 0x10, 0x2A, 0x2A, 0x2A, 0x1A, 0x3C, 0x00,   // U+0061 (a)
	0x00, 0x3F, 0x14, 0x24, 0x24, 0x24, 0x18, 0x00,   // U+0062 (b)
	0x00, 0x00, 0x18, 0x24, 0x24, 0x24, 0x00, 0x00,   // U+0063 (c)
	0x00, 0x18, 0x24, 0x24, 0x24, 0x14, 0x3F, 0x00,   // U+0064 (d)
	0x00, 0x1C, 0x2A, 0x2A, 0x2A, 0x2A, 0x0C, 0x00,   // U+0065 (e)
	0x00, 0x00, 0x08, 0x3E, 0x0A, 0x00, 0x00, 0x00,   // U+0066 (f)
	0x00, 0x18, 0xA4, 0xA4, 0x88, 0x7C, 0x00, 0x00,   // U+0067 (g)
	0x00, 0x00, 0x3F, 0x04, 0x04, 0x38, 0x00, 0x00,   // U+0068 (h)
	0x00, 0x00, 0x00, 0x00, 0x3D, 0x00, 0x00, 0x00,   // U+0069 (i)
	0x00, 0x80, 0x80, 0x84, 0x7D, 0x00, 0x00, 0x00,   // U+006A (j)
	0x00, 0x00, 0x3F, 0x10, 0x28, 0x24, 0x00, 0x00,   // U+006B (k)
	0x00, 0x00, 0x00, 0x3F, 0x20, 0x00, 0x00, 0x00,   // U+006C (l)
	0x00, 0x3C, 0x04, 0x08, 0x08, 0x04, 0x3C, 0x00,   // U+006D (m)
	0x00, 0x00, 0x3C, 0x08, 0x04, 0x04, 0x3C, 0x00,   // U+006E (n)
	0x00, 0x18, 0x24, 0x24, 0x24, 0x24, 0x18, 0x00,   // U+006F (o)
	/*
	00000000
	00011000
	00111100
	00100100
	00100100
	00100100
	00011000
	00000000
	*/
	0x00, 0xFC, 0x28, 0x24, 0x24, 0x24, 0x18, 0x00,   // U+0070 (p)
	0x00, 0x18, 0x24, 0x24, 0x24, 0x28, 0xFC, 0x00,   // U+0071 (q)
	0x00, 0x00, 0x3C, 0x08, 0x04, 0x04, 0x08, 0x00,   // U+0072 (r)
	0x00, 0x00, 0x24, 0x2A, 0x2A, 0x12, 0x00, 0x00,   // U+0073 (s)
	0x00, 0x00, 0x04, 0x3E, 0x24, 0x04, 0x00, 0x00,   // U+0074 (t)
	0x00, 0x00, 0x1C, 0x20, 0x20, 0x10, 0x3C, 0x00,   // U+0075 (u)
	0x00, 0x0C, 0x10, 0x20, 0x20, 0x10, 0x0C, 0x00,   // U+0076 (v)
	0x0C, 0x30, 0x20, 0x10, 0x10, 0x20, 0x30, 0x0C,   // U+0077 (w)
	0x00, 0x24, 0x28, 0x10, 0x10, 0x28, 0x24, 0x00,   // U+0078 (x)
	0x00, 0x84, 0x88, 0x50, 0x20, 0x10, 0x0C, 0x00,   // U+0079 (y)
	0x00, 0x00, 0x24, 0x34, 0x2C, 0x24, 0x00, 0x00,   // U+007A (z)
	0x00, 0x00, 0x0C, 0x3F, 0x21, 0x21, 0x00, 0x00,   // U+007B ({)
	0x00, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00,   // U+007C (|)
	0x00, 0x00, 0x21, 0x21, 0x3F, 0x0C, 0x00, 0x00,   // U+007D (})
	0x00, 0x10, 0x08, 0x08, 0x10, 0x10, 0x08, 0x00,   // U+007E (~)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // U+007F
	0x00, 0x18, 0x3c, 0x3c, 0x3c, 0x3c, 0x18, 0x00,   // U+0080
};

#endif /* MAIN_FONT8X8_H_ */


