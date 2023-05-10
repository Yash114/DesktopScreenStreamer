#pragma once

#include "Common.h"

class Control
{
public:
	
	static const int numStart = 0x30;
	static const int alphaStart = 0x41;
	static const int functionStart = 0x70;
	static const int numpadStart = 0x60;

	const int mouseAssignments[3] = {
		0x01, //left click
		0x04, //middle click,
		0x02, //right click
	};

	const int keyAssignments[100] = 
	{
		numStart + 0, //0
		numStart + 1, //1
		numStart + 2, //2
		numStart + 3, //3
		numStart + 4, //4
		numStart + 5, //5
		numStart + 6, //6
		numStart + 7, //7
		numStart + 8, //8
		numStart + 9, //9

		alphaStart + 0, //a
		alphaStart + 1, //b
		alphaStart + 2, //c
		alphaStart + 3, //d
		alphaStart + 4, //e
		alphaStart + 5, //f
		alphaStart + 6, //g
		alphaStart + 7, //h
		alphaStart + 8, //i
		alphaStart + 9, //j
		alphaStart + 10, //k
		alphaStart + 11, //l
		alphaStart + 12, //m
		alphaStart + 13, //n
		alphaStart + 14, //o
		alphaStart + 15, //p
		alphaStart + 16, //q
		alphaStart + 17, //r
		alphaStart + 18, //s
		alphaStart + 19, //t
		alphaStart + 20, //u
		alphaStart + 21, //v
		alphaStart + 22, //w
		alphaStart + 23, //x
		alphaStart + 24, //y
		alphaStart + 25, //z

		0xBE, //-
		0xBB, //=
		0xDB, //[
		0xDD, //]
		0xDC, //|
		0xBA, //;
		0xDE, //'
		0xBC, //,
		0xBE, //.
		0xBF, //?

		0x1B, //esc
		0x09, //tab
		0x14, //caps
		0xA0, //left shift
		0xA2, //left control
		0xA4, //Left alt
		0x20, //space
		0xA5, //right alt
		0xA3, // right control
		0x25, //left arrow
		0x28, //down arrow
		0x27, //right arrow
		0x26, //up  arrow
		0xA1, //right shift
		0x0D, //enter
		0x08, //backspace
		0x2D, //insert
		0x2E, //delete
		0xAC, //home
		0x23, //end
		0x21, //page up
		0x22, //page down
		0x90, //number lock
		0x91, //scroll lock

		functionStart + 0, //f1
		functionStart + 1, //f2
		functionStart + 2, //f3
		functionStart + 3, //f4
		functionStart + 4, //f5
		functionStart + 5, //f6
		functionStart + 6, //f7
		functionStart + 7, //f8
		functionStart + 8, //f9
		functionStart + 9, //f10
		functionStart + 10, //f11
		functionStart + 11, //f12

		numpadStart + 0, //num0
		numpadStart + 1, //num1
		numpadStart + 2, //num2
		numpadStart + 3, //num3
		numpadStart + 4, //num4
		numpadStart + 5, //num5
		numpadStart + 6, //num6
		numpadStart + 7, //num7
		numpadStart + 8, //num8
		numpadStart + 9, //num9

		//Add keyboard keycodes
	};

    bool sendInput(uint8_t);
	bool sendMouse(int, int);
	bool sendMouseButton(uint8_t);

	private:
		int binaryMask(int, int);

};

