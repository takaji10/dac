#include <stdint.h>

#include <Arduino.h>

#include "ir_remote.h"

int getIRkey()
{
	int buttonCode = 0;
	int duration = 0;

	do //Wait for a start pulse > 2 seconds
	{
		duration = pulseIn(REMOTEPIN, HIGH, 15000);
	} while (duration < 2000 && duration != 0);

	if (duration == 0 || duration >= 5000)
		return 255; //No pulse or pulse too long
	if (duration < 3000)
		return 0; //Repeat the last command

	// Read the first 16 bits. Don't do anything with them
	for (int i = 0; i < 16; i++)
	{
		pulseIn(REMOTEPIN, HIGH, 3000);
	}

	// The next 8 bits represent the button pressed
	// pulses > 1 sec = 1
	// pulses < 1 sec = 0
	// pulses are delivered little-endian
	for (int power = 0; power < 8; power++)
	{
		if (pulseIn(REMOTEPIN, HIGH, 3000) > 1000)
		{
			buttonCode += 1 << power;
		}
	}

	// TODO: Do we need to read the useless trailing bits?
	for (int i = 0; i < 8; i++)
	{
		pulseIn(REMOTEPIN, HIGH, 3000);
	}
	return buttonCode;
}
