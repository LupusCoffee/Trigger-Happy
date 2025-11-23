#include "Core/Data/DeveloperSettings/LookTraceSettings.h"

float ULookTraceSettings::GetTraceLength()
{
	return PlayerTraceLength;
}

float ULookTraceSettings::GetTraceRadius()
{
	return PlayerTraceRadius;
}

bool ULookTraceSettings::DoesDrawDebug()
{
	return bDrawTraceDebug;
}
