#include "Control.h"
#include <windows.h>
#include <iostream>


bool Control::sendInput(uint8_t keyValue) {
	
	uint8_t depressed = keyValue >> 7; //read the first bit
	uint8_t keyCode = binaryMask(keyValue, 0x7F); //ignore the 8th bit
	
	if (depressed == 1) { 
		keybd_event(keyAssignments[keyCode], 0, 0, 0); //Press down the Key
		return true;

	}

	keybd_event(keyAssignments[keyCode], 0, KEYEVENTF_KEYUP, 0); //Release the Key
	return true;
}

bool Control::sendMouse(int xpos, int ypos) {

	POINT mousePos;


	GetCursorPos(&mousePos);

	INPUT inputs[1] = {};
	ZeroMemory(inputs, sizeof(inputs));

	inputs[0].type = INPUT_MOUSE;

	tagMOUSEINPUT mouseInputData{};

	mouseInputData.dx = (int)(xpos) / 2;
	mouseInputData.dy = (int)(ypos) / 2;
	mouseInputData.dwFlags = MOUSEEVENTF_MOVE;


	inputs[0].mi = mouseInputData;

	SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));

	return true;
}

bool Control::sendMouseButton(uint8_t input) {
	
	uint8_t depressed = input >> 7; //read the first bit
	uint8_t button = binaryMask(input, 0x7F);

	INPUT inputs[1] = {};
	ZeroMemory(inputs, sizeof(inputs));

	inputs[0].type = INPUT_MOUSE;

	tagMOUSEINPUT mouseInputData{};
	

	switch (button) {
	
		case (0):
			mouseInputData.dwFlags = depressed == 1 ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
			break;

		case (1):

			mouseInputData.dwFlags = depressed == 1 ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
			break;

		case (2):

			mouseInputData.dwFlags = depressed == 1 ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
			break;

		default:
			break;
	}

	inputs[0].mi = mouseInputData;

	SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));

	return true;

}

int Control::binaryMask(int mask, int input) {
	return 255 & mask & input;
}