/*********************************************************************\
	myUtil.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef MATFUNC_H
#define MATFUNC_H

#define MATFUNC_FUNCTION_GAUSS                      0
#define MATFUNC_FUNCTION_PSEUDE_VOIGT               1
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#pragma warning( disable : 4996 )
#pragma warning( disable : 4099 )

typedef double (*ptFunc)(double *);

// include files
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cmath>
#include <math.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <time.h>
#include <sstream>
#include <iomanip>
#include <random>
#include "tableClass.h"
#include "alglib\\src\\interpolation.h"
#include "alglib\\src\\ap.h"
#include "alglib\\src\\optimization.h"
#include "alglib\\src\\statistics.h"

using namespace alglib;
using namespace std;

/*** Constants *****************************************************/
#define PI										3.1415926f
const double  c_pi													= 3.14159265358979323846;

/***Macros *********************************************************/
#define SAFE_DELETE(p)							{ if(p)      { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p)					{ if(p)      { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)							{ if(p)      { (p)->Release(); (p)=NULL; } }
#define COPY_ARRAY(dest,source,size,type)		{ if(source) { SAFE_DELETE_ARRAY(dest); dest = new type[size]; memcpy(dest, source, sizeof(type) * size); } }

/*** Structures ****************************************************/
struct TEXRECT		{ float top, bottom, left, right; };
struct POINT_FLOAT	{ float x, y; };

// a vector-struct in double format
struct double_vector
{
	double x, y, z;

	double_vector()									{}
	double_vector(int a)							{ x = 0; y = 0; z = 0; }
	double_vector( double x, double y, double z)	{double_vector::x = x;	double_vector::y = y; 	double_vector::z = z;}

	double_vector operator + (double_vector other)	{return double_vector(x + other.x, y + other.y, z + other.z);}
	double_vector operator - (double_vector other)	{return double_vector(x - other.x, y - other.y, z - other.z);}
	double_vector operator / (double Divisor)		{return double_vector(x / Divisor, y / Divisor, z / Divisor);}
	double_vector operator * (double Muliplikant)	{return double_vector(x * Muliplikant, y * Muliplikant, z * Muliplikant);}
		
	double_vector double_vector::Kreuz( double_vector other ){return double_vector(y*other.z - z*other.y, z*other.x - x*other.z, x*other.y - y*other.x);}
	double		  double_vector::Skalar(double_vector other ){return x*other.x + y*other.y + z*other.z;}
	double double_vector::Length(void) {return sqrt(x*x + y*y + z*z); }

	double_vector double_vector::DreheUmAchse(double_vector Achse, double Winkel)
	{
		Achse = Achse / Achse.Length();
		return (*this) * cos(Winkel) + Achse * ((1 - cos(Winkel)) * Achse.Skalar(*this)) + Achse.Kreuz(*this) * sin(Winkel);
	}

	double_vector double_vector::Bezugsystem(double_vector e1_, double_vector e2_, double_vector e3_)
	{
		return e1_ * x + e2_ * y + e3_ * z;
	}

	void EuklidToPolarDreibein(double_vector e1, double_vector e2, double_vector e3, double theta, double phi, double_vector *er, double_vector *et, double_vector *ep)
	{
		double sin_theta = sin(theta);
		double sin_phi   = sin(phi  );
		double cos_theta = cos(theta);
		double cos_phi   = cos(phi  );
		*er = (e1 * (sin_theta * cos_phi)) + (e2 * (sin_theta * sin_phi)) + (e3 * (cos_theta));
		*et = (e1 * (cos_theta * cos_phi)) + (e2 * (cos_theta * sin_phi)) - (e3 * (sin_theta));
		*ep = (e1 * (       -1 * sin_phi)) + (e2 * (            cos_phi));
	}
};

// shape classes
struct shapeClass
{
	static const unsigned int	typeUndefined	= 0;
	static const unsigned int	typeRect		= 1;
	static const unsigned int	typeCircle		= 2;
	unsigned int				type;

	virtual void render			(bool userFunc(int x, int y, void *pUser), unsigned int *numRenderedPixels, void* pUser) {};
	virtual bool getNextPixel	(int &x, int &y, bool start) { return false; };
};

struct circleClass : shapeClass
{
	double centerX, centerY, radius;
	void	render				(bool userFunc(int x, int y, void *pUser), unsigned int *numRenderedPixels, void* pUser);
	bool	getNextPixel		(int &x, int &y, bool start);
};

struct rectClass : shapeClass
{
public:
	double left, right, top, bottom;
			rectClass			(double left, double right, double top, double bottom);
	void	render				(bool userFunc(int x, int y, void *pUser), unsigned int *numRenderedPixels, void* pUser);
	bool	getNextPixel		(int &x, int &y, bool start);
};

/*** Functions ***************************************************/

// Mathematical Functions
double		interpolateValue										(double x1, double x2, double y1, double y2, double &x);
void		interpolateValues										(unsigned int numPointsToInterpolate, double xSource[], double ySource[], double *interpolatedValues, unsigned int numTargetPoints, double xTarget[], double yTarget[], double targetValues[], bool biCubic);
void		extrapolateValues										(unsigned int numDataPoints, double *xData, double *yData, unsigned int numTargetPoints, double *xTarget, double *yTarget, unsigned int numFitParameters, double *fitParameter, ptFunc fitFunction, ptFunc* gradient);
int			compareDouble											(const void * a, const void * b);
double		integrate												(double f(double x), double lowerLimit, double upperLimit, double stepSize);
double		calcFWHM												(double f(double x), double xMin, double xMax, double x0, double yDelta);
double		invertMonotonFunction									(double f(double x, void* userPointer), double y, void* userPointer, double xMin, double xMax, double deltaPrecision, bool xLog, bool yLog);
//void		performPeakAnalysis										(table<double> * &peakTable, double xData[], double yData[], unsigned int numDataPoints, double minDelta, unsigned functionType);
bool		findCentersBetweenPeaks									(double xData[], double yData[], unsigned int numDataPoints, unsigned int &numPeaks, unsigned int * &startingIndex, unsigned int * &peakPosition, unsigned int * &width, double minDelta);
void		fitToNonLinearFunction									(unsigned int numParameters, double *fitParameter, ptFunc function, ptFunc* gradient, double xData[], double yData[], unsigned int numDataPoints);
void		fitDoublet												(unsigned int numParameters, double *fitParameter, ptFunc function, ptFunc* gradient, double xData[], double yData[], unsigned int numDataPoints);
//void		foldDataPointsWithFunction								(table<double>  &spectrum, ptFunc function, double * parameter, table<double>  &datapoints, double xDelta, double xBorder);
double		calcIntegralWidth										(double xData[], double yData[], unsigned int numDataPoints);
double		calcFWHM												(double xData[], double yData[], unsigned int numDataPoints);
double		gaussFunction											(double *p);
double		gradientOfGaussFunction_y0								(double *p);
double		gradientOfGaussFunction_x0								(double *p);
double		gradientOfGaussFunction_w								(double *p);
double		pseudoVoightFunction									(double *p);
double		gradientOfPseudoVoightFunction_x0						(double *p);
double		gradientOfPseudoVoightFunction_w						(double *p);
double		gradientOfPseudoVoightFunction_n						(double *p);
double		gradientOfPseudoVoightFunction_y0						(double *p);
double		pseudoVoigtFunction										(double *p);
double		gradientOfPseudoVoigtFunction_x0						(double *p);
double		gradientOfPseudoVoigtFunction_wg						(double *p);
double		gradientOfPseudoVoigtFunction_wl						(double *p);
double		gradientOfPseudoVoigtFunction_n							(double *p);
double		gradientOfPseudoVoigtFunction_y0						(double *p);
double		doNothing												(double x);
double		inverseValue											(double x);
double		divide													(double dividend, double divisor);
double		dotProduct												(double vx1, double vy1, double vx2, double vy2);
bool		isIntegerValue											(double a, double delta);
bool		isEqual													(double a, double b, double delta);
bool		isValuePositive											(float  value);
bool		isValueNegative											(float  value);
bool		isValuePositive											(double value);
bool		isInsideRect											(double left, double top, double right, double bottom, double xPos, double yPos);
bool		isInsideCirCle											(double circleCenterX, double circleCenterY, double radius, double xPos, double yPos);
bool		doesLineCrossRectangle									(double x1, double y1, double x2, double y2, double rx[], double ry[]);
bool		isPointInsideParallelogram								(double x,  double y,  double rx[], double ry[]);
bool		findProduct												(unsigned int &factor1, unsigned int &factor2, unsigned int product, double factor1DividedByFactor2);
bool		isThereAnyPeak											(unsigned int numValues, double *data, double threshold);
double		getAngleInPolarCoord									(double x, double y);
float		returnMax												(float a, float b, float c);
float		FindeNullstelle											(float f(float x), float von, float bis, float Genauigkeit);
float		X_percent_are_smaller_than								(float x, DWORD AmountNumbers, float *Numbers);
float		The_BIGGEST_Number_Is									(DWORD AmountNumber, float *Numbers);
float		RandBetweenZeroAndOne									(void);
float		RandBetweenMinusOneAndPlusOne							(void);
void		LoeseQuadratischeGleichung								(float a, float b, float c, float *x1, float *x2);
BOOL		LoeseLGS												(DWORD AnzGleichungen, DWORD AnzUnbekannte, float *Ergebnis, float *Koeffizienten, float *Inhomogenitaet);
BOOL		LoeseLGS												(DWORD AnzGleichungen, DWORD AnzUnbekannte, float *Ergebnis, D3DXVECTOR3 *Koeffizienten, D3DXVECTOR3 *Inhomogenitaet);
void		LoeseLGSmitGauss										(DWORD size, float *result, float *coeff, float *inhomo);
void		fourierTransformation									(unsigned int numInputValues, float *input, unsigned int numSpectrumValues, float *spectrum);
float		fourierTransformation									(float f(float x), float k, float von, float bis, unsigned int steps);
float		integrate												(float f(float x), float von, float bis, unsigned int steps);
float	   *differentiate											(unsigned int numColumns, float *function, float xmin, float xmax);
bool		differentiate											(unsigned int numColumns, float *function, float xmin, float xmax, float *differentiatedFunction);
bool		differentiate											(unsigned int numColumns, double *function, double xmin, double xmax, double *differentiatedFunction);
void		integrateWithEuler										(double *orbit, unsigned int dim, double func(double *x, double t, unsigned int curDim), double *initValue, double stepSize, unsigned int numSteps);
void		integrateWithRungeKutta									(double *orbit, unsigned int dim, double func(double *x, double t, unsigned int curDim), double *initValue, double stepSize, unsigned int numSteps);
float		eulerIteration											(float f(float x), float x, float dt);
float		improvedEulerIteration									(float f(float x), float x, float dt);
float		rungeKuttaIteration										(float f(float x), float x, float dt);
void		makeGraphInPlainXY										(unsigned int numColumns, float xmin, float xmax, GR3D_POINT_VTX *vector, DWORD color, float *data);
void		normalize												(unsigned int columns, float xmin, float xmax, float value, float *data);

// Usefull Functions
void		makeNumberStr											(char *str, double value, double minValue, double maxValue, char *invalidStr, unsigned int length);
void		swapPointers											(void **pt1, void **pt2);
char *		makeString												(char str[], unsigned int length);
void		makeString												(char *dest, char source[], unsigned int length);
void		addString												(char *dest, char source[], unsigned int length);
bool		showMessageAndQuit										(char *msg, bool returnValue);
double		strToTime												(const char * str, const char *timeFormat, char decimalSeparator);
void		timeToStr												(stringstream &ss, const char *timeFormat, double secondsPassedSince1900, unsigned int secondsPrecision);
bool		testTimeToStrConversion									(unsigned int numTests, unsigned int verbosity, bool stopOnError);
void		correctSummerWinterTime									(tm &tmTime, double &myTime);
void		convertTime												(SYSTEMTIME &source, tm &target, double &secondsPassedSince1900);
void		addTabs													(ostream      &ss, unsigned int strLength, unsigned int tabLength, unsigned int lengthToFill);
bool		addArgumentToArgv										(int &argc, char **&argv, char additionalArg[]);
bool		seperateFileAndPath										(const char filePath[], char *path, char *file);
bool		seperateFilePathAndEnding								(string &filePath, string *path, string *file, string *ending);
void		getRainbowColor											(double valueFromZeroToOne, RGBTRIPLE &color);
double		polynom													(vector<double> *coefficients, double x);
double		integerPower											(double basis, unsigned int exponent);
void		exchangeVariables										(float *a, float *b);

#endif
