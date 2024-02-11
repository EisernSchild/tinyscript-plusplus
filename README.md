
# TinyScript++

Tiny script parser and compiler, based on [*TinyExpr*](https://github.com/codeplea/tinyexpr)/[*TinyExpr++*](https://github.com/Blake-Madden/tinyexpr-plusplus). Embed a simple scripting language within your C++ project.

## Features

! Note that this project is in alpha stage and NOT (!) fully tested !

- **C++17 with no dependencies**.
- Only three files : Single source/header file (TinyScript), source and header file (TinyExpr).
- Simple and fast.
- Implements TinyExpr for expression statements.
- Floating point/Boolean expression statements and If statements
- Released under the zlib license - free for nearly any use.
- Easy to use and integrate with your code.
- Thread-safe; parser is in a self-contained object.

## Building

add following files to your project :

    * "tinyscript.h"
    * "tinyexpr.h"
    * "tinyexpr.cpp"

## Short Example

Here is a minimal example to evaluate an expression at runtime.

```cpp
#include "tinyscript.h"

float fX{ 2.f }, fY{ 3.f };
std::set<ts_variable> asVars = { { "fX", &fX }, { "fY", &fY } };
bool b0 = true, b1 = true, b2 = false, b3 = false, b4 = false;
std::set<ts_boolean> asBools = {
	{ "b0", &b0 },
	{ "b1", &b1 },
	{ "b2", &b2 },
	{ "b3", &b3 }
};

std::string_view atCode =
	"	\n\r\
		b0 = fX < fY;\n\r\
		b1 = (((((fX >= fY)))));\n\r\
		b2 = (((((fX == fY) && b1) || (fY < 1.0)));\n\r\
		b3 = (b2 && true ) || false;\n\r\
		if (b0) fX = 0.999;\n\r\
		if (b1) fY = 1.999;\n\r\
		if (b2) { fY = 2.999; }\n\r\
		if (b3)\n\r\
		{\n\r\
			fX = 4.9;\n\r\
		}\n\r\
		if (b3)\n\r\
			fY = 5.4;\n\r\
		if (b1) {\n\r\
			fX = .11;\n\r\
		}\n\r\
		if (b2) { fX = 2.1;\n\r\
		}\n\r\
		fY = sin(fY + fX);\n\r\
		fX = 2.5 * fY;\n\r\
	";

ts_parser cTSP = ts_parser(atCode, asVars, asBools);
cTSP.evaluate();
```
