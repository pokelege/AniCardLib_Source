#include <Clock.h>
AniCardLib::Clock::Clock()
{
	QueryPerformanceFrequency(&clockSpeed);
}

void AniCardLib::Clock::Start()
{
	QueryPerformanceCounter(&startTick);
	QueryPerformanceCounter(&intervalTick);
}

float AniCardLib::Clock::Stop()
{
	QueryPerformanceCounter(&intervalTick);
	float result = (float)(intervalTick.QuadPart - startTick.QuadPart) / (float)clockSpeed.QuadPart;
	QueryPerformanceCounter(&startTick);
	return result;
}

float AniCardLib::Clock::Interval()
{
	LARGE_INTEGER lastinterval = intervalTick;
	QueryPerformanceCounter(&intervalTick);
	return (float)(intervalTick.QuadPart - lastinterval.QuadPart) / (float)clockSpeed.QuadPart;
}

float AniCardLib::Clock::fromStart()
{
	LARGE_INTEGER lastinterval;
	QueryPerformanceCounter(&lastinterval);
	return (float)(lastinterval.QuadPart - startTick.QuadPart) / (float)clockSpeed.QuadPart;
}