#include <MathHelpers.h>

float MathHelpers::Determinant( float* m1 , int size )
{
	if ( size == 1 )
	{
		return m1[0];
	}
	else
	{
		float result = 0;
		for ( int i = 0; i < size; i++ )
		{
			float* temp = 0;
			temp = new float[( size - 1 ) * ( size - 1 )];

			for ( int l = 0; l < size - 1; l++ )
			{
				for ( int j = 0; j < size - 1; j++ )
				{
					if ( l < i )
					{
						temp[( l * ( size - 1 ) ) + j] = m1[( l * size ) + j + 1];
					}
					else
					{
						temp[( l * ( size - 1 ) ) + j] = m1[( ( l + 1 ) * size ) + j + 1];
					}
					//if ( ( l * ( size - 1 ) ) + j >= ( size - 1 ) * ( size - 1 ) || (l * (size - 1 )) + j >= ( size - 1 ) * ( size - 1 ) ) std::cout << "went out" << std::endl;
				}
			}

			if ( i % 2 == 0 )
			{
				result += m1[( i * size )] * MathHelpers::Determinant( temp , size - 1 );
			}
			else
			{
				result -= m1[( i * size )] * MathHelpers::Determinant( temp , size - 1 );
			}
			//std::cout << size - 1 << std::endl;
			delete[] temp;
		}
		return result;
	}
}

float MathHelpers::cofactor( float* matrix , int size , int i , int j )
{
	float* gets = new float[( size - 1 )* ( size - 1 )];

	for ( int b = 0; b < size - 1; b++ )
	{
		for ( int a = 0; a < size - 1; a++ )
		{
			int x = a;
			int y = b;
			if ( i <= a )
			{
				++x;
			}
			if ( j <= b )
			{
				++y;
			}
			gets[( b * ( size - 1 ) ) + a] = matrix[( y * size ) + x];
		}
	}


	return powf( ( -1.0f ) , ( float ) i + ( float ) j ) * MathHelpers::Determinant( gets , size - 1 );
}

void MathHelpers::preAdj( float* matrix , int size )
{
	float* temp = new float[size * size];

	for ( int j = 0; j < size; ++j )
	{
		for ( int i = 0; i < size; ++i )
		{
			temp[( j* size ) + i] = cofactor( matrix , size , i , j );
		}
	}
	memcpy( matrix , temp , sizeof( float ) * ( size * size ) );
	delete[] temp;
}

void MathHelpers::Transpose( float* m1 , int size )
{
	float* result = new float[size * size];

	for ( int j = 0; j < size; j++ )
	{
		for ( int i = 0; i < size; i++ )
		{
			result[( j * size ) + i] = m1[( i * size ) + j];
		}

	}
	memcpy( m1 , result , sizeof( float ) * ( size * size ) );
	delete[] result;
}

void MathHelpers::Multiply( float* matrix , float toMultiply , int size )
{
	for ( int i = 0; i < size*size; ++i )
	{
		matrix[i] *= toMultiply;
	}
}
void MathHelpers::MultiplyVector( float* matrix , float* vector , int size )
{
	float* vectorCopy = new float[size];
	memcpy( vectorCopy , vector , sizeof( float ) * size );
	for ( int y = 0; y < size; ++y )
	{
		float theSolution = 0;
		for ( int x = 0; x < size; ++x )
		{
			theSolution += matrix[( y * size ) + x] * vectorCopy[x];
		}
		vector[y] = theSolution;
	}
	delete[] vectorCopy;
}