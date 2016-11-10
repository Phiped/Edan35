// -------------------------------------------------------------------------
// File:    misc.h 
// Desc:    miscellaneous useful stuff
//
// Author:  Tomas Akenine-Möller
// History: March, 2000  (started)
//          August, 2001 (removed "float" stuff)
// -------------------------------------------------------------------------

#ifndef MISC_H
#define MISC_H

#include <assert.h>

// makes the asserts stand out more
#define ASSERT(a) assert(a)  


typedef unsigned char	uint8;
typedef unsigned short	uint16;
typedef unsigned int	uint32;
typedef char			int8;
typedef short			int16;
typedef int				int32;
typedef _int64			int64;
typedef char			sint8;
typedef short			sint16;
typedef int				sint32;


const float fEPSILON=1.0e-5f;

typedef float ZBufferType;

inline int ceilInt(int a, int val) {
	return (a+val-1) - (a+val-1) % val;
}

inline void fSwap(float &a, float &b)
{
   float tmp;
   tmp=a;
   a=b;
   b=tmp;
}

inline void dSwap(double &a, double &b)
{
   double tmp;
   tmp=a;
   a=b;
   b=tmp;
}

inline uint8 uClamp(int value, uint8 low,uint8 high)
{
	if(value<low) return low;
	if(value>high) return high;
	return uint8(value);
}

inline int iClamp(int value, int low,int high)
{
	if(value<low) return low;
	if(value>high) return high;
	return value;
}

inline float fClamp(float value, float low, float high)
{
	if(value<low) return low;
	if(value>high) return high;
	return value;
}

inline bool isPowerOfTwo(uint32 number) { return (number&(number-1))==0;}


typedef unsigned char ubyte;

inline ubyte cropToByte(float color)
{
   if(color<0) return 0;
   else if(color>1.0) return 255;
   else return (ubyte)(color*255);
}

// convert from degrees to radians constant
#ifndef TORAD 
#define TORAD 0.017453293
#endif

#ifndef MAX2
#define MAX2(a,b) ((a)>(b) ? (a) : (b))
#endif

#ifndef MAX3
#define MAX3(a,b,c) MAX2(MAX2(a,b),c)
#endif

#ifndef MIN2
#define MIN2(a,b) ((a)<(b) ? (a) : (b))
#endif

#ifndef MIN3
#define MIN3(a,b,c) MIN2(MIN2(a,b),c)
#endif

#ifndef FABS
#define FABS(a) ((a)<0 ? -(a) : (a))
#endif


#ifndef _MAXFLOAT
#define _MAXFLOAT
#define MAXFLOAT        ((float)3.40282346638528860e+38)
#endif  /* _MAXFLOAT */


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


#ifndef NULL
#define NULL 0
#endif

// find the min and max of three floats
inline void fFindMinMax(float x0,float x1, float x2, float &min, float &max)
{
   min=MIN3(x0,x1,x2);
   max=MAX3(x0,x1,x2);
}

// need to ask Timo, if I can use these. Ulf could use them in Illuminate Labs' code, so this should be fine...
inline int intChop (const float& f) 
{ 
	int32 a			= *reinterpret_cast<const int32*>(&f);			// take bit pattern of float into a register
	int32 sign		= (a>>31);										// sign = 0xFFFFFFFF if original value is negative, 0 if positive
	int32 mantissa	= (a&((1<<23)-1))|(1<<23);						// extract mantissa and add the hidden bit
	int32 exponent	= ((a&0x7fffffff)>>23)-127;						// extract the exponent
	int32 r			= ((uint32)(mantissa)<<8)>>(31-exponent);		// ((1<<exponent)*mantissa)>>24 -- (we know that mantissa > (1<<24))
	return ((r ^ (sign)) - sign ) &~ (exponent>>31);				// add original sign. If exponent was negative, make return value 0.
}

inline int intFloor (const float& f) 
{ 
	int32 a			= *reinterpret_cast<const int32*>(&f);									// take bit pattern of float into a register
	int32 sign		= (a>>31);																// sign = 0xFFFFFFFF if original value is negative, 0 if positive
	a&=0x7fffffff;																			// we don't need the sign any more
	int32 exponent	= (a>>23)-127;															// extract the exponent
	int32 expsign   = ~(exponent>>31);														// 0xFFFFFFFF if exponent is positive, 0 otherwise
	int32 imask		= ( (1<<(31-(exponent))))-1;											// mask for true integer values
	int32 mantissa	= (a&((1<<23)-1));														// extract mantissa (without the hidden bit)
	int32 r			= ((uint32)(mantissa|(1<<23))<<8)>>(31-exponent);						// ((1<<exponent)*(mantissa|hidden bit))>>24 -- (we know that mantissa > (1<<24))
	r = ((r & expsign) ^ (sign)) + ((!((mantissa<<8)&imask)&(expsign^((a-1)>>31)))&sign);	// if (fabs(value)<1.0) value = 0; copy sign; if (value < 0 && value==(int)(value)) value++; 
	return r;
}

inline int intCeil (const float& f) 
{ 
	int32 a			= *reinterpret_cast<const int32*>(&f) ^ 0x80000000;						// take bit pattern of float into a register
	int32 sign		= (a>>31);																// sign = 0xFFFFFFFF if original value is negative, 0 if positive
	a&=0x7fffffff;																			// we don't need the sign any more
	int32 exponent	= (a>>23)-127;															// extract the exponent
	int32 expsign   = ~(exponent>>31);														// 0xFFFFFFFF if exponent is positive, 0 otherwise
	int32 imask		= ( (1<<(31-(exponent))))-1;											// mask for true integer values
	int32 mantissa	= (a&((1<<23)-1));														// extract mantissa (without the hidden bit)
	int32 r			= ((uint32)(mantissa|(1<<23))<<8)>>(31-exponent);						// ((1<<exponent)*(mantissa|hidden bit))>>24 -- (we know that mantissa > (1<<24))
	r = ((r & expsign) ^ (sign)) + ((!((mantissa<<8)&imask)&(expsign^((a-1)>>31)))&sign);	// if (fabs(value)<1.0) value = 0; copy sign; if (value < 0 && value==(int)(value)) value++; 
	return -r;
}

inline int32 floatToFixed(int fractionalBits,float v)
{
	ASSERT(fractionalBits>=0);
	return intFloor((1<<fractionalBits)*v+0.5f);
}

#endif
