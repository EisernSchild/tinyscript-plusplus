// SPDX-License-Identifier: Zlib
/*
 * TINYEXPR - Tiny recursive descent parser and evaluation engine in C
 *
 * Copyright (c) 2015-2020 Lewis Van Winkle
 *
 * http://CodePlea.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgement in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

 /*
  * TINYEXPR++ - Tiny recursive descent parser and evaluation engine in C++
  *
  * Copyright (c) 2020-2024 Blake Madden
  *
  * C++ version of the TinyExpr library.
  *
  * This software is provided 'as-is', without any express or implied
  * warranty. In no event will the authors be held liable for any damages
  * arising from the use of this software.
  *
  * Permission is granted to anyone to use this software for any purpose,
  * including commercial applications, and to alter it and redistribute it
  * freely, subject to the following restrictions:
  *
  * 1. The origin of this software must not be misrepresented; you must not
  * claim that you wrote the original software. If you use this software
  * in a product, an acknowledgement in the product documentation would be
  * appreciated but is not required.
  * 2. Altered source versions must be plainly marked as such, and must not be
  * misrepresented as being the original software.
  * 3. This notice may not be removed or altered from any source distribution.
  */

  /*
   * TINYSCRIPT++ - Tiny script parser based on TinyExpr in C++
   *
   * Copyright (c) 2024 Denis Reischl
   *
   * This software is provided 'as-is', without any express or implied
   * warranty. In no event will the authors be held liable for any damages
   * arising from the use of this software.
   *
   * Permission is granted to anyone to use this software for any purpose,
   * including commercial applications, and to alter it and redistribute it
   * freely, subject to the following restrictions:
   *
   * 1. The origin of this software must not be misrepresented; you must not
   * claim that you wrote the original software. If you use this software
   * in a product, an acknowledgement in the product documentation would be
   * appreciated but is not required.
   * 2. Altered source versions must be plainly marked as such, and must not be
   * misrepresented as being the original software.
   * 3. This notice may not be removed or altered from any source distribution.
   */

// dont forget to define that in "tinyexpr.cpp" as well if using float
#define TE_FLOAT

#include "../tinyscript.h"
#include <iostream>

#define PI 3.141592654f

/// <summary>
/// C++ version of the used function
/// (Inverse Kinematics function)
/// </summary>
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

	// TinyScript++ version of the used function
	// (Inverse Kinematics function)
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