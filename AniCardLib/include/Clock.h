#pragma once
#include <AniCardLibExportHeader.h>
#include <windows.h>
namespace AniCardLib
{
	class ANICARDLIB_SHARED Clock
	{
		LARGE_INTEGER clockSpeed , startTick , intervalTick;

	public:
		Clock();

		void Start();
		float Stop();
		float Interval();
		float fromStart();
	};
}