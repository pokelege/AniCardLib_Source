#pragma once
#include <math.h>
#include <memory>
#include <AniCardLibExportHeader.h>
struct ANICARDLIB_SHARED MathHelpers
{
	static float Determinant( float* m1 , int size );

	static float cofactor( float* matrix , int size , int i , int j );

	static void preAdj( float* matrix , int size );

	static void Transpose( float* m1 , int size );

	static void Multiply( float* matrix , float toMultiply , int size );
	static void MultiplyVector( float* matrix , float* vector , int size );
};