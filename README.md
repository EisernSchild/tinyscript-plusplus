
# TinyScript++

Tiny script parser and compiler, based on [*TinyExpr*](https://github.com/codeplea/tinyexpr)/[*TinyExpr++*](https://github.com/Blake-Madden/tinyexpr-plusplus). Embed a simple scripting language (in C Code Style) within your C++ project.

## Features

! Note that this project is in alpha stage and NOT (!) fully tested ! Currently bug-fixing !

- **C++17 with no dependencies**.
- Only three files : Single source/header file (TinyScript), source and header file (TinyExpr).
- Simple and fast.
- Implements TinyExpr for expression statements.
- Floating point/Boolean expression statements and If statements
- Released under the zlib license - free for nearly any use.
- Easy to use and integrate with your code.
- Thread-safe; parser is in a self-contained object.

## Building

- add following files to your project :

    * "tinyscript.h"
    * "tinyexpr.h"
    * "tinyexpr.cpp"

- define "TE_FLOAT" if using "float" type instead of "double" for expressions (in "tinyexpr.cpp" as well)
- include "tinyscript.h"
- enlist variables (both floating point and boolean)
- provide your script code string to TinyScript++ to compile
- evaluate the compiled script with the values given to your variables

## Example

[TinyScript++ Test](test/test_tinyscript.cpp)

```cpp
// dont forget to define that in "tinyexpr.cpp" as well if using float
#define TE_FLOAT

#include "../tinyscript.h"
#include <iostream>

#define PI 3.141592654f

void IK_EndEffectorToTargetAngles(
	float fTarX, float fTarY, float fTarZ,     /* <= base to target local vector */
	float fA,                                  /* <= length of bone 1 */
	float fC,                                  /* <= length of bone 0 */
	float& fAlpha,                             /* <= local space rotation Z angle base joint */
	float& fBeta,                              /* <= local space rotation Z angle mid joint */
	float& fGamma)                             /* <= local space rotation Y angle base joint */
{
	// length base->target, length(ad)
	float fB = sqrt(fTarX * fTarX + fTarY * fTarY + fTarZ * fTarZ);
	float fD = sqrt(fTarX * fTarX + fTarZ * fTarZ);

	// triangle angles

	// arccos(b*b + c*c - a*a) / 2bc
	fAlpha = acos((fB * fB + fC * fC - fA * fA) / (2.f * fB * fC));
	// arccos(a*a + c*c - b*b) / 2ac
	fBeta = acos((fA * fA + fC * fC - fB * fB) / (2.f * fA * fC));

	// triangle angles to local angles

	// arctan(cy-dy / length(ad))
	fAlpha = fAlpha + atan(fTarX / fD);
	// PI - beta
	fBeta = abs(PI - fBeta);

	// Y rotation

	// -arctan(z / y)
	fGamma = -atan(fTarZ / fTarX);
	if (fTarX < 0.f)
	{
		fGamma = PI + fGamma;
	}
}

int main()
{
	float fTarX = 0.f, fTarY = 0.f, fTarZ = 0.f, fAlpha = 0.f, fBeta = 0.f, fGamma = 0.f;
	float fA = 2.f, fB = 0.f, fC = 3.f, fD = 0.f;

	// variable list
	std::set<ts_variable> asVars =
	{
		{ "fTarX", &fTarX },
		{ "fTarY", &fTarY },
		{ "fTarZ", &fTarZ },
		{ "fAlpha", &fAlpha },
		{ "fBeta", &fBeta },
		{ "fGamma", &fGamma },
		{ "fA", &fA },
		{ "fB", &fB },
		{ "fC", &fC },
		{ "fD", &fD }
	};

	// script (copy from c++ code)
	std::string_view atCode =
		"	                                                            \n\r\
		// length base->target, length(ad)	                            \n\r\
		fB = sqrt(fTarX * fTarX + fTarY * fTarY + fTarZ * fTarZ);	    \n\r\
		fD = sqrt(fTarX * fTarX + fTarZ * fTarZ);	                    \n\r\
		                                                                \n\r\
		// triangle angles	                                            \n\r\
		                                                                \n\r\
		// arccos(b*b + c*c - a*a) / 2bc	                            \n\r\
		fAlpha = acos((fB * fB + fC * fC - fA * fA) / (2. * fB * fC));	\n\r\
		// arccos(a*a + c*c - b*b) / 2ac	                            \n\r\
		fBeta = acos((fA * fA + fC * fC - fB * fB) / (2. * fA * fC));	\n\r\
		                                                                \n\r\
		// triangle angles to local angles	                            \n\r\
		                                                                \n\r\
		// arctan(cy-dy / length(ad))	                                \n\r\
		fAlpha = fAlpha + atan(fTarX / fD);	                            \n\r\
		// PI - beta	                                                \n\r\
		fBeta = abs(3.141592654 - fBeta);	                            \n\r\
		                                                                \n\r\
		// Y rotation	                                                \n\r\
		                                                                \n\r\
		// -arctan(z / y)	                                            \n\r\
		fGamma = -atan(fTarZ / fTarX);	                                \n\r\
		if (fTarX < 0.)	                                                \n\r\
		{	                                                            \n\r\
			fGamma = 3.141592654 + fGamma;	                            \n\r\
		}	                                                            \n\r\
		                                                                \n\r\
	";

	// ... and compile
	std::set<ts_boolean> asBools = { };
	ts_parser cTSP = ts_parser(atCode, asVars, asBools);

	// evaluate and compare to hardcoded function
	for (float fX : { -1.1f, 1.2f })
		for (float fY : { .5f, 1.9f })
			for (float fZ : { 1.3f, 1.4f })
			{
				fTarX = fX, fTarY = fY, fTarZ = fZ;
				std::cout << "Target (x/y/z) : " << fX << "/ " << fY << "/ " << fZ << "\n";

				// C++
				fAlpha = 0.f, fBeta = 0.f, fGamma = 0.f;
				IK_EndEffectorToTargetAngles(fTarX, fTarY, fTarZ, fA, fC, fAlpha, fBeta, fGamma);
				std::cout << "Alpha, Beta, Gamma (C++)          : " << fAlpha << ", " << fBeta << ", " << fGamma << "\n";

				// TinyScript++
				fAlpha = 0.f, fBeta = 0.f, fGamma = 0.f;
				cTSP.evaluate();
				std::cout << "Alpha, Beta, Gamma (TinyScript++) : " << fAlpha << ", " << fBeta << ", " << fGamma << "\n\n";
			}
}
```

Outputs currently (boolean bug present !) :

```console
Target (x/y/z) : -1.1/ 0.5/ 1.3
Alpha, Beta, Gamma (C++)          : 0.125719, 2.53366, 4.01013
Alpha, Beta, Gamma (TinyScript++) : 0.125719, 2.53366, 0.868539

Target (x/y/z) : -1.1/ 0.5/ 1.4
Alpha, Beta, Gamma (C++)          : 0.155852, 2.49532, 4.04642
Alpha, Beta, Gamma (TinyScript++) : 0.155852, 2.49532, 0.904827

Target (x/y/z) : -1.1/ 1.9/ 1.3
Alpha, Beta, Gamma (C++)          : 0.146409, 2.14222, 4.01013
Alpha, Beta, Gamma (TinyScript++) : 0.146409, 2.14222, 0.868539

Target (x/y/z) : -1.1/ 1.9/ 1.4
Alpha, Beta, Gamma (C++)          : 0.163225, 2.1157, 4.04642
Alpha, Beta, Gamma (TinyScript++) : 0.163225, 2.1157, 0.904827

Target (x/y/z) : 1.2/ 0.5/ 1.3
Alpha, Beta, Gamma (C++)          : 1.30397, 2.50087, -0.825377
Alpha, Beta, Gamma (TinyScript++) : 1.30397, 2.50087, -0.825377

Target (x/y/z) : 1.2/ 0.5/ 1.4
Alpha, Beta, Gamma (C++)          : 1.29268, 2.46413, -0.86217
Alpha, Beta, Gamma (TinyScript++) : 1.29268, 2.46413, -0.86217

Target (x/y/z) : 1.2/ 1.9/ 1.3
Alpha, Beta, Gamma (C++)          : 1.31315, 2.1196, -0.825377
Alpha, Beta, Gamma (TinyScript++) : 1.31315, 2.1196, -0.825377

Target (x/y/z) : 1.2/ 1.9/ 1.4
Alpha, Beta, Gamma (C++)          : 1.29052, 2.09343, -0.86217
Alpha, Beta, Gamma (TinyScript++) : 1.29052, 2.09343, -0.86217
```
