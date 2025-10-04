/*********************************************************************\
	myUtil.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "myUtil.h"

//-----------------------------------------------------------------------------
// Name: FindeNullstelle()
// Desc: Findet die Nullstelle der übergebenen float-Funktion
//		 Voraussetzung ist das f() stetig ist.
//-----------------------------------------------------------------------------
float FindeNullstelle(float f(float x), float von, float bis, float Genauigkeit)
{
	float x1, x2, y1, y2, m;

	// Vorzeichenwechsel ?
	y1 = f(von);		y2 = f(bis);
	if (((y1 > 0) && (y2 > 0)) || ((y1 < 0)&&(y2 < 0)))
	{
		return 0;	// Not a Number
	}
	else if (y1 < 0)	{ x1 = von; x2 = bis; }
	else				{ x2 = von; x1 = bis; }

	// Intervall-Halbierungsmethode
	while (fabs(x2-x1) > Genauigkeit)
	{
		m = (x1 + x2) / 2.0f;
		if (f(m) >= 0)	x2 = m;
		else			x1 = m;
	};
	return (x1 + x2) / 2.0f;
}

//-----------------------------------------------------------------------------
// Name: MittelVektor()
// Desc: Suche die Zahl, die größer als x-Prozent der Zahlen ist.
//-----------------------------------------------------------------------------
int compare(const void *elem1, const void *elem2 )
{
		 if (*((float*)elem1)  < *((float*)elem2))	return -1;
	else if (*((float*)elem1) == *((float*)elem2))	return  0;
	else											return  1;
}
float	X_percent_are_smaller_than(float x, DWORD AmountNumbers, float *Numbers)
{
	// Allocate Memory for sorted Numbers
	float	*NewNumbers = new float[AmountNumbers];
	if (x >= 1) return 0;

	// Sort
	memcpy(NewNumbers, Numbers, sizeof(float) * AmountNumbers);
	qsort(NewNumbers, AmountNumbers, sizeof(float), compare); 

	x = NewNumbers[(DWORD)(x * 	AmountNumbers)];

	delete NewNumbers;

	return x;
}

//-----------------------------------------------------------------------------
// Name: The_BIGGEST_Number_Is()
// Desc: return the BIGGEST Value of the array
//-----------------------------------------------------------------------------
float The_BIGGEST_Number_Is(DWORD AmountNumber, float *Numbers)
{
	float biggest_number = -1E30f;
	for (DWORD a = 0; a < AmountNumber; a++) if (Numbers[a] > biggest_number) biggest_number =  Numbers[a];
	return biggest_number;
}

//-----------------------------------------------------------------------------
// Name: RandBetweenZeroAndOne
// Desc: return (float) rand() / RAND_MAX
//-----------------------------------------------------------------------------
float RandBetweenZeroAndOne(void)
{
	return (float) rand() / RAND_MAX;
}

//-----------------------------------------------------------------------------
// Name: RandBetweenMinusOneAndPlusOne
// Desc: return (float) rand() / RAND_MAX * 2 - 1.0f ;
//-----------------------------------------------------------------------------
float RandBetweenMinusOneAndPlusOne(void)
{
	return (float) rand() / RAND_MAX * 2 - 1.0f;
}

//-----------------------------------------------------------------------------
// Name: LoeseQuadratischeGleichung
// Desc: a*x²+b*x+c = 0 nach x aufgeloest
//-----------------------------------------------------------------------------
void LoeseQuadratischeGleichung(float a, float b, float c, float *x1, float *x2)
{
	*x1 = (float)sqrt(b*b-4*a*c);
	*x2 = (-b +(*x1))  / (2*a);
	*x1 = (-b -(*x1))  / (2*a);
}

//-----------------------------------------------------------------------------
// Name: GaussChangeRow()
// Desc: Vertauscht Zeile a mit Zeile b
//-----------------------------------------------------------------------------
void GaussChangeRow(DWORD size, DWORD arraywidth, DWORD a, DWORD b, float *coeff, float *inhomo)
{
	float tmp;

	for (unsigned int i = 0; i < size; i++)
	{
		tmp							= coeff[a * arraywidth + i];
		coeff[a * arraywidth + i]	= coeff[b * arraywidth + i];
		coeff[b * arraywidth + i]	= tmp;
	}

	tmp			= inhomo[a];
	inhomo[a]	= inhomo[b];
	inhomo[b]	= tmp;
}

//-----------------------------------------------------------------------------
// Name: GaussAddRow()
// Desc: Addiert Vielfaches von Zeile a zu Zeile b
//-----------------------------------------------------------------------------
void GaussAddRow(DWORD size, DWORD arraywidth, DWORD a, DWORD b, float times, float *coeff, float *inhomo)
{
 	for (unsigned int i = 0; i < size; i++)
	{
		coeff[b * arraywidth + i]	+= coeff[a * arraywidth + i] * times;
	}

	inhomo[b] += times * inhomo[a];
}

//-----------------------------------------------------------------------------
// Name: GaussAlgorithmLeftColumn()
// Desc: Addiert, multipliziert Zeilen miteinander und macht somit die linke Spalte null.
//		 Dann folgt rekursiver Aufruf um linkes unteres Dreieck zu nullen
//-----------------------------------------------------------------------------
void GaussAlgorithmLeftColumn(DWORD size, DWORD arraywidth, float *coeff, float *inhomo)
{
	// Locals
	unsigned int a, rowWidthBiggest;
	float        biggest;

	// Suche Größten Koeffizienten in der linken Spalte
	for (biggest = 0.0f, a = 0, rowWidthBiggest = 0; a < size; a++)
	{
		if ((float)fabs(biggest) < (float)fabs(coeff[a * arraywidth + 0]))
		{
			biggest			= coeff[a * arraywidth + 0];
			rowWidthBiggest	= a;
		}
	}

	// Tausche Zeile mit größtem Koeefizieten mit der ersten Zeile
	GaussChangeRow(size, arraywidth, rowWidthBiggest, 0, coeff, inhomo);

	// Ziehe "vielfaches" der ersten Zeile von jeder Zeile ab.
	for (a = 1; a < size; a++)
		GaussAddRow(size, arraywidth, 0, a, -1 * coeff[a * arraywidth] / biggest, coeff, inhomo);

	// Weiter nullen ?
	if (size > 2)
		GaussAlgorithmLeftColumn(size - 1, arraywidth, &coeff[arraywidth + 1], &inhomo[1]);
}

//-----------------------------------------------------------------------------
// Name: GaussAlgorithmRightColumn()
// Desc: Addiert, multipliziert Zeilen miteinander und macht somit die rechte Spalte null.
//		 Dann folgt rekursiver Aufruf um rechtes oberes Dreieck zu nullen
//-----------------------------------------------------------------------------
void GaussAlgorithmRightColumn(DWORD size, DWORD arraywidth, float *coeff, float *inhomo)
{
	// Locals
	unsigned int a;
	float		 biggest = coeff[(size-1) * arraywidth + size-1];

	// Ziehe "vielfaches" der letzten Zeile von jeder Zeile ab.
	for (a = 0; a < size-1; a++)
		GaussAddRow(size, arraywidth, size-1, a, -1 * coeff[a * arraywidth + size - 1] / biggest, coeff, inhomo);

	// Weiter nullen ?
	if (size > 2)
		GaussAlgorithmRightColumn(size - 1, arraywidth, coeff, inhomo);
}

//-----------------------------------------------------------------------------
// Name: LoeseLGSmitGauss()
// Desc: ACHTUNG: LGS muss quadratisch sein und daten werden verändert.
//-----------------------------------------------------------------------------
void LoeseLGSmitGauss(DWORD size, float *result, float *coeff, float *inhomo)
{
//	float *myCoeff  = new float[size * size];
//	float *myInhomo = new float[size];

//	memcpy(myCoeff , coeff , sizeof(float) * size * size);
//	memcpy(myInhomo, inhomo, sizeof(float) * size);

	GaussAlgorithmLeftColumn (size, size, coeff, inhomo);

	GaussAlgorithmRightColumn(size, size, coeff, inhomo);

	for (unsigned int a = 0; a < size; a++) result[a] = inhomo[a] / coeff[a * size + a];

//	delete [] myCoeff;
//	delete [] myInhomo;
}

//-----------------------------------------------------------------------------
// Name: LoeseLGS()
// Desc: 
//			Koeffizienten[AnzUnbekannte][AnzGleichungen]
//			Ergebnis[AnzUnbekannte]
//			Inhomogenitaet[AnzGleichungen]
//-----------------------------------------------------------------------------
BOOL LoeseLGS(DWORD AnzGleichungen, DWORD AnzUnbekannte, float *Ergebnis, float *Koeffizienten, float *Inhomogenitaet)
{
	// Locals
	unsigned int a, b;
	float	 *result, *coeff, *inhomo;

	// Parameter gültig ?
	if (Ergebnis == NULL || Koeffizienten == NULL || Inhomogenitaet == NULL)
		return FALSE;

	// Wenn unterbestimmt, dann Fülle mit Gleichungen auf
	if (AnzGleichungen < AnzUnbekannte)
	{
		// Lege neue größere Arrays an
		result = Ergebnis;
		coeff  = new float[AnzUnbekannte * AnzUnbekannte];
		inhomo = new float[AnzUnbekannte];

		// Nullen & Kopiere Daten
		for (a = 0; a < AnzUnbekannte; a++)
		{
			if (a < AnzGleichungen) inhomo[a] = Inhomogenitaet[a];
			else					inhomo[a] = 0.0f;
			for (b = 0; b < AnzUnbekannte; b++)
			{
				if (a < AnzGleichungen) coeff[a * AnzUnbekannte + b] = Koeffizienten[b * AnzGleichungen + a];
				else					coeff[a * AnzUnbekannte + b] = 0.0f;
			}
		}
		AnzGleichungen = AnzUnbekannte;
	}
	// Wenn überbestimmt, dann ...
	else if (AnzUnbekannte < AnzGleichungen)
	{
		return FALSE;
	}
	// Alles passt, dann kann array übernommen werden
	else
	{
		// Lege neue Arrays an
		result = Ergebnis;
		coeff  = new float[AnzUnbekannte * AnzUnbekannte];
		inhomo = new float[AnzUnbekannte];

		// Kopiere Daten
		for (a = 0; a < AnzUnbekannte; a++)
		{
			inhomo[a] = Inhomogenitaet[a];
			
			for (b = 0; b < AnzUnbekannte; b++)
				coeff[a * AnzUnbekannte + b] = Koeffizienten[b * AnzGleichungen + a];
		}
	}
	
	// Wende Gauss-Algorithmus an
	LoeseLGSmitGauss(AnzGleichungen, result, coeff, inhomo);

	// Gebe Speicher wieder frei
	delete [] coeff;
	delete [] inhomo;

	return TRUE;
}

//-----------------------------------------------------------------------------
// Name: ggT()
// Desc: Größter Gemeinsamer Teiler
//-----------------------------------------------------------------------------
unsigned int ggT(unsigned int a, unsigned int b)
{
	unsigned int r;

	do 
	{
		r = a % b;
		a = b;
		b = r;
	} while (b != 0);
	
	return a;
}

//-----------------------------------------------------------------------------
// Name: alg()
// Desc: Algorithmus
//-----------------------------------------------------------------------------
unsigned int alg(unsigned int a, unsigned int b)
{
	unsigned int dwAnzahlSchritte = 0;									// Anzahl der benötigten Schritte
	unsigned int c1x = 2, c1y = 0, c2x = 3, c2y = 0, c3x = 1, c3y = 0;	// Konstanten
	unsigned int zx = a, zy = b;										// Startwerte
	unsigned int tx, ty, tt = c1x * c1x + c1y + c1y;					// Hilfsvariablen

	// teilbar durch c1
	do 
	{
		tx = zx * c1x + zy * c1y;
		ty = c1x * zy - zx * c1y;

		if ((tx % tt == 0) && (ty % tt == 0))
		{
			zx = tx / tt;
			zy = ty / tt;
		}
		// sonst z mit c2 multiplizieren und c3 addieren
		else 
		{
			zx = zx * c2x - zy * c2y + c3x;
			zy = zx * c2y + c2x * zy + c3y;
		}
		dwAnzahlSchritte++;
	}
	while ((zx != 1) && (zy != 1) && (dwAnzahlSchritte < 1000));
	
	return dwAnzahlSchritte;
}

//-----------------------------------------------------------------------------
// Name: exchangeVariables()
// Desc: a = b; && b = a;
//-----------------------------------------------------------------------------
void exchangeVariables(float *a, float *b)
{
	float t = *a;
	*a = *b;
	*b = t;
}

//-----------------------------------------------------------------------------
// Name: integrate()
// Desc: 
//-----------------------------------------------------------------------------
float integrate(float f(float x), float von, float bis, unsigned int steps)
{
	// von < bis ?
	if (bis > von) exchangeVariables(&von, &bis);

	// locals
	float dx		= (bis - von) / steps;
	float result	= 0.0f;
	float x			= von;

	for (unsigned int a = 0; a < steps; a++)
	{
		result += f(x) * dx;
		x	   += dx;
	}

	return result;
}

//-----------------------------------------------------------------------------
// Name: fourierTransformation()
// Desc: 
//-----------------------------------------------------------------------------
float fourierTransformation(float f(float x), float k, float von, float bis, unsigned int steps)
{
	// von < bis ?
	if (bis > von) exchangeVariables(&von, &bis);

	// locals
	float dx		= (bis - von) / steps;
	float result	= 0.0f;
	float x			= von;

	for (unsigned int a = 0; a < steps; a++)
	{
		result += f(x) * (float)sin(-k * x) * dx;
		x	   += dx;
	}

	return result;
}

//-----------------------------------------------------------------------------
// Name: fourierTransformation()
// Desc: 
//-----------------------------------------------------------------------------
float fourierTransformation(unsigned int numInputValues, float *input, float k, float dx)
{
	// locals
	float result	= 0.0f;
	float x			= 0.0f;

	for (unsigned int a = 0; a < numInputValues; a++)
	{
		result += input[a] * (float)sin(-k * x) * dx;
		x	   += dx;
	}

	return result;
}

//-----------------------------------------------------------------------------
// Name: Differentiate()
// Desc: 
//-----------------------------------------------------------------------------
bool differentiate(unsigned int numColumns, float *function, float xmin, float xmax, float *differentiatedFunction)
{
	// Locals & Errors
	float *f	= function;
	float *d	= differentiatedFunction;
	float xStep = (xmax - xmin) / (numColumns - 1);
	if (xmax <= xmin) 	return false;
	if (f == NULL) 		return false;
	if (d == NULL) 		return false;
	if (numColumns < 2)	return false;

	// Differentiate
	d[0] 			= 	(f[1]            - f[0]           ) / xStep;
	d[numColumns-1]	=	(f[numColumns-1] - f[numColumns-2]) / xStep;

	for (unsigned int n=1; n < numColumns - 1; n++) d[n] = (f[n+1] - f[n-1]) / xStep / 2.0f;

	return true;
}

//-----------------------------------------------------------------------------
// Name: Differentiate()
// Desc: 
//-----------------------------------------------------------------------------
bool differentiate(unsigned int numColumns, double *function, double xmin, double xmax, double *differentiatedFunction)
{
	// Locals & Errors
	double *f	= function;
	double *d	= differentiatedFunction;
	double xStep = (xmax - xmin) / (numColumns - 1);
	if (xmax <= xmin) 	return false;
	if (f == NULL) 		return false;
	if (d == NULL) 		return false;
	if (numColumns < 2)	return false;

	// Differentiate
	d[0] 			= 	(f[1]            - f[0]           ) / xStep;
	d[numColumns-1]	=	(f[numColumns-1] - f[numColumns-2]) / xStep;

	for (unsigned int n=1; n < numColumns - 1; n++) d[n] = (f[n+1] - f[n-1]) / xStep / 2.0f;

	return true;
}

//-----------------------------------------------------------------------------
// Name: Differentiate()
// Desc: Creates a new array and returns it!
//-----------------------------------------------------------------------------
float *differentiate(unsigned int numColumns, float *function, float xmin, float xmax)
{
	// Locals & Errors
	float *f	= function;
	float xStep = (xmax - xmin) / (numColumns - 1);
	if (xmax <= xmin) 	return NULL;
	if (f == NULL) 		return NULL;
	if (numColumns < 2)	return NULL;
	float *d	= new float[numColumns];

	// Differentiate
	d[0] 			= 	(f[1]            - f[0]           ) / xStep;
	d[numColumns-1]	=	(f[numColumns-1] - f[numColumns-2]) / xStep;

	for (unsigned int n=1; n < numColumns - 1; n++) d[n] = (f[n+1] - f[n-1]) / 2 / xStep;

	return d;
}

//-----------------------------------------------------------------------------
// Name: makeGraphInPlainXY()
// Desc: if (data == NULL)	y = 0.0f;
//		 else				y = data;
//-----------------------------------------------------------------------------
void makeGraphInPlainXY(unsigned int numColumns, float xmin, float xmax, GR3D_POINT_VTX *vector, DWORD color, float *data)
{
	float xStep		= (xmax - xmin) / numColumns;
	float xPos		= xmin + xStep  / 2.0f;
	unsigned int i;
		
	for (unsigned int i= 0; i < numColumns; i++)
	{
		vector[i].x		= xPos;
		vector[i].z		= 0.0f;
		vector[i].Dif	= color;
		xPos		   += xStep;
	}

	if (data != NULL) for (i= 0; i < numColumns; i++)	vector[i].y		= data[i];
	else			  for (i= 0; i < numColumns; i++)	vector[i].y		= 0.0f;
}

//-----------------------------------------------------------------------------
// Name: normalize()
// Desc: 
//-----------------------------------------------------------------------------
void normalize(unsigned int columns, float xmin, float xmax, float value, float *data)
{
	// Locals
	unsigned int i;
	float		 integral	= 0.0f;
	float		 xStep		= (xmax - xmin) / columns;

	for (i = 0; i < columns; i++) integral += data[i]  * xStep;
	for (i = 0; i < columns; i++) data[i]  /= integral / value; 
}

//-----------------------------------------------------------------------------
// Name: eulerIteration()
// Desc: For the equation: dx/dt = f(x)
//-----------------------------------------------------------------------------
float eulerIteration(float f(float x), float x, float dt)
{
	return f(x) * dt + x;
}

//-----------------------------------------------------------------------------
// Name: normalize()
// Desc: 
//-----------------------------------------------------------------------------
float improvedEulerIteration(float f(float x), float x, float dt)
{
	float	fx = f(x);
	float	x_ = x + fx * dt;
	return	0.5f * (fx + f(x_)) * dt + x;
}

//-----------------------------------------------------------------------------
// Name: rungeKuttaIteration()
// Desc: 
//-----------------------------------------------------------------------------
float rungeKuttaIteration(float f(float x), float x, float dt)
{
	float k1 = dt * f(x);
	float k2 = dt * f(x + 0.5f * k1);
	float k3 = dt * f(x + 0.5f * k2);
	float k4 = dt * f(x +        k3);
	return x + 1 / 6.0f * (k1 + k2 + k3 + k4);
}

//-----------------------------------------------------------------------------
// Name: returnMax()
// Desc: returns the biggest value
//-----------------------------------------------------------------------------
float returnMax(float a, float b, float c)
{
	if (a > b && a > c) return a;
	else if     (b > c)	return b;
	else			    return c;
}

//-----------------------------------------------------------------------------
// Name: getAngleInPolarCoord()
// Desc: returns phi
//-----------------------------------------------------------------------------
double getAngleInPolarCoord(double x, double y)
{
	double angle = atan(y / x);

		 if (x < 0 && y > 0) angle =     PI + angle;
	else if (x > 0 && y < 0) angle = 2 * PI + angle;
	else if (x < 0 && y < 0) angle =     PI + angle;

	return angle;
}

//-----------------------------------------------------------------------------
// Name: integrateWithEuler()
// Desc: orbit must be of size "dim * numSteps"
//-----------------------------------------------------------------------------
void integrateWithEuler(double *orbit, unsigned int dim, double func(double *x, double t, unsigned int curDim), double *initValue, double stepSize, unsigned int numSteps)
{
  /*** Euler Method *************
  x(t + dt) = x(t) + f(x) * dt
  *******************************/

  // Locals
  unsigned int n, i, di;
  double       *curOrbit;
  double       t = 0;

  // initial condition
  for (n=0; n<dim; n++) orbit[n] = initValue[n];

  // Integrate
  for (i=1; i<numSteps; i++) 
  {
    // pointer to current Orbit
    curOrbit = &orbit[(i-1) * dim];

    // set next value in every dimension
    for (n=0, di=(i-1)*dim; n<dim; n++,di++) orbit[di + dim] = orbit[di] + func(curOrbit, t, n) * stepSize;

    // current time
    t += stepSize;
  } 
}

//-----------------------------------------------------------------------------
// Name: integrateWithRungeKutta()
// Desc: orbit must be of size "dim * numSteps"
//-----------------------------------------------------------------------------
void integrateWithRungeKutta(double *orbit, unsigned int dim, double func(double *x, double t, unsigned int curDim), double *initValue, double stepSize, unsigned int numSteps)
{
  /*** Euler Method *************
  k1 = f(x(t)       ) * dt
  k2 = f(x(t) + k1/2) * dt
  k3 = f(x(t) + k2/2) * dt
  k4 = f(x(t) + k3  ) * dt
  x(t + dt) = x(t) + (k1 + 2*k2 + 2*k3 + k4) / 6
  *******************************/

  // Locals
  unsigned int n, i, di, si;
  double       t = 0;

  // Make array for result & k
  double *k1        = new double[dim];
  double *k2        = new double[dim];
  double *k3        = new double[dim];
  double *k4        = new double[dim];
  double *curOrbit  = new double[dim];

  // initial condition
  for (n=0; n<dim; n++) orbit[n] = initValue[n];

  // Integrate
  for (i=1; i<numSteps; i++) 
  {
    // static index, it is used several times
    si = (i-1) * dim;

    // calc k1
    for (n=0; n<dim; n++) curOrbit[n] = orbit[si + n];
    for (n=0; n<dim; n++) k1[n]       = func(curOrbit, t, n) * stepSize;

    // calc k2
    for (n=0; n<dim; n++) curOrbit[n] = orbit[si + n] + k1[n] / 2;
    for (n=0; n<dim; n++) k2[n]       = func(curOrbit, t, n) * stepSize;

    // calc k3
    for (n=0; n<dim; n++) curOrbit[n] = orbit[si + n] + k2[n] / 2;
    for (n=0; n<dim; n++) k3[n]       = func(curOrbit, t, n) * stepSize;

    // calc k4
    for (n=0; n<dim; n++) curOrbit[n] = orbit[si + n] + k3[n];
    for (n=0; n<dim; n++) k4[n]       = func(curOrbit, t, n) * stepSize;

    // set next value in every dimension
    for (n=0, di=si; n<dim; n++,di++) orbit[di + dim] = orbit[di] + (k1[n] + 2*k2[n] + 2*k3[n] + k4[n]) / 6;

    // current time
    t += stepSize;
  } 
}

//-----------------------------------------------------------------------------
// Name: interpolateValues()
// Desc: Returns interpolated scalar values at desired positions for a given 2D scalar field.
//-----------------------------------------------------------------------------
void interpolateValues(unsigned int numPointsToInterpolate,         // in:  number of values to interpolate
                       double xSource[],                            // in:  query positions
                       double ySource[],                            // in: 
                       double *interpolatedValues,                  // out: values of interpolated points
                       unsigned int numTargetPoints,                // in: number of points describing the given 2D scalar field
                       double xTarget[],                            // in: positions of the given points
                       double yTarget[],                            // in: 
                       double targetValues[],                       // in: values of the given points
                       bool biCubic)                                // in: use bicubic interpolation - default is bilinear
{
    // n -> x
    // m -> y

    // locals
    unsigned int        curPoint, m, n, curM, curN;
    unsigned int        *indexMap           = new unsigned int[numTargetPoints];
    double              *xTargetSorted      = new double[numTargetPoints];
    double              *yTargetSorted      = new double[numTargetPoints];
    double              lastValueX, lastValueY;
    real_1d_array       xPos, yPos;
    real_2d_array       values;
    spline2dinterpolant coeffTable;

    double              delta = 0.00001;    // mm

    // sort target points coordinates
    memcpy(xTargetSorted, xTarget, sizeof(double) * numTargetPoints);
    memcpy(yTargetSorted, yTarget, sizeof(double) * numTargetPoints);
    qsort(xTargetSorted, numTargetPoints, sizeof(double), compareDouble);
    qsort(yTargetSorted, numTargetPoints, sizeof(double), compareDouble);

    // calc m and n
    for (curPoint=1, m=1, n=1, lastValueX=xTargetSorted[0], lastValueY=yTargetSorted[0]; curPoint<numTargetPoints; curPoint++) {

        // next column achieved?
        if (lastValueX + delta < xTargetSorted[curPoint]) {
            lastValueX = xTargetSorted[curPoint];
            n++;
        }

        // next row achieved?
        if (lastValueY + delta < yTargetSorted[curPoint]) {
            lastValueY = yTargetSorted[curPoint];
            m++;
        }
    }

    // check
    if (m * n != numTargetPoints) {
        cout << "ERROR!           m * n != numTargetPoints" << endl;
        return;
    }

    // calc values, xPos, yPos
    values.setlength(m-1, n-1);
    xPos.setlength(n-1);
    yPos.setlength(m-1);

    for (curN=0; curN<n; curN++) xPos(curN) = xTargetSorted[m*curN];
    for (curM=0; curM<m; curM++) yPos(curM) = yTargetSorted[n*curM];

    // calc indexMap
    for (curPoint=0; curPoint<numTargetPoints; curPoint++) {

        // goto and find to corresponding row- and column-index
        for (curN=0; curN<n; curN++) { if (xTarget[curPoint] >= xPos(curN) - delta && xTarget[curPoint] <= xPos(curN) + delta) break; }
        for (curM=0; curM<m; curM++) { if (yTarget[curPoint] >= yPos(curM) - delta && yTarget[curPoint] <= yPos(curM) + delta) break; }

        if (curN>=n||curM>=m) {
            cout << "ERROR!           curN>=n||curM>=m" << endl;
            return;
        }

        indexMap[curM*n+curN] = curPoint;
    }

    // copy data using indexMap
    for (curPoint=0; curPoint<numTargetPoints; curPoint++) {
        curM               = curPoint / n;
        curN               = curPoint % n;
        values(curM, curN) = targetValues[indexMap[curM*n+curN]];
    }

    // make bilinear interpolation
    if (!biCubic) {
        spline2dbuildbilinear(xPos, yPos, values, m, n, coeffTable);
    // make bicubic interpolation
    } else {
        spline2dbuildbicubic(xPos, yPos, values, m, n, coeffTable);
    }

    // calc interpolated values
    for (curPoint=0; curPoint<numPointsToInterpolate; curPoint++) {
        interpolatedValues[curPoint] = spline2dcalc(coeffTable, xSource[curPoint], ySource[curPoint]);
    }
    
    // free mem
    delete [] xTargetSorted, yTargetSorted, indexMap;
}

//-----------------------------------------------------------------------------
// Name: extrapolateValues()
// Desc: Fits a function to given data points and calculates the values at desired positions.
//-----------------------------------------------------------------------------
void extrapolateValues(unsigned int numDataPoints,          // in: number of given datapoints
                       double *xData,                       // in: x-coordinates of datapoints
                       double *yData,                       // in: y-coordinates of datapoints
                       unsigned int numTargetPoints,        // in: number of queried points
                       double *xTarget,                     // in: x-coordinates of queried points
                       double *yTarget,                     // out: extrapolated value
                       unsigned int numFitParameters,       // in: number of parameters to fit
                       double *fitParameter,                // in/out: initial values and fitted parameters
                       ptFunc fitFunction,                  // in: function to fit
                       ptFunc* gradient)                    // in: gradients of function, one for each parameter to fit
{
    // locals
    unsigned int curTargetPoint, curFitParameter;
    double *param = new double[1 + numFitParameters];

    // make fit
    fitToNonLinearFunction(numFitParameters, fitParameter, fitFunction, gradient, xData, yData, numDataPoints);

    // calc extrapolated values
    for (curTargetPoint=0; curTargetPoint<numTargetPoints; curTargetPoint++) {
        param[0] = xTarget[curTargetPoint];
        for (curFitParameter=0; curFitParameter<numFitParameters; curFitParameter++) {
            param[1+curFitParameter] = fitParameter[curFitParameter];
        }
        yTarget[curTargetPoint] = fitFunction(param);
    }
}

//-----------------------------------------------------------------------------
// Name: compareDouble()
// Desc: Shall return: 
//       - positive value when a is greater than b
//       - negative value when b is greater than a
//       - zero when both are equal
//-----------------------------------------------------------------------------
int compareDouble(const void * a, const void * b)
{
    return (((*((double*)a)) > (*((double*)b))) ? 1 : -1);
}

//-----------------------------------------------------------------------------
// Name: integrate()
// Desc: Performs a simple integration of the function f(x).
//-----------------------------------------------------------------------------
double integrate(double f(double x), double lowerLimit, double upperLimit, double stepSize)
{
    // locals
    double sum = 0, x;

    for (x=lowerLimit; x<=upperLimit; x += stepSize) {
        sum += f(x) * stepSize;
    }

    return sum;
}

//-----------------------------------------------------------------------------
// Name: calcFWHM()
// Desc: Calculates the full width at half maximum of the function f(x).
//-----------------------------------------------------------------------------
double calcFWHM(double f(double x), double xMin, double xMax, double x0, double yDelta)
{
    // locals
    double x;                                       // current x-value
    double y0       = f(x0);                        // maximum y-value
    double dx       = 0.1f * (xMax - xMin);         // increment 
    double xLeft    = xMin;                         // left  position where f(left ) = y0/2
    double xRight   = xMax;                         // right position where f(right) = y0/2
    double yLeft    = f(xLeft);
    double yRight   = f(xRight);

    // leave if y-value is already greater than supposed half maximum y-value
    if (yLeft  > 0.5*y0) return 0;
    if (yRight > 0.5*y0) return 0;

    // search left and right
    do {

        // search left
        for (x=xLeft; x<x0; x+=dx) {

            // calc current y-value
            yLeft = f(x);

            if (yLeft > 0.5*y0) {
                yLeft = f(xLeft);
                break;
            } else {
                xLeft = x;
            }
        }

        // search right
        for (x=xRight; x>x0; x-=dx) {

            // calc current y-value
            yRight = f(x);

            if (yRight > 0.5*y0) {
                yRight = f(xRight);
                break;
            } else {
                xRight = x;
            }
        }

        // refine step size by one order of magnitude
        dx *= 0.1f;

    } while (yLeft  < 0.5*y0 - yDelta || yLeft  > 0.5*y0 + yDelta
          || yRight < 0.5*y0 - yDelta || yRight > 0.5*y0 + yDelta);

    return xRight - xLeft;
}

//-----------------------------------------------------------------------------
// Name: invertMonotonFunction()
// Desc: Return the x value of the equation y = f(x) for monoton functions.
//-----------------------------------------------------------------------------
double invertMonotonFunction(double f(double x, void* userPointer), 
                             double y, 
							 void* userPointer,
                             double xMin, 
                             double xMax, 
                             double deltaPrecision, 
                             bool xLog, 
                             bool yLog)
{
    // locals
    unsigned int numSteps = 10;
    double       stepSize, xCur, curError, yCur, yMin, yMax;
    bool         tooBig;

    // adjust parameters
    if (xMin>xMax) {
        xCur = xMin;
        xMin = xMax;
        xMax = xCur;
    }

    // calc min/max-values
    yMin = f(xMin, userPointer);
    yMax = f(xMax, userPointer);

    // return minimum respectively maximum value if both are smaller or bigger than y
    if (yMin < y && yMax < y) return (yMin>yMax) ? xMin : xMax;
    if (yMin > y && yMax > y) return (yMin<yMax) ? xMin : xMax;

    if (yMin > y) { 
        tooBig = true;
    } else {
        tooBig = false;
    }

    // transform x-parameters when f(x) is exponetial-like
    xMin = xLog ? log(xMin) : xMin;
    xMax = xLog ? log(xMax) : xMax;

    do {
        stepSize = (xMax - xMin) / numSteps;

        for (xCur=xMin+stepSize; xCur <= xMax; xCur += stepSize) {
            
            // calculate current function value f(x) and deviation from target value y
            yCur        = xLog ? f(exp(xCur), userPointer)  : f(xCur, userPointer);
            curError    = yLog ? abs(log(y) - log(yCur))    : abs(y - yCur);
            
            // desired precision reached?
            if (curError < deltaPrecision) break;

            // leaave loop if sign of difference has changed
            if (yCur > y) {
                if (!tooBig) break;
            } else {
                if ( tooBig) break;
            }
        }

        // calculate new beginning and ending positions
        xMin        = xCur - stepSize;
        xMax        = xCur;

    } while (curError > deltaPrecision);

    return xLog ? exp(xCur) : xCur;
}

//-----------------------------------------------------------------------------
// Name: fitToNonLinearFunction()
// Desc: Fits a nonlinear function.
//-----------------------------------------------------------------------------
void fitToNonLinearFunction(unsigned int numParameters,         // in: number of parameters to fit
                            double *fitParameter,               // in/out: parameters to fit - initial values and fitted values
                            ptFunc function,                    // in: function to fit
                            ptFunc* gradient,                   // in: gradients of function, one for each parameter to fit
                            double xData[],                     // in: x-coordinates of data points
                            double yData[],                     // in: y-coordinates of data points
                            unsigned int numDataPoints)         // in: number of given datapoints
{
    // locals
    int                 numDimensions       = 1;
    int                 numParamsBeingFit   = numParameters;
    real_1d_array       yPoints;
    real_2d_array       xPoints;
    real_1d_array       cParam;
    lsfitreport         rep;
    lsfitstate          state;
    ae_int_t            info;
    double              epsf;
    double              epsx;
    int                 maxits;
    int                 i;
    double              *param   = new double[numParameters+2];     // +2 for x and f
    
    // Prepare task matrix
    yPoints.setlength(numDataPoints);
    xPoints.setlength(numDataPoints, numDimensions);
    cParam.setlength(numParamsBeingFit);
    for(i=0; i<(int)numDataPoints; i++) {
        xPoints(i,0)  = xData[i];
        yPoints(i)    = yData[i];
    }

    // initial value
    for (i=0; i<numParamsBeingFit; i++) {
        cParam(i)    = fitParameter[i];
    }

    // stopping conditions
    epsf    = 0.0;
    epsx    = 0.0001;
    maxits  = 0;
    
    // Solve
    lsfitcreatefg(xPoints, yPoints, cParam, numDataPoints, numDimensions, numParamsBeingFit, true, state);
    lsfitsetcond(state, epsf, epsx, maxits);
/*
    lsfitfit(state, function_cx_1_func, function_cx_1_grad);

    void function_cx_1_func(const real_1d_array &c, const real_1d_array &x, double &func, void *ptr) 
    {
        // this callback calculates f(c,x)=exp(-c0*sqr(x0))
        // where x is a position on X-axis and c is adjustable parameter
        func = exp(-c[0]*pow(x[0],2));
    }
    void function_cx_1_grad(const real_1d_array &c, const real_1d_array &x, double &func, real_1d_array &grad, void *ptr) 
    {
        // this callback calculates f(c,x)=exp(-c0*sqr(x0)) and gradient G={df/dc[i]}
        // where x is a position on X-axis and c is adjustable parameter.
        // IMPORTANT: gradient is calculated with respect to C, not to X
        func = exp(-c[0]*pow(x[0],2));
        grad[0] = -pow(x[0],2)*func;
    }

    while(lsfitnonlineariteration(state))
    { 
        // copy parameters
        param[0] = state.x(0);
        for (i=0; i<numParamsBeingFit; i++) {
            param[i+1] = state.c(i);
        }

        // function value F(x)
        if( state.needf ) {

            state.f = function(param);

        // function value F(x) and gradient dF/dc
        } else if( state.needfg ) {
            
            param[numParamsBeingFit+1]  = state.f;
            state.f                     = function(param); 
            for (i=0; i<numParamsBeingFit; i++) {
                state.g(i)  = gradient[i](param);
            }
        }
    }
*/
    // extract results
    lsfitresults(state, info, cParam, rep);
    
    for (i=0; i<numParamsBeingFit; i++) {
        fitParameter[i] = cParam(i);
        printf("%d:       %0.3lf\n", i+1, double(cParam(i)));
    }
    printf("rms.err: %0.3lf\n",         double(rep.rmserror));
    printf("max.err: %0.3lf\n",         double(rep.maxerror));
    printf("Termination type: %0ld\n",  long(info));
    printf("\n\n");

    // free mem
    delete [] param;
}

//-----------------------------------------------------------------------------
// Name: performPeakAnalysis()
// Desc: Fits a gaussian or pseudo voigt function to a given set of data points.
//-----------------------------------------------------------------------------
void performPeakAnalysis(table<double> * &peakTable,        // out: table containing the found peaks
                         double xData[],                    // in: x-coordinates of data points
                         double yData[],                    // in: y-coordinates of data points
                         unsigned int numDataPoints,        // in: number of given datapoints
                         double minDelta,                   // in: 
                         unsigned functionType)             // in: MATFUNC_FUNCTION_GAUSS or MATFUNC_FUNCTION_PSEUDE_VOIGT
{
    // locals
    unsigned int    curPeak, numPeaks, numParameters;
    unsigned int    *numDataPointsOfCurPeak, *startingIndexOfCurPeak, *peakPosition;

    // count peaks
    findCentersBetweenPeaks(xData, yData, numDataPoints, numPeaks, startingIndexOfCurPeak, peakPosition, numDataPointsOfCurPeak, minDelta);

    // column names
    switch (functionType) 
    {
    case MATFUNC_FUNCTION_GAUSS:
        numParameters = 3;
        peakTable = new table<double>(10, numPeaks);
        peakTable->setColumnName(0, "xFrom");       // begin to fit from this point
        peakTable->setColumnName(1, "xPeak");       // position of peak
        peakTable->setColumnName(2, "xTo");         // fit until this point
        peakTable->setColumnName(3, "y0");          // maximum of peak
        peakTable->setColumnName(4, "x0");          // center of peak
        peakTable->setColumnName(5, "s");           // standard deviation
        peakTable->setColumnName(6, "w");           // full width at half maximum
        peakTable->setColumnName(7, "b");           // integral width
        peakTable->setColumnName(8, "w_num");       // numerically calculated full width at half maximum
        peakTable->setColumnName(9, "b_num");       // numerically calculated integral width
        break;
    case MATFUNC_FUNCTION_PSEUDE_VOIGT:
        numParameters = 5;
        peakTable = new table<double>(14, numPeaks);
        peakTable->setColumnName(0,  "xFrom");       // begin to fit from this point
        peakTable->setColumnName(1,  "xPeak");       // position of peak
        peakTable->setColumnName(2,  "xTo");         // fit until this point
        peakTable->setColumnName(3,  "y0");          // maximum of peak
        peakTable->setColumnName(4,  "x0");          // center of peak
        peakTable->setColumnName(5,  "n");           // weight parameter
        peakTable->setColumnName(6,  "wl");          // full width at half maximum
        peakTable->setColumnName(7,  "wg");
        peakTable->setColumnName(8,  "wv");
        peakTable->setColumnName(9,  "bl");          // integral width
        peakTable->setColumnName(10, "bg");
        peakTable->setColumnName(11, "bv");
        peakTable->setColumnName(12, "wv_num");       // numerically calculated full width at half maximum
        peakTable->setColumnName(13, "bv_num");       // numerically calculated integral width
        break;
    }

    // allocate mem
    ptFunc *gradient= new ptFunc[numParameters];
    double *param   = new double[numParameters];

    for (curPeak=0; curPeak<numPeaks; curPeak++) {

        // initial values for fit
        peakTable->data[peakTable->getColumnIndex("xFrom")*peakTable->numRows+curPeak] = xData[startingIndexOfCurPeak[curPeak]];
        peakTable->data[peakTable->getColumnIndex("xPeak")*peakTable->numRows+curPeak] = xData[peakPosition[curPeak]];
        peakTable->data[peakTable->getColumnIndex("xTo")  *peakTable->numRows+curPeak] = xData[startingIndexOfCurPeak[curPeak] + numDataPointsOfCurPeak[curPeak]];
        peakTable->data[peakTable->getColumnIndex("y0")   *peakTable->numRows+curPeak] = yData[peakPosition[curPeak]];
        peakTable->data[peakTable->getColumnIndex("x0")   *peakTable->numRows+curPeak] = xData[peakPosition[curPeak]];

        /* zero noise
        for (curDataPoint=startingIndexOfCurPeak[curPeak]; curDataPoint < startingIndexOfCurPeak[curPeak] + numDataPointsOfCurPeak[curPeak]; curDataPoint++) {
            if (yData[curDataPoint] < 0.10 * yData[peakPosition[curPeak]]) {
                yData[curDataPoint] = 0;
            }
        }*/

        switch (functionType) 
        {
        case MATFUNC_FUNCTION_GAUSS:

            // gauss function specific initial values for fit
            peakTable->data[peakTable->getColumnIndex("s")    *peakTable->numRows+curPeak] = 0.2;

            // calc parameters of fitted gauss function for current peak
            gradient[0]     = gradientOfGaussFunction_y0;
            gradient[1]     = gradientOfGaussFunction_x0;
            gradient[2]     = gradientOfGaussFunction_w;
            param[0]        = peakTable->data[peakTable->getColumnIndex("y0")*peakTable->numRows+curPeak];
            param[1]        = peakTable->data[peakTable->getColumnIndex("x0")*peakTable->numRows+curPeak];
            param[2]        = peakTable->data[peakTable->getColumnIndex("w")*peakTable->numRows+curPeak];
            
            fitToNonLinearFunction(numParameters, param, gaussFunction, gradient, 
                &xData[startingIndexOfCurPeak[curPeak]], &yData[startingIndexOfCurPeak[curPeak]], numDataPointsOfCurPeak[curPeak]);

            peakTable->data[peakTable->getColumnIndex("y0")*peakTable->numRows+curPeak]  = param[0];
            peakTable->data[peakTable->getColumnIndex("x0")*peakTable->numRows+curPeak]  = param[1];
            peakTable->data[peakTable->getColumnIndex("s")*peakTable->numRows+curPeak]   = param[2];

            // calc additional values
            peakTable->data[peakTable->getColumnIndex("w")*peakTable->numRows+curPeak] = 2*sqrt(2*log(2.0)) * abs(peakTable->data[peakTable->getColumnIndex("s")*peakTable->numRows+curPeak]);
            peakTable->data[peakTable->getColumnIndex("b")*peakTable->numRows+curPeak] = sqrt(c_pi/4.0/log(2.0)) *  peakTable->data[peakTable->getColumnIndex("w")*peakTable->numRows+curPeak];
            break;

        case MATFUNC_FUNCTION_PSEUDE_VOIGT:
            
            // pseudo-voigt function specific initial values for fit
            peakTable->data[peakTable->getColumnIndex("n")     *peakTable->numRows+curPeak] = 0.5;
            peakTable->data[peakTable->getColumnIndex("wl")    *peakTable->numRows+curPeak] = 0.2;
            peakTable->data[peakTable->getColumnIndex("wg")    *peakTable->numRows+curPeak] = 0.2;

            gradient[0]     = gradientOfPseudoVoigtFunction_y0;
            gradient[1]     = gradientOfPseudoVoigtFunction_x0;
            gradient[2]     = gradientOfPseudoVoigtFunction_wl;
            gradient[3]     = gradientOfPseudoVoigtFunction_wg;
            gradient[4]     = gradientOfPseudoVoigtFunction_n;
            param[0]        = peakTable->data[peakTable->getColumnIndex("y0")*peakTable->numRows+curPeak];
            param[1]        = peakTable->data[peakTable->getColumnIndex("x0")*peakTable->numRows+curPeak];
            param[2]        = peakTable->data[peakTable->getColumnIndex("wl")*peakTable->numRows+curPeak];
            param[3]        = peakTable->data[peakTable->getColumnIndex("wg")*peakTable->numRows+curPeak];
            param[4]        = peakTable->data[peakTable->getColumnIndex("n" )*peakTable->numRows+curPeak];

            fitToNonLinearFunction(numParameters, param, pseudoVoigtFunction, gradient, 
                &xData[startingIndexOfCurPeak[curPeak]], &yData[startingIndexOfCurPeak[curPeak]], numDataPointsOfCurPeak[curPeak]);

            peakTable->data[peakTable->getColumnIndex("y0")*peakTable->numRows+curPeak]  = abs(param[0]);
            peakTable->data[peakTable->getColumnIndex("x0")*peakTable->numRows+curPeak]  = abs(param[1]);
            peakTable->data[peakTable->getColumnIndex("wl")*peakTable->numRows+curPeak]  = abs(param[2]);
            peakTable->data[peakTable->getColumnIndex("wg")*peakTable->numRows+curPeak]  = abs(param[3]);
            peakTable->data[peakTable->getColumnIndex("n" )*peakTable->numRows+curPeak]  = abs(param[4]);

            // calc FWHM
            peakTable->data[peakTable->getColumnIndex("wv")*peakTable->numRows+curPeak] = 0;
            peakTable->data[peakTable->getColumnIndex("bl")*peakTable->numRows+curPeak] = peakTable->data[peakTable->getColumnIndex("wl") *peakTable->numRows+curPeak] * c_pi / 2.0;
            peakTable->data[peakTable->getColumnIndex("bg")*peakTable->numRows+curPeak] = sqrt(c_pi/4.0/log(2.0)) *  peakTable->data[peakTable->getColumnIndex("wg")*peakTable->numRows+curPeak];
            peakTable->data[peakTable->getColumnIndex("bv")*peakTable->numRows+curPeak] = peakTable->data[peakTable->getColumnIndex("n") *peakTable->numRows+curPeak] * peakTable->data[peakTable->getColumnIndex("bl") *peakTable->numRows+curPeak] + (1.0 - peakTable->data[peakTable->getColumnIndex("n") *peakTable->numRows+curPeak]) * peakTable->data[peakTable->getColumnIndex("bg") *peakTable->numRows+curPeak];

            break;
        }
    }

    // calculate numerically the integral width and FWHM
    for (curPeak=0; curPeak<numPeaks; curPeak++) {
        peakTable->data[peakTable->getColumnIndex("wv_num")*peakTable->numRows+curPeak] = calcFWHM            (&xData[startingIndexOfCurPeak[curPeak]], &yData[startingIndexOfCurPeak[curPeak]], numDataPointsOfCurPeak[curPeak]);
        peakTable->data[peakTable->getColumnIndex("bv_num")*peakTable->numRows+curPeak] = calcIntegralWidth   (&xData[startingIndexOfCurPeak[curPeak]], &yData[startingIndexOfCurPeak[curPeak]], numDataPointsOfCurPeak[curPeak]);
    }

    // free mem
    delete [] param;
    delete [] gradient;
}

//-----------------------------------------------------------------------------
// Name: calcFWHM()
// Desc: 
//-----------------------------------------------------------------------------
double calcFWHM(double xData[], double yData[], unsigned int numDataPoints)
{
    // locals
    double          inclination;
    double          max             = 0;
    unsigned int    curDataPoint;
    double          right           = 0;
    double          left            = 0;
   
    // calc maximum
    for (curDataPoint=0; curDataPoint<numDataPoints; curDataPoint++) {
        if (max < yData[curDataPoint]) max = yData[curDataPoint];
    }

    // calc left
    for (curDataPoint=1; curDataPoint<numDataPoints; curDataPoint++) {
        if (yData[curDataPoint] > 0.5 * max) {
            inclination = (yData[curDataPoint] - yData[curDataPoint-1]) / (xData[curDataPoint] - xData[curDataPoint-1]);
            left        = (0.5*max - yData[curDataPoint-1] + inclination*xData[curDataPoint-1]) / inclination;
            break;
        }
    }

    // calc right
    for (curDataPoint=numDataPoints-2; curDataPoint>0; curDataPoint--) {
        if (yData[curDataPoint] > 0.5 * max) {
            inclination = (yData[curDataPoint] - yData[curDataPoint+1]) / (xData[curDataPoint] - xData[curDataPoint+1]);
            right       = (0.5*max - yData[curDataPoint+1] + inclination*xData[curDataPoint+1]) / inclination;
            break;
        }
    }

    return right - left;
}
//-----------------------------------------------------------------------------
// Name: calcIntegralWidth()
// Desc: 
//-----------------------------------------------------------------------------
double calcIntegralWidth(double xData[], double yData[], unsigned int numDataPoints)
{
    // locals
    double          sum = 0;
    double          max = 0;
    unsigned int    curDataPoint;
   
    // calc maximum
    for (curDataPoint=0; curDataPoint<numDataPoints; curDataPoint++) {
        if (max < yData[curDataPoint]) max = yData[curDataPoint];
    }

    // calc sum
    for (curDataPoint=0; curDataPoint<numDataPoints-1; curDataPoint++) {
        sum += (yData[curDataPoint] + 0.5*(yData[curDataPoint+1]-yData[curDataPoint])) * (xData[curDataPoint+1] - xData[curDataPoint]);
    }

    return sum  / max;
}

//-----------------------------------------------------------------------------
// Name: findCentersBetweenPeaks()
// Desc: 
//-----------------------------------------------------------------------------
bool findCentersBetweenPeaks(double xData[],                    // in: x-coordinates of data points
                             double yData[],                    // in: y-coordinates of data points
                             unsigned int numDataPoints,        // in: number of given datapoints
                             unsigned int &numPeaks,            // 
                             unsigned int * &startingIndex,     // 
                             unsigned int * &peakPosition,      //
                             unsigned int * &width,             // 
                             double minDelta)                   // in:
{
    // locals
    unsigned int i;
    unsigned int curPeak            = 0;
    unsigned int *myPeakPosition    = new unsigned int[numDataPoints];
    unsigned int lastPeakPos;
    double       minValue           = 1e50;
    double       maxValue           =-1e50;
    bool         searchingNextPeak;
    double       lastPeakHeight;

    // calc minValue and maxValue
    for (i=0; i<numDataPoints; i++) {
        if (minValue>yData[i]) minValue = yData[i];
        if (maxValue<yData[i]) maxValue = yData[i];
    }

    // calc peakPosition and numPeaks
    numPeaks            = 0;
    lastPeakPos         = 0;
    lastPeakHeight      = minValue;
    searchingNextPeak   = true;
    
    for (i=0; i<numDataPoints; i++) {
        if (yData[i] > lastPeakHeight && searchingNextPeak) {
            lastPeakHeight          = yData[i];
            lastPeakPos             = i;
        } else if (yData[i] < lastPeakHeight - minDelta && searchingNextPeak) {
            searchingNextPeak       = false;
            myPeakPosition[numPeaks]= lastPeakPos;
            numPeaks++;
        } else if (yData[i] > lastPeakHeight + minDelta && !searchingNextPeak) {
            searchingNextPeak       = true;
        } else if (yData[i] < lastPeakHeight && !searchingNextPeak) {
            lastPeakHeight          = yData[i];
        }
    }
    
    if (numPeaks==0) {
        delete [] myPeakPosition;
        return false;
    }

    // calc output parameters
    startingIndex       = new unsigned int[numPeaks];
    width               = new unsigned int[numPeaks];
    peakPosition        = new unsigned int[numPeaks];
    startingIndex[0]    = 0;
    width[0]            = numDataPoints;
    for (curPeak=0; curPeak<numPeaks; curPeak++) {
        peakPosition[curPeak] = myPeakPosition[curPeak];
    }
    for (curPeak=0; curPeak<numPeaks-1; curPeak++) {
        startingIndex[curPeak+1]    = (peakPosition[curPeak] + peakPosition[curPeak+1]) / 2;
        width[curPeak]              = startingIndex[curPeak+1] - startingIndex[curPeak];
    }
    width[curPeak] = numDataPoints - startingIndex[curPeak] - 1;

    delete [] myPeakPosition;
    return true;
}

/*** Gauss Function ********************************************************************/
double gaussFunction(double *p)
{
    double x = p[0];
    double a = p[1];
    double b = p[2];
    double c = p[3];
    return a * exp(-((x-b)*(x-b))/(2*c*c));
}

double gradientOfGaussFunction_y0(double *p)
{
    double x = p[0];
    double a = p[1];
    double b = p[2];
    double c = p[3];
    double f = p[4];
    return f / a;
}

double gradientOfGaussFunction_x0(double *p)
{
    double x = p[0];
    double a = p[1];
    double b = p[2];
    double c = p[3];
    double f = p[4];
    return f * (x-b) / (c*c);
}

double gradientOfGaussFunction_w(double *p)
{
    double x = p[0];
    double a = p[1];
    double b = p[2];
    double c = p[3];
    double f = p[4];
    return f *  (x-b)*(x-b) / (c*c*c);
}

/*** Pseudo-Voigt Function ********************************************************************
double pseudoVoightFunction(double *p)
{
    double x  = p[0];
    double y0 = p[1];
    double x0 = p[2];
    double w  = p[3];
    double n  = p[4];

    double s = (x-x0)/w;
    double q = (s*s);
    double L = 1/(1+q);                 // Lorentz
    double G = exp(-1*log(2.0)*q);      // Gauss
    return y0 * (n*L + (1-n)*G);
}

double gradientOfPseudoVoightFunction_x0(double *p)
{
    double x  = p[0];
    double y0 = p[1];
    double x0 = p[2];
    double w  = p[3];
    double n  = p[4];
    double f  = p[5];

    double s = (x-x0)/w;
    double q = (s*s);
    double L = (2*(x-x0)/(w*w))/((q+1)*(q+1));
    double G = log(2.0)*pow(2, -q)*(2*(x-x0)/(w*w));

    return y0*((1-n)*G-n*L);
}

double gradientOfPseudoVoightFunction_w(double *p)
{
    double x  = p[0];
    double y0 = p[1];
    double x0 = p[2];
    double w  = p[3];
    double n  = p[4];
    double f  = p[5];

    double s = (x-x0)/w;
    double q = (s*s);
    double L = 2*q/w/((q+1)*(q+1));
    double G = log(2.0)*pow(2, -q)*(2*q/w);

    return y0*((1-n)*G+n*L);
}

double gradientOfPseudoVoightFunction_n(double *p)
{
    double x  = p[0];
    double y0 = p[1];
    double x0 = p[2];
    double w  = p[3];
    double n  = p[4];
    double f  = p[5];

    double s = (x-x0)/w;
    double q = (s*s);
    double L = 1/(q+1);
    double G = log(2.0)*pow(2, -q);

    return y0*((1-n)*G - pow(2, -q) + L);
}

double gradientOfPseudoVoightFunction_y0(double *p)
{
    double x  = p[0];
    double y0 = p[1];
    double x0 = p[2];
    double w  = p[3];
    double n  = p[4];
    double f  = p[5];

    return f / y0;
}

/*** Pseudo-Voigt Function ********************************************************************/
double pseudoVoigtFunction(double *p)
{
    double x  = p[0];
    double y0 = p[1];
    double x0 = p[2];
    double wl = p[3];
    double wg = p[4];
    double n  = p[5];

    double sg = 2*(x-x0)/wg;
    double sl = 2*(x-x0)/wl;
    double L  = 1/(1+sl*sl);             // Lorentz
    double G  = exp(-1*log(2.0)*sg*sg);  // Gauss
    return y0 * (n*L + (1-n)*G);         // Pseudo-Voigt
}

double gradientOfPseudoVoigtFunction_x0(double *p)
{
    double x  = p[0];
    double y0 = p[1];
    double x0 = p[2];
    double wl = p[3];
    double wg = p[4];
    double n  = p[5];
    double f  = p[6];

    double s  = 4 * (x0-x)*(x0-x);
    double tl = s / (wl*wl) + 1.0;
    double L  = n / (tl*tl*wl*wl);
    double tg = pow(2.0, -1.0*s / (wg*wg));
    double G  = (n-1)*log(2.0)*tg / (wg*wg);
    return 8*y0*(x-x0)*(L-G);
}

double gradientOfPseudoVoigtFunction_wl(double *p)
{
    double x  = p[0];
    double y0 = p[1];
    double x0 = p[2];
    double wl = p[3];
    double wg = p[4];
    double n  = p[5];
    double f  = p[6];

    double d = x0 - x;
    double t = 4*x0*x0 - 8*x0*x + wl*wl + 4*x*x;
    return 8*y0*wl*n*d*d / (t*t);
}

double gradientOfPseudoVoigtFunction_wg(double *p)
{
    double x  = p[0];
    double y0 = p[1];
    double x0 = p[2];
    double wl = p[3];
    double wg = p[4];
    double n  = p[5];
    double f  = p[6];

    double d = x0 - x;
    double e = pow(2.0, 3 - 4*d*d/(wg*wg));
    double t = y0*(n-1)*log(2.0)*d*d*e;
    return -1 * t / (wg*wg*wg);
}

double gradientOfPseudoVoigtFunction_n(double *p)
{
    double x  = p[0];
    double y0 = p[1];
    double x0 = p[2];
    double wl = p[3];
    double wg = p[4];
    double n  = p[5];
    double f  = p[6];

    double s = 4.0*(x0-x)*(x0-x);
    double L = 1.0 / (s/(wl*wl) + 1.0);
    double G = pow(2.0, -1.0*s/(wg*wg));
    return y0 * (L - G);
}

double gradientOfPseudoVoigtFunction_y0(double *p)
{
    double x  = p[0];
    double y0 = p[1];
    double x0 = p[2];
    double wl = p[3];
    double wg = p[4];
    double n  = p[5];
    double f  = p[6];

    return f / y0;
}

//-----------------------------------------------------------------------------
// Name: makeNumberStr()
// Desc: Writes "value" into the char-array "str", but replaces it by "invalidStr" 
//       if the value is < "minValue" or > "maxValue".
//-----------------------------------------------------------------------------
void makeNumberStr(char *str, double value, double minValue, double maxValue, char *invalidStr, unsigned int length)
{
    // locals
    unsigned int i;

    // print value
    if (value <= minValue || value >= maxValue) {
        strcpy(str, invalidStr);
    } else {
        sprintf(str, "%e", value);
    }

    // fill up with blank space
    for (i=(unsigned int) strlen(str); i<length; i++) {
        str[i] = ' ';
    }
    str[i] = '\0';
}

//-----------------------------------------------------------------------------
// Name: swapPointers()
// Desc: Swap two pointers.
//-----------------------------------------------------------------------------
void swapPointers(void **pt1, void **pt2)
{
    void *pt3 = *pt2;
         *pt2 = *pt1;
         *pt1 = pt3;
}

//-----------------------------------------------------------------------------
// Name: doNothing()
// Desc: Returns the same value.
//-----------------------------------------------------------------------------
double doNothing(double x)
{
    return x; 
}

//-----------------------------------------------------------------------------
// Name: inverseValue()
// Desc: Return 1/x.
//-----------------------------------------------------------------------------
double inverseValue(double x)
{
    return 1.0/x; 
}

//-----------------------------------------------------------------------------
// Name: divide()
// Desc: Return dividend/divisor.
//-----------------------------------------------------------------------------
double divide(double dividend, double divisor)
{
    return dividend/divisor; 
}

//-----------------------------------------------------------------------------
// Name: isValuePositive()
// Desc: Return true when value>0
//-----------------------------------------------------------------------------
bool isValuePositive(double value)
{
    return (value>0.0); 
}

//-----------------------------------------------------------------------------
// Name: isValuePositive()
// Desc: Return true when value>0
//-----------------------------------------------------------------------------
bool isValuePositive(float value)
{
    return (value>0.0f); 
}

//-----------------------------------------------------------------------------
// Name: isEqual()
// Desc: Return true when a==b
//-----------------------------------------------------------------------------
bool isEqual(double a, double b, double delta)
{
    return ((a+delta>b)&&(a-delta<b));
}

//-----------------------------------------------------------------------------
// Name: isIntegerValue()
// Desc: Return true when a+delta>int(a) && a-delta<int(a)
//-----------------------------------------------------------------------------
bool isIntegerValue(double a, double delta)
{
    return ((a+delta>int(a))&&(a-delta<int(a)));
}

//-----------------------------------------------------------------------------
// Name: doesLineCrossRectangle()
// Desc: Return true when line defined by both points (x1,y1,x2,y2) crosses the rectangle defined by the four points (rx[4],ry[4])
//-----------------------------------------------------------------------------
bool doesLineCrossRectangle(double x1, double y1, double x2, double y2, double rx[], double ry[])
{
    return false;
}

//-----------------------------------------------------------------------------
// Name: dotProduct()
// Desc: Return the dot product of two 2D vectors
//-----------------------------------------------------------------------------
double dotProduct(double vx1, double vy1, double vx2, double vy2)
{
    return vx1 * vx2 + vy1 * vy2;
}

//-----------------------------------------------------------------------------
// Name: isPointInsideParallelogram()
// Desc: Return true when point defined by (x,y) is inside the rectangle defined by the four points (rx[3],ry[3])
//-----------------------------------------------------------------------------
bool isPointInsideParallelogram(double x, double y, double rx[], double ry[])
{
    // locals
    double v1x = rx[1] - rx[0];
    double v1y = ry[1] - ry[0];
    double v2x = rx[2] - rx[0];
    double v2y = ry[2] - ry[0];
    double vx  =     x - rx[0];
    double vy  =     y - ry[0];

    return (                        0 <= dotProduct( vx, vy,v1x,v1y) 
         && dotProduct(vx,vy,v1x,v1y) <= dotProduct(v1x,v1y,v1x,v1y)
         &&                         0 <= dotProduct( vx, vy,v2x,v2y)
         && dotProduct(vx,vy,v2x,v2y) <= dotProduct(v2x,v2y,v2x,v2y));
}

//-----------------------------------------------------------------------------
// Name: isValueNegative()
// Desc: Return true when value<0
//-----------------------------------------------------------------------------
bool isValueNegative(float value)
{
    return (value<0.0f); 
}

//-----------------------------------------------------------------------------
// Name: foldDataPointsWithFunction()
// Desc: Perform a folding of data points with a function in order to simulate peak-broadening
//-----------------------------------------------------------------------------
void foldDataPointsWithFunction(table<double>  &spectrum, 
                                ptFunc          function, 
                                double *        parameter, 
                                table<double>  &datapoints, 
                                double          xDelta,
                                double          xBorder)
{
    // parameters ok?
    if (function   == 0) return;
    if (parameter  == 0) return;

    // locals
    unsigned int    curDataPoint, curRow, curFunctionValue;
    unsigned int    colX        = 0;
    unsigned int    colY        = 1;
    unsigned int    fvFactor    = 3;
    double          curX;
    double          xMin        = datapoints.calcMinValue(colX);
    double          xMax        = datapoints.calcMaxValue(colX);
    double          *functionValue;

    // allocate mem for result
    spectrum.redim(2, (int)((xMax - xMin + 2*xBorder) / xDelta)+1);

    // precalculate function values
    functionValue = new double[fvFactor*spectrum.numRows];
    for (curRow=0; curRow<fvFactor*spectrum.numRows; curRow++) {
        parameter[0]                = (curRow - fvFactor * spectrum.numRows / 2.0) * xDelta;
        functionValue[curRow]       = function(parameter);
    }

    // zero output variables
    for (curRow=0; curRow<spectrum.numRows; curRow++) {
        spectrum.data[colX * spectrum.numRows + curRow]    = xMin - xBorder + curRow * xDelta;
        spectrum.data[colY * spectrum.numRows + curRow]    = 0;
    }

    // process each data point
    for (curDataPoint=0; curDataPoint<datapoints.numRows; curDataPoint++) {
        for (curRow=0; curRow<spectrum.numRows; curRow++) {
            curX                                             = xMin - xBorder + curRow * xDelta - datapoints.data[colX * datapoints.numRows + curDataPoint];
            curFunctionValue                                 = (unsigned int) (curX / xDelta + fvFactor * spectrum.numRows / 2.0);
            spectrum.data[colY * spectrum.numRows + curRow] += functionValue[curFunctionValue] * datapoints.data[colY * datapoints.numRows + curDataPoint];
        }
    }

    delete [] functionValue;
}

//-----------------------------------------------------------------------------
// Name: isInsideRect()
// Desc: Returns true if the passed position is inside the define rectangle.
//-----------------------------------------------------------------------------
bool isInsideRect(double left, double top, double right, double bottom, double xPos, double yPos)
{
    return (xPos > left && xPos < right && yPos > top && yPos < bottom);
}

//-----------------------------------------------------------------------------
// Name: isInsideCirCle()
// Desc: Returns true if the distance between the circle center and the passed position is smaller than the radius.
//-----------------------------------------------------------------------------
bool isInsideCirCle(double circleCenterX, double circleCenterY, double radius, double xPos, double yPos)
{
    double dx = circleCenterX - xPos;
    double dy = circleCenterY - yPos;
    return (sqrt(dx*dx+dy*dy) < radius);
}

//-----------------------------------------------------------------------------
// Name: calcRoughness()
// Desc: avg  = 1/n * sum(z)          - Mittelwert
//       Ra   = 1/n * sum(abs(z-avg)) - Arithmetischer Mittenrauwert DIN EN ISO 4287
//       Rq   = sqrt(1/n*sum(z*z))    - Quadratischer Mittenrauwert  DIN EN ISO 4287
//       Rt   = ???                   - Rautiefe
//       Rz   = 1/m * Sum(Rz_i)       - Gemittelte Rautiefe
//  
//       z_i                          - Datenpunkte einer Einzelmessstrecke
//       Rz_i = max(z_i)-min(z_i)     - Einzelrautiefe
//       Rmax = max(Rz_i)             - Maximale Rautiefe
//       m    = n/sectionLength       - Anzahl der sections
//  
//-----------------------------------------------------------------------------
void calcRoughness(unsigned int numDatapoints, unsigned int sectionLength, double z[], double &Ra, double &Rz, double &Rt, double &Rq, double &avg)
{
    // locals
    unsigned int    curDatapoint, curSection;                       // counter vars
    unsigned int    numSections = numDatapoints / sectionLength;    // number of sections m
    double          sumAvg, sumRa, sumRq, sumRz;                    // temporary sums
    double          minInCurSection, maxInCurSection;

    // init
    sumAvg          = 0;
    sumRa           = 0;
    sumRq           = 0;
    sumRz           = 0;
    minInCurSection = +1e30;
    maxInCurSection = -1e30;

    // calc average
    for (curDatapoint=0; curDatapoint<numDatapoints; curDatapoint++) {
        sumAvg  += z[curDatapoint];
    }
    avg = sumAvg / numDatapoints;
    
    // sum up
    for (curSection=0, curDatapoint=0; curDatapoint<numDatapoints; curDatapoint++) {

        sumRa   += abs(z[curDatapoint] - avg);
        sumRq   += z[curDatapoint] * z[curDatapoint];

        // min/max of current section
        if (minInCurSection > z[curDatapoint]) minInCurSection = z[curDatapoint];
        if (maxInCurSection < z[curDatapoint]) maxInCurSection = z[curDatapoint];

        if (curDatapoint > (1+curSection)*sectionLength) {
            sumRz           += maxInCurSection - minInCurSection;
            minInCurSection  = +1e30;
            maxInCurSection  = -1e30;
            curSection++;
        }
    }
    
    // calc averages
    Ra = sumRa / numDatapoints;
    Rq = sqrt(sumRq / numDatapoints);
    Rz = sumRz / numSections;
    Rt = 0; // ???
}

//-----------------------------------------------------------------------------
// Name: makeString()
// Desc: Calls the new-operator, copies 'Str' into the new array, fills up with blank-space until 'length' and returns a pointer to the new array.
//       A use of stringstreams is always better!
//-----------------------------------------------------------------------------
char *makeString(char str[], unsigned int length)
{
    // locals
    char          * myStr = new char[max(strlen(str)+1, length+1)];
    unsigned int    i;

    // copy string
    strcpy(myStr, str);

    // fill up with blank space
    for (i=(unsigned int) strlen(str); i<length; i++) {
        myStr[i] = ' ';
    }
    myStr[i] = '\0';

    // return pointer
    return myStr;
}

//-----------------------------------------------------------------------------
// Name: makeString()
// Desc: Copies 'source' to 'dest' and fill up with blank space until 'length' is reached.
//-----------------------------------------------------------------------------
void makeString(char *dest, char source[], unsigned int length)
{
    // locals
    unsigned int    i;

    // copy string
    strcpy(dest, source);

    // fill up with blank space
    for (i=(unsigned int) strlen(source); i<length; i++) {
        dest[i] = ' ';
    }
    dest[i] = '\0';
}

//-----------------------------------------------------------------------------
// Name: addString()
// Desc: Appends 'source' to 'dest' and fill up with blank space until 'length' is reached.
//-----------------------------------------------------------------------------
void addString(char *dest, char source[], unsigned int length)
{
    // locals
    unsigned int    offset = (unsigned int) strlen(dest);
    unsigned int    i;

    // copy string
    strcat(dest, source);

    // fill up with blank space
    for (i=(unsigned int) strlen(source); i<length; i++) {
        dest[offset + i] = ' ';
    }
    dest[offset + i] = '\0';
}

//-----------------------------------------------------------------------------
// Name: showMessageAndQuit()
// Desc: Prints 'msg' with the cerr-stream and returns 'returnValue'.
//       Doing so one command (return showMessageAndQuit("blub", returnValue);) is sufficient for errorhandling (instead of { cerr << "blub"; return returnValue; } ).
//-----------------------------------------------------------------------------
bool showMessageAndQuit(char *msg, bool returnValue)
{
    char c;
    cerr << msg << endl;
    cin >> c;
    return returnValue;
}

//-----------------------------------------------------------------------------
// Name: fitToNonLinearFunction()
// Desc: Fits a nonlinear function.
//-----------------------------------------------------------------------------
void fitDoublet            (unsigned int numParameters,         // in: number of parameters to fit
                            double *fitParameter,               // in/out: parameters to fit - initial values and fitted values
                            ptFunc function,                    // in: function to fit
                            ptFunc* gradient,                   // in: gradients of function, one for each parameter to fit
                            double xData[],                     // in: x-coordinates of data points
                            double yData[],                     // in: y-coordinates of data points
                            unsigned int numDataPoints)         // in: number of given datapoints
{

    // locals
    int                 xIndex              = 0;
    int                 parOffset           = 1;
    int                 yIndex              = numParameters + 1;
    int                 numCurves           = 2;
    int                 numDimensions       = 1;
    int                 numParamsBeingFit   = numCurves*numParameters;
    real_1d_array       yPoints;
    real_2d_array       xPoints;
    real_1d_array       cParam;
    lsfitreport         rep;
    lsfitstate          state;
    ae_int_t            info;
    double              epsf;
    double              epsx;
    int                 maxits;
    int                 i;
    double              *param   = new double[numCurves*(numParameters+2)];     // +2 for x and f
    
    // Prepare task matrix
    yPoints.setlength(numDataPoints);
    xPoints.setlength(numDataPoints, numDimensions);
    cParam.setlength(numParamsBeingFit);
    for(i=0; i<(int) numDataPoints; i++) {
        xPoints(i,0)  = xData[i];
        yPoints(i)    = yData[i];
    }

    // initial value
    for (i=0; i<numParamsBeingFit; i++) {
        cParam(i)    = fitParameter[i];
    }

    // stopping conditions
    epsf    = 0.0;
    epsx    = 0.00001;
    maxits  = 0;
    
    // Solve
    lsfitcreatefg(xPoints, yPoints, cParam, numDataPoints, numDimensions, numParamsBeingFit, true, state);
    lsfitsetcond(state, epsf, epsx, maxits);
/*  
    lsfitfit(state, function_cx_1_func, function_cx_1_grad);

    void function_cx_1_func(const real_1d_array &c, const real_1d_array &x, double &func, void *ptr) 
    {
        // this callback calculates f(c,x)=exp(-c0*sqr(x0))
        // where x is a position on X-axis and c is adjustable parameter
        func = exp(-c[0]*pow(x[0],2));
    }
    void function_cx_1_grad(const real_1d_array &c, const real_1d_array &x, double &func, real_1d_array &grad, void *ptr) 
    {
        // this callback calculates f(c,x)=exp(-c0*sqr(x0)) and gradient G={df/dc[i]}
        // where x is a position on X-axis and c is adjustable parameter.
        // IMPORTANT: gradient is calculated with respect to C, not to X
        func = exp(-c[0]*pow(x[0],2));
        grad[0] = -pow(x[0],2)*func;
    }

    while(lsfitnonlineariteration(state))
    {
        // copy x and parameters - y will not be copied
        for (j=0; j<numCurves; j++) {
            param[j*(numParameters+2)+xIndex] = state.x(0);
            for (i=0; i<numParameters; i++) {
                param[j*(numParameters+2)+i+parOffset] = state.c(j*numParameters+i);
            }
        }

        // function value F(x)
        if( state.needf ) {

            // fit an linear combination of the function
            for (state.f=0, j=0; j<numCurves; j++) {
                state.f += function(&param[j*numParameters+0]);
            }

        // function value F(x) and gradient dF/dc
        } else if( state.needfg ) {
            
            for (state.f=0, j=0; j<numCurves; j++) {
                param[j*(numParameters+2)+yIndex]  = function(&param[j*(numParameters+2)+0]);
                state.f                           += param[j*(numParameters+2)+yIndex]; 
                for (i=0; i<numParameters; i++) {
                    state.g(j*numParameters+i)  = gradient[i](&param[j*(numParameters+2)+0]);
                }
            }
        }
    }
*/
    // extract results
    lsfitresults(state, info, cParam, rep);
    
    for (i=0; i<numParamsBeingFit; i++) {
        fitParameter[i] = cParam(i);
        printf("%d:       %0.3lf\n", i+1, double(cParam(i)));
    }
    printf("rms.err: %0.3lf\n",         double(rep.rmserror));
    printf("max.err: %0.3lf\n",         double(rep.maxerror));
    printf("Termination type: %0ld\n",  long(info));
    printf("\n\n");

    // free mem
    delete [] param;
}

//-----------------------------------------------------------------------------
// Name: timetoStr()
// Desc: Return a formatted date and time string based on the passed number of seconds since the 1st Jan 1900.
//       %d		Day of the month, 01-31
//       %m		Month, 01-12
//       %Y		Year, 4-digit
//       %H		hour, 00-23 (always two digits)
//       %M		minute, 0-60
//       %S		second, integer 0-60 on output, double on input
//-----------------------------------------------------------------------------
void timeToStr(stringstream &ss, const char *timeFormat, double secondsPassedSince1900, unsigned int secondsPrecision)
{
	// locals
	tm				tmTime;
	time_t			now;
	unsigned int	curChrInTimeFormat	= 0;
	unsigned int	lenTimeFormat		= (unsigned int) strlen(timeFormat);
	double			power				= pow((double) 10, (int) secondsPrecision);
	int				secFraction			= (int) std::ceil((secondsPassedSince1900 - (unsigned long) secondsPassedSince1900) * power - 0.5);

	now		= (time_t) secondsPassedSince1900;
	tmTime	= *localtime(&now);

	// handle case where time is hh:mm:ss.100
	if (secFraction == (int) power) {
		secFraction = 0;
		tmTime.tm_sec += 1;
		if (tmTime.tm_sec == 60) {
			tmTime.tm_sec = 0;
			tmTime.tm_min += 1;
			if (tmTime.tm_min == 60) {
				tmTime.tm_min = 0;
				tmTime.tm_hour += 1;
				if (tmTime.tm_hour == 24) {
					mktime(&tmTime);
				}
			}
		}
	}

	ss.str("");

	while (curChrInTimeFormat < lenTimeFormat) {

		switch (timeFormat[curChrInTimeFormat])
		{
		case '%': 
			curChrInTimeFormat++;
			switch (timeFormat[curChrInTimeFormat]) 
			{
			case 'd': // Day of the month, 01-31
				ss << setw(2) << setfill('0') << tmTime.tm_mday;
				break;
			case 'm': // Month, 01-12 (+1 because the month in tm-structure is zero based)
				ss << setw(2) << setfill('0') << tmTime.tm_mon + 1;
				break;
			case 'Y': // Year, 4-digit
				ss << tmTime.tm_year + 1900;
				break;
			case 'H': // hour, 00-23 (always two digits)
				ss << setw(2) << setfill('0') << tmTime.tm_hour;
				break;
			case 'M': // hour, minute, 0-60
				ss << setw(2) << setfill('0') << tmTime.tm_min;
				break;		
			case 'S': // second, integer 0-60 on output, double on input
				ss << setw(2) << setfill('0') << tmTime.tm_sec << "." << setw(secondsPrecision) << setfill('0') << secFraction;
				break;
			}
			curChrInTimeFormat++;
			break;
		default:
			ss << timeFormat[curChrInTimeFormat];
			curChrInTimeFormat++;
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: strToTime()
// Desc: Return the number of seconds passed since the 1st Jan 1900
//       %d		Day of the month, 01-31
//       %m		Month, 01-12
//       %Y		Year, 4-digit
//       %H		hour, 00-23 (always two digits)
//       %M		minute, 0-60
//       %S		second, integer 0-60 on output, double on input
//-----------------------------------------------------------------------------
double strToTime(const char * str, const char * timeFormat, char decimalSeparator)
{
	// locals
	time_t			zeroHour			= (time_t) 0;
	tm				tmTime				= {0,0,0,0,0,0,0,0,0};				//	= *localtime(&zeroHour);
	char			cNumber[16];
	unsigned int	curChrInNumber		= 0;
	unsigned int	curChrInStr			= 0;
	unsigned int	curChrInTimeFormat	= 0;
	unsigned int	lenTimeFormat		= (unsigned int) strlen(timeFormat);
	double			fractionOfSecond	= 0;

	while (curChrInTimeFormat < lenTimeFormat) {

		switch (timeFormat[curChrInTimeFormat])
		{
		case '%': 
			curChrInTimeFormat++;
			curChrInNumber = 0;
			switch (timeFormat[curChrInTimeFormat]) 
			{
			case 'd': // Day of the month, 01-31
																	      cNumber[curChrInNumber++] = str[curChrInStr++]; 
				if (str[curChrInStr] >= '0' && str[curChrInStr] <= '9') { cNumber[curChrInNumber++] = str[curChrInStr++]; }
				cNumber[curChrInNumber] = '\0';
				tmTime.tm_mday	= atoi(cNumber);
				break;
			case 'm': // Month, 01-12 
																	      cNumber[curChrInNumber++] = str[curChrInStr++]; 
				if (str[curChrInStr] >= '0' && str[curChrInStr] <= '9') { cNumber[curChrInNumber++] = str[curChrInStr++]; }
				cNumber[curChrInNumber] = '\0';
				tmTime.tm_mon	= atoi(cNumber) - 1; // (-1 because the month in tm-structure is zero based)
				break;
			case 'Y': // Year, 4-digit
				cNumber[curChrInNumber++] = str[curChrInStr];	curChrInStr++;
				cNumber[curChrInNumber++] = str[curChrInStr];	curChrInStr++;
				cNumber[curChrInNumber++] = str[curChrInStr];	curChrInStr++;
				cNumber[curChrInNumber++] = str[curChrInStr];	curChrInStr++;
				cNumber[curChrInNumber  ] = '\0';
				tmTime.tm_year	= atoi(cNumber) - 1900;
				break;
			case 'H': // hour, 00-23 (always two digits)
				cNumber[curChrInNumber++] = str[curChrInStr];	curChrInStr++;
				cNumber[curChrInNumber++] = str[curChrInStr];	curChrInStr++;
				cNumber[curChrInNumber  ] = '\0';
				tmTime.tm_hour = atoi(cNumber);
				break;
			case 'M': // hour, minute, 0-60
																	      cNumber[curChrInNumber++] = str[curChrInStr++]; 
				if (str[curChrInStr] >= '0' && str[curChrInStr] <= '9') { cNumber[curChrInNumber++] = str[curChrInStr++]; }
				cNumber[curChrInNumber] = '\0';
				tmTime.tm_min = atoi(cNumber);
				break;		
			case 'S': // second, integer 0-60 on output, double on input
																		  cNumber[curChrInNumber++] = str[curChrInStr++]; 
				if (str[curChrInStr] >= '0' && str[curChrInStr] <= '9') { cNumber[curChrInNumber++] = str[curChrInStr++]; }
				cNumber[curChrInNumber] = '\0';
				if (str[curChrInStr] == decimalSeparator) {
					tmTime.tm_sec = atoi(cNumber);
					curChrInStr++;
					cNumber[0]		= '0';
					cNumber[1]		= '.';
					curChrInNumber	= 2;
					while (str[curChrInStr] >= '0' && str[curChrInStr] <= '9') { cNumber[curChrInNumber++] = str[curChrInStr++]; }
					cNumber[curChrInNumber] = '\0';
					fractionOfSecond = atof(cNumber);
				} else {
					tmTime.tm_sec = atoi(cNumber);
				}
				break;
			}
			curChrInTimeFormat++;
			break;
		default:
			curChrInTimeFormat++;
			curChrInStr++;
			break;
		}
	}

	// if not onyl time and no date is passed than add year, month and date
	if (tmTime.tm_year == 0) { 
		tmTime.tm_year	= 70;
		tmTime.tm_mday	= 1;
		tmTime.tm_mon	= 0;
		tmTime.tm_isdst = 0;
		tmTime.tm_hour  += 1;	// mktime() returns however 0 for the time 01:00:00
	}

	// calc seconds passed since 1900
	double myTime = (double) mktime(&tmTime);
	if (myTime < 0) {
		cout << endl << "ERROR: strToTime(): mktime() returned " << myTime << " when converting time " << str;
		while (true);
	}
	correctSummerWinterTime(tmTime, myTime);
	return myTime + fractionOfSecond;
}

//-----------------------------------------------------------------------------
// Name: correctSummerWinterTime()
// Desc: should be called after a mktime call.
//-----------------------------------------------------------------------------
void correctSummerWinterTime(tm &tmTime, double &myTime)
{
	// correct summer/winter time
	if (tmTime.tm_isdst == 1) {
		myTime -= 3600;
	// the summertime ends in Germany on the last Sunday in October
	// except in the years 1981–1995 where it was the last Sunday in september
	} else if (tmTime.tm_hour == 2 && tmTime.tm_wday == 0) {
		if (tmTime.tm_year >= 82 && tmTime.tm_year <= 95) {
			if (tmTime.tm_mon == 8) {
				myTime -= 3600;
			}
		} else {
			if (tmTime.tm_mon == 9) {
				myTime -= 3600;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Name: convertTime()
// Desc: 
//-----------------------------------------------------------------------------
void convertTime(SYSTEMTIME &source, tm &target, double &secondsPassedSince1900)
{
	target.tm_year		= source.wYear - 1900;
	target.tm_mon		= source.wMonth - 1;
	target.tm_mday		= source.wDay;
	target.tm_hour		= source.wHour;
	target.tm_min		= source.wMinute;
	target.tm_sec		= source.wSecond;
	target.tm_wday		= source.wDayOfWeek;
	target.tm_yday		= 0;
	target.tm_isdst		= 0;

	secondsPassedSince1900 = (double) mktime(&target);
	correctSummerWinterTime(target, secondsPassedSince1900);
	secondsPassedSince1900 += source.wMilliseconds * 0.001;
}

//-----------------------------------------------------------------------------
// Name: testTimeToStrConversion()
// Desc: Generates random doubles representating the time since 1900 in seconds, than converts it to a string, back to a double and again to a string.
//-----------------------------------------------------------------------------
bool testTimeToStrConversion(unsigned int numTests, unsigned int verbosity, bool stopOnError)
{
	// locals
	unsigned int	curTest;
	unsigned int	curPresision;
	double			secondsPassedSince1900_random;
	double			secondsPassedSince1900_converted;
	double			lower_bound			= 0;
	double			upper_bound			= 100.0*365.0*24.0*60.0*60.0;		// roughly 100 years
	double			precisionFactor;
	unsigned int	secondsPrecision	= 1;
	double			comparisonPresision = pow((double) 10.0, (int) -1*((int)secondsPrecision+1));
	stringstream	ss_1stConversion;
	stringstream	ss_2ndConversion;
	string			timeFormat("%d.%m.%Y %H:%M:%S");

	std::uniform_real_distribution<double>	unif(lower_bound,upper_bound);
	std::default_random_engine				re;
	srand((unsigned int)time(NULL));
	// ... initialization of unif() is not implemented yet

	for (curTest=0; curTest<numTests; curTest++) {

		// output 
		if (verbosity > 1) cout << endl;

		// generate random time in seconds after 1900
		secondsPassedSince1900_random	= ceil(unif(re));
		for (curPresision=0, precisionFactor=0.1; curPresision<secondsPrecision; curPresision++, precisionFactor/=10.0) {
			int		digit	= (rand() % 10);
			secondsPassedSince1900_random += digit * precisionFactor; 
		}
		if (verbosity > 1) cout << "Seconds passed since 1900:\t" << std::fixed << setprecision(1) <<secondsPassedSince1900_random << endl;

		// convert time into a string
		timeToStr(ss_1stConversion, timeFormat.c_str(), secondsPassedSince1900_random, 1);
		if (verbosity > 1) cout << "Calculated time string:\t\t" << ss_1stConversion.str() << endl;

		// convert string back to time in seconds
		secondsPassedSince1900_converted = strToTime(ss_1stConversion.str().c_str(), timeFormat.c_str(), '.');
		if (verbosity > 1) cout << "Seconds passed since 1900:\t" << std::fixed << setprecision(1) << secondsPassedSince1900_converted << endl;
		
		// compare calculated time strings 
		if (secondsPassedSince1900_random - comparisonPresision > secondsPassedSince1900_converted ||  secondsPassedSince1900_random + comparisonPresision < secondsPassedSince1900_converted)  {
			if (verbosity > 0) cout << "Time in seconds differ!" << endl;
			if (stopOnError) {
				while (stopOnError);
			} else {
				if (verbosity > 0 && verbosity < 2) {
					 cout << "Seconds passed since 1900:\t" << std::fixed << setprecision(1) <<secondsPassedSince1900_random << endl;
					 cout << "Calculated time string:\t\t" << ss_1stConversion.str() << endl;
					 cout << "Seconds passed since 1900:\t" << std::fixed << setprecision(1) << secondsPassedSince1900_converted << endl << endl;
				}
			}
			//return false;
		}

		// convert time again to a string
		timeToStr(ss_2ndConversion, timeFormat.c_str(), secondsPassedSince1900_converted, 1);
		if (verbosity > 1) cout << "Calculated time string:\t\t" << ss_2ndConversion.str() << endl;

		// compare calculated time strings 
		if (ss_1stConversion.str() != ss_2ndConversion.str())  {
			if (verbosity > 0) cout << "String differ!" << endl;
			if (stopOnError) {
				while (stopOnError);
			} else {
				if (verbosity > 0 && verbosity < 2) {
					 cout << "Seconds passed since 1900:\t" << std::fixed << setprecision(1) <<secondsPassedSince1900_random << endl;
					 cout << "Calculated time string:\t\t" << ss_1stConversion.str() << endl;
					 cout << "Seconds passed since 1900:\t" << std::fixed << setprecision(1) << secondsPassedSince1900_converted << endl;
					 cout << "Calculated time string:\t\t" << ss_2ndConversion.str() << endl << endl;
				}
			}			
			//return false;
		}
	}

	// test successful
	return true;
}

//-----------------------------------------------------------------------------
// Name: addTabs()
// Desc: Add '\t' to 'ss' until 'lengthToFill' chars has been added.
//-----------------------------------------------------------------------------
void addTabs(ostream &ss, unsigned int strLength, unsigned int tabLength, unsigned int lengthToFill)
{
	if (lengthToFill > strLength) {
		ss << string((lengthToFill - strLength - 1) / tabLength + 1, '\t');
	}
}

//-----------------------------------------------------------------------------
// Name: addArgumentToArgv()
// Desc: Add 'additionalArg' to argv[] and increases argc by 1.
//-----------------------------------------------------------------------------
bool addArgumentToArgv(int &argc, char **&argv, char additionalArg[]) 
{
	// locals
	int  curArg;
	char **newArgv	= new char*[argc+1];

	// copy existing arguments
	for (curArg=0; curArg<argc; curArg++) {
		newArgv[curArg] = new char[strlen(argv[curArg])+1];
		strcpy(newArgv[curArg], argv[curArg]);
	}

	newArgv[curArg] = new char[strlen(additionalArg)+1];
	strcpy(newArgv[curArg], additionalArg);

	// delete only own arrays, not the origianl one passed. a problem arise when the orignal number of arguments is higher than one
	if (argc>1) delete [] argv;
	argv = newArgv;

	argc++;
	return true;
}

//-----------------------------------------------------------------------------
// Name: seperateFilePathAndEnding()
// Desc: _splitpath() does the same !!!
//-----------------------------------------------------------------------------
bool seperateFilePathAndEnding(string &filePath, string *path, string *file, string *ending) 
{
	// locals
	size_t			slashPos, dotPos;

    slashPos	= filePath.find_last_of('\\');
	dotPos		= filePath.find_last_of('.');

	if (path   != NULL) path->assign  (filePath.substr(0,			slashPos			));
	if (file   != NULL) file->assign  (filePath.substr(slashPos+1,	dotPos-slashPos-1	));
	if (ending != NULL) ending->assign(filePath.substr(dotPos+1,	string::npos		));

	return true;
}

//-----------------------------------------------------------------------------
// Name: seperateFileAndPath()
// Desc: Copies the path and file of a given 'filePath' into the passed parameters.
//       Returns false if the 'fileType' does not match.
//-----------------------------------------------------------------------------
bool seperateFileAndPath(const char filePath[], char *path, char *file) 
{
	// locals
	size_t		slashPos, dotPos;
	string		fullpath;

    fullpath.assign(filePath);
    slashPos	= fullpath.find_last_of('\\');
	dotPos		= fullpath.find_last_of('.');

    if (file != NULL) file[fullpath.copy(file, dotPos - slashPos - 1, slashPos+1)]	= '\0';
    if (path != NULL) path[fullpath.copy(path, slashPos, 0)]						= '\0';

	return true;
}

//-----------------------------------------------------------------------------
// Name: findProduct()
// Desc: Tries to find values for factor1 and factor2, such that factor1*factor2==product.
//-----------------------------------------------------------------------------
bool findProduct(unsigned int &factor1, unsigned int &factor2, unsigned int product, double factor1DividedByFactor2)
{
	// locals
	unsigned int my1, my2, i;

	factor1 = (unsigned int) sqrt(product*factor1DividedByFactor2);
	factor2 = (unsigned int) sqrt(product/factor1DividedByFactor2);
	i		= 0;

	do {

		// check quadratic area
		for (my1=factor1-i; my1<=factor1+i; my1++) {
			for (my2=factor2-i; my2<=factor2+i; my2++) {
				if (my1*my2 == product) {
					factor1 = my1;
					factor2 = my2;
					return true;
				}
			}
		}

		// increase search distance
		i++;

	} while (factor1-i > 0 && factor1+i < product && factor1-i > 0 && factor1+i < product);

	return false;
}

//-----------------------------------------------------------------------------
// Name: isThereAnyPeak()
// Desc: not implemented yet
//-----------------------------------------------------------------------------
bool isThereAnyPeak(unsigned int numValues, double *data, double threshold)
{
	// locals
	unsigned int	i			= 0;
	double		*	curValue	= data;
	double			leftMinimum	= *curValue;

	/* search left minimum
	do {
		i++;
		curValue++;
	} while (i<numValues && leftMinimum>*curValue);
	*/
	return true;
}

//-----------------------------------------------------------------------------
// Name: getRainbowColor()
// Desc: Diese Funktion nimmt einen Wert zwischen 0 und 1 an und gibt eine Farbe wieder.
//-----------------------------------------------------------------------------
void getRainbowColor(double valueFromZeroToOne, RGBTRIPLE &color)
{
	DWORD		AnzAbschnitte	= 6;
	double		AbschnittLaenge = 1.0 / AnzAbschnitte;

	#define		WERT_TO_X_UP(p)		(BYTE)((valueFromZeroToOne - ((p)-1) * AbschnittLaenge) / AbschnittLaenge * 255.0)
	#define		WERT_TO_X_DOWN(p)	(BYTE)(255 - ((valueFromZeroToOne - ((p)-1) * AbschnittLaenge) / AbschnittLaenge * 255.0))

	// Farben wiederholen bei großen Werten
	if (valueFromZeroToOne > 1)	{
		valueFromZeroToOne -= (int) valueFromZeroToOne;
	}

	// Rot -> Lila
	if (valueFromZeroToOne < 1 * AbschnittLaenge) {
		color.rgbtRed	= 255;
		color.rgbtGreen	= 0;
		color.rgbtBlue	= WERT_TO_X_UP(1); 
	// Lila -> Blau
	} else if (valueFromZeroToOne < 2 * AbschnittLaenge) {
		color.rgbtRed	= WERT_TO_X_DOWN(2);
		color.rgbtGreen	= 0;
		color.rgbtBlue	= 255;
	// Blau -> Türkis
	} else if (valueFromZeroToOne < 3 * AbschnittLaenge) {
		color.rgbtRed	= 0;
		color.rgbtGreen	= WERT_TO_X_UP(3);
		color.rgbtBlue	= 255;
	// Türkis -> Grün
	} else if (valueFromZeroToOne < 4 * AbschnittLaenge) {
		color.rgbtRed	= 0;
		color.rgbtGreen	= 255;
		color.rgbtBlue	= WERT_TO_X_DOWN(4); 
	// Grün -> Gelb
	} else if (valueFromZeroToOne < 5 * AbschnittLaenge) {
		color.rgbtRed	= WERT_TO_X_UP(5);
		color.rgbtGreen	= 255;
		color.rgbtBlue	= 0; 
	// Gelb -> Rot
	} else if (valueFromZeroToOne < 6 * AbschnittLaenge) {
		color.rgbtRed	= 255;
		color.rgbtGreen	= WERT_TO_X_DOWN(6);
		color.rgbtBlue	= 0; 
	}
}

//-----------------------------------------------------------------------------
// Name: rectClass::rectClass()
// Desc: 
//-----------------------------------------------------------------------------
rectClass::rectClass(double left, double right, double top, double bottom)
{
	this->left = left; this->right = right; this->bottom = bottom; this->top = top; this->type = typeRect;
}

//-----------------------------------------------------------------------------
// Name: rectClass::render()
// Desc: 
//-----------------------------------------------------------------------------
void rectClass::render(bool userFunc(int x, int y, void *pUser), unsigned int *numRenderedPixels, void* pUser)
{
	// locals
	int x, y;
	unsigned int numRenderedPixelsCounter = 0;

	for (x=(int)left; x<=(int)right; x++) {
		for (y=(int)top; y<=(int)bottom; y++) {
			if (userFunc(x, y, pUser) == false) break;
			numRenderedPixelsCounter++;
		}
	}

	if (numRenderedPixels != NULL) *numRenderedPixels = numRenderedPixelsCounter;
}

//-----------------------------------------------------------------------------
// Name: rectClass::getNextPixel()
// Desc: 
//-----------------------------------------------------------------------------
bool rectClass::getNextPixel(int &x, int &y, bool start)
{
	// first rendering call?
	if (start) {
		x = (int) left;
		y = (int) top;
	// when right border reached, than jump to next line
	} else if (x==(int)right) {
		// alread bottom line reached?
		if (y==(int)bottom) {
			return false;
		}
		x = (int) left;
		y++;
	} else {
		x++;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: interpolateValue()
// Desc: 
//-----------------------------------------------------------------------------
double	interpolateValue(double x1, double x2, double y1, double y2, double &x)
{
	return (x-x1) / (x2-x1) * (y2-y1) + y1;
}

//-----------------------------------------------------------------------------
// Name: polynom()
// Desc: 
//-----------------------------------------------------------------------------
double polynom(vector<double> *coefficients, double x)
{
	// parameters ok?
	if (coefficients == NULL) {
		return x;
	}

	// locals
	double y = 0;
	size_t curOrder, order = coefficients->size() - 1;

	for (curOrder=0; curOrder<=order; curOrder++) {
		y += integerPower(x, (unsigned int) curOrder) * ((*coefficients)[curOrder]);
	}
	return y;
}

//-----------------------------------------------------------------------------
// Name: integerPower()
// Desc: 
//-----------------------------------------------------------------------------
double integerPower(double basis, unsigned int exponent)
{
	double result = 1.0;
	while (exponent) {
		 result *= basis;
		 exponent--;
	}
	return result;
}