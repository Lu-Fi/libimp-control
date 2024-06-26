#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "imp_control_help.h"
#include "imp_control_video.h"
#include "imp_control_util.h"
#include "include/imp_log.h"

// Function to generate random numbers within a specified range
int randomRange(int min, int max) {
	return min + rand() % (max - min + 1);
}

// Declaration of IMPTune function
char *IMPTune(int fd, char *tokenPtr);

// Function to demonstrate Mask settings
char* MaskDemo() {
	srand(time(NULL)); // Seed the random number generator
	IMP_LOG_INFO(TAG, "Starting Mask Demo\n");

	time_t start, now;
	time(&start);

	while (1) {
		time(&now);
		if (difftime(now, start) >= 10) break;

		char command[256];
		snprintf(command, sizeof(command), "mask 0 1 %d %d 320 240 %d %d %d",
			randomRange(0, 640), randomRange(0, 1024),
			randomRange(0, 255), randomRange(0, 255), randomRange(0, 255));

		IMP_LOG_INFO(TAG, "Command: %s\n", command);
		IMPTune(0, command);
		usleep(500000);
	}

	char finalCommand[] = "mask 0 0 516 34 320 240 49 98 163";
	IMP_LOG_INFO(TAG, "Final Command: %s\n", finalCommand);
	IMPTune(0, finalCommand);

	return "Mask demo complete";
}

// Function to ramp control values for various settings
void rampControlValue(const char *controlName) {
	int value;
	int (*setFunctionInt)(int) = NULL;
	int (*setFunctionChar)(unsigned char) = NULL;
	int (*setFunctionUint)(uint32_t) = NULL;

	// Assign appropriate function based on control name
	if (strcmp(controlName, "brightness") == 0) {
		setFunctionChar = &IMP_ISP_Tuning_SetBrightness;
	} else if (strcmp(controlName, "contrast") == 0) {
		setFunctionChar = &IMP_ISP_Tuning_SetContrast;
	} else if (strcmp(controlName, "saturation") == 0) {
		setFunctionChar = &IMP_ISP_Tuning_SetSaturation;
	} else if (strcmp(controlName, "sharpness") == 0) {
		setFunctionChar = &IMP_ISP_Tuning_SetSharpness;
	} else if (strcmp(controlName, "sinter") == 0) {
		setFunctionUint = &IMP_ISP_Tuning_SetSinterStrength;
	} else if (strcmp(controlName, "temper") == 0) {
		setFunctionUint = &IMP_ISP_Tuning_SetTemperStrength;
#ifndef CONFIG_T21
	} else if (strcmp(controlName, "aecomp") == 0) {
		setFunctionInt = &IMP_ISP_Tuning_SetAeComp;
#else
	IMP_LOG_ERR(TAG, HELP_MESSAGE_PLATFORM);
#endif
#ifndef CONFIG_T20
	} else if (strcmp(controlName, "dpc") == 0) {
		setFunctionUint = &IMP_ISP_Tuning_SetDPC_Strength;
#else
	IMP_LOG_ERR(TAG, HELP_MESSAGE_PLATFORM);
#endif
	} else if (strcmp(controlName, "drc") == 0) {
#ifndef CONFIG_T20
		setFunctionUint = &IMP_ISP_Tuning_SetDRC_Strength;
#else
	IMP_LOG_ERR(TAG, HELP_MESSAGE_PLATFORM);
#endif
	} else if (strcmp(controlName, "hue") == 0) {
#ifndef CONFIG_T20
		setFunctionChar = &IMP_ISP_Tuning_SetBcshHue;
#else
	IMP_LOG_ERR(TAG, HELP_MESSAGE_PLATFORM);
#endif
	}

	// Ramping logic for the control values
	for (value = 127; value >= 1; value--) {
		if (setFunctionInt) setFunctionInt(value);
		else if (setFunctionChar) setFunctionChar(value);
		else if (setFunctionUint) setFunctionUint(value);
		usleep(10000); // 0.05 seconds
	}

	for (value = 1; value <= 255; value++) {
		if (setFunctionInt) setFunctionInt(value);
		else if (setFunctionChar) setFunctionChar(value);
		else if (setFunctionUint) setFunctionUint(value);
		usleep(10000); // 0.05 seconds
	}

	for (value = 255; value >= 127; value--) {
		if (setFunctionInt) setFunctionInt(value);
		else if (setFunctionChar) setFunctionChar(value);
		else if (setFunctionUint) setFunctionUint(value);
		usleep(10000); // 0.05 seconds
	}
}

// Function to demonstrate White Balance settings
char* WhitebalanceDemo() {
	IMP_LOG_INFO(TAG, "Starting Whitebalance Demo\n");
	for (int wb = 1; wb <= 9; wb++) {
		char command[256];
		snprintf(command, sizeof(command), "whitebalance %d", wb % 10);
		IMP_LOG_DBG(TAG,"Command: %s\n", command);
		IMPTune(0, command);
		sleep(2); // 1 second
	}

	return "Whitebalance demo complete";
}

// Function to demonstrate Auto Zoom feature
char* AutoZoomDemo() {
	IMP_LOG_INFO(TAG, "Starting AutoZoom Demo\n");

	// Use integer steps to avoid floating-point precision issues
	int zoomStep;
	const int baseZoom = 100; // Represents 1.0
	const int zoomIncrement = 10; // Represents 0.1
	const int maxZoom = 180; // Represents 1.3 - 1.8 max tested

	// Ramp Up
	for (zoomStep = baseZoom; zoomStep <= maxZoom; zoomStep += zoomIncrement) {
		double zoom = zoomStep / 100.0;
		int outWidth = (int)(1920 * zoom);
		int outHeight = (int)(1080 * zoom);

		char command[256];
		snprintf(command, sizeof(command), "autozoom 0 1 %d %d 1 0 0 1920 1080", outWidth, outHeight);
		IMP_LOG_DBG(TAG, "Autozoom: %.1f\n", zoom); // Debug print
		IMP_LOG_DBG(TAG, "Command: %s\n", command); // Debug print
		IMPTune(0, command);
		sleep(1); // 1 second
	}

	sleep(2);

	// Ramp Down
	for (zoomStep = maxZoom; zoomStep >= baseZoom; zoomStep -= zoomIncrement) {
		double zoom = zoomStep / 100.0;
		int outWidth = (int)(1920 * zoom);
		int outHeight = (int)(1080 * zoom);

		char command[256];
		snprintf(command, sizeof(command), "autozoom 0 1 %d %d 1 0 0 1920 1080", outWidth, outHeight);
		IMP_LOG_DBG(TAG, "Autozoom: %.1f\n", zoom); // Debug print
		IMP_LOG_DBG(TAG, "Command: %s\n", command); // Debug print
		IMPTune(0, command);
		sleep(1); // 1 second
	}

	char finalCommand[] = "autozoom 0 0 1920 1080 0 0 0 1920 1080";
	IMP_LOG_DBG(TAG, "Final Command: %s\n", finalCommand); // Debug print
	IMPTune(0, finalCommand);

	return "Autozoom demo complete";
}

// Function to demonstrate Front Crop feature
char* FrontCropDemo() {
	IMP_LOG_INFO(TAG, "Starting Front Crop Demo\n");

	// Pan Down
	for (int v = 0; v <= 350; v++) {
		char command[1024];
		snprintf(command, sizeof(command), "frontcrop 1 %d 0 1280 720", v);
		IMPTune(0, command);
		usleep(8000); // 0.008 seconds
	}


	for (int v = 0; v <= 600; v++) {
		char command[1024];
		snprintf(command, sizeof(command), "frontcrop 1 350 %d 1280 720", v);
		IMPTune(0, command);
		usleep(8000); // 0.008 seconds
	}

	for (int v = 350; v >= 0; v--) {
		char command[1024];
		snprintf(command, sizeof(command), "frontcrop 1 %d 600 1280 720", v);
		IMPTune(0, command);
		usleep(8000); // 0.008 seconds
	}

	for (int h = 600; h >= 0; h--) {
		char command[1024];
		snprintf(command, sizeof(command), "frontcrop 1 0 %d 1280 720", h);
		IMPTune(0, command);
		usleep(8000); // 0.008 seconds
	}

	sleep(1);

	char finalCommand[] = "frontcrop 1 0 0 1920 1080";
	IMP_LOG_DBG(TAG, "Final Command: %s\n", finalCommand); // Debug print
	IMPTune(0, finalCommand);

	return "Front Crop demo complete";
}

void ispdemo() {
IMP_LOG_INFO(TAG, "Starting ISP Mode demo\n");
IMP_LOG_INFO(TAG, "Setting night mode\n");
IMP_ISP_Tuning_SetISPRunningMode(1);
sleep(2);
IMP_LOG_INFO(TAG, "Setting day mode\n");
IMP_ISP_Tuning_SetISPRunningMode(0);
}

// Function to run the full demo sequence
char* fullDemo(char *tokenPtr) {
	char *p = strtok_r(tokenPtr, " \t\r\n", &tokenPtr);

	if (p != NULL && strcmp(p, "-h") == 0)
{
	return HELP_MESSAGE_FULLDEMO;
}
	IMP_LOG_INFO(TAG, "Starting Full Demo\n");
	const char *controls[] = {"brightness", "contrast", "saturation", "sharpness", "sinter", "temper", "aecomp", "dpc", "drc", "hue"};
	int numControls = sizeof(controls) / sizeof(controls[0]);

	MaskDemo();
	sleep(1);
	WhitebalanceDemo();
	sleep(1);
	if (p != NULL && strcmp(p, "-f") == 0)
	{
		// Autozoom requires performant SOCs for now
		FrontCropDemo();
		sleep(1);
		AutoZoomDemo();
		sleep(1);
	}
	ispdemo();
	sleep(1);
	// Ramp all the controls
	for (int i = 0; i < numControls; i++) {
		IMP_LOG_INFO(TAG, "Ramping %s\n", controls[i]);
		rampControlValue(controls[i]);
		IMP_LOG_INFO(TAG, "Ramping %s complete\n", controls[i]);
	}
	IMP_LOG_ERR(TAG, "Demo complete, restart for best performance\n");
	return "Demo complete, restart for best performance.";
}
