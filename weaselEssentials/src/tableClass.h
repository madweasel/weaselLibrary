/*********************************************************************\
	tableClass.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef TABLECLASS_H
#define TABLECLASS_H

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#pragma warning( disable : 4996 )

#include <cmath>
#include <fstream>
#include <cstring>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cstdio>
#include <list>
#include <vector>
#include <windows.h>
#include <sys/stat.h>

#ifdef TABLECLASS_BITMAP
    #include "BitmapLoader.h"
#endif

using namespace std;

/*** Macros **********************************************************/

#define TABLE_DATA(T,C,R) (T->data[C * T->numRows + R])

/*** Klassen *********************************************************/

template <class varType> class table
{
private:

	// delete functions
    void            deleteDataArray         ();
    void            deleteColumnNameArray   (bool deleteNames);
    void            deleteRowNameArray      (bool deleteNames);

    // sort functions
    static int     compareRowAscending      (const void * a, const void * b);
    static int     compareRowDescending     (const void * a, const void * b);

	// merge helper functions
	bool			areKeyValuesLessOrEqual	(vector<unsigned int> &keyColumns, table<varType> &curTable, vector<varType> &keyValuesToCompare, unsigned int curRow, double delta);

public:
    // variables
	unsigned int	numDigitsOnOutput;
    unsigned int    numColumns;
    unsigned int    numRows;   
    varType     *   data;                   // access is data[curColumn * numRows + curRow]
    char       **   columnName;
    char       **   rowName;
	bool			stopOnError;

    // functions
    table           ();
    table           (table *t);
	table           (unsigned int nCols, unsigned int nRows, varType  defaultValue);
    table           (unsigned int nCols, unsigned int nRows, varType *initialDataArray);
    table           (unsigned int nCols, unsigned int nRows);
    table           (string str, bool firstLineAreColumnNames);
    table           (unsigned int numData, varType theData[], unsigned int numBins, varType minimumValue, varType binSize, bool normalize);
    ~table          ();

    // input/output
    bool            readDataFromBitmapFile  (const char * fileName, varType convertFromARGB(unsigned int pixelColor));
    bool            readDataFromRptFile     (const char * fileName);
    bool            readDataFromBinaryFile  (const char * fileName);
    bool            readDataFromAsciiFile   (const char * fileName);
    bool            readDataFromAsciiFile   (const char * fileName, const char *decimalSeparator, const char *columnSeparator, const char *lineComment);
    bool            writeDataToBinaryFile   (const char * fileName);
    bool            writeDataToAsciiFile    (const char * fileName, char *lineComment, bool printColumnNames, bool printRowNames, unsigned int blankLineEvery, char *columnSeparator, bool printColumnIndices);
    bool            convertToBinAndOpenRpt  (const char * fileName);
    bool            printToConsole          (bool printColumnNames = true, bool printRowNames = true);

    // static functions
    static bool     fileExists              (const char * fileName);
    static void     transposeArray          (unsigned int numRows, unsigned int numColumns, varType* data, varType* transposedData);
    static void     swapColumns             (unsigned int numRows, unsigned int numColumns, unsigned int colA, unsigned int colB, varType* data, varType* dataWithSwappedCol);
    
    // calculations
	varType         calcMaxValue            ();
    varType         calcMinValue            ();
	varType         calcLowerQuartile       ();
    varType         calcUpperQuartile       ();
	varType         calcMedian              ();
	varType         calcSum		            ();

    varType         calcMaxValue            (char name[]);
    varType         calcMinValue            (char name[]);
	varType         calcMinValue            (char name[], varType considerValuesGreaterThan);
    varType         calcLowerQuartile       (char name[]);
    varType         calcUpperQuartile       (char name[]);
    varType         calcMedian              (char name[]);
    varType         calcAvgTop              (char name[], varType threshold);
    varType         calcAvgFlop             (char name[], varType threshold);
    
    varType         calcSum		            (unsigned int columnIndex);
    varType         calcAverage             (unsigned int columnIndex);
    varType         calcMaxValue            (unsigned int columnIndex);
    varType         calcMaxValue            (unsigned int columnIndex, unsigned int &rowWithMaxValue);
    varType         calcMaxValue            (unsigned int columnIndex, unsigned int &rowWithMaxValue, unsigned int fromRow, unsigned int toRow);
    varType         calcMinValue            (unsigned int columnIndex);
    varType         calcMinValue            (unsigned int columnIndex, unsigned int &rowWithMinValue);
    varType         calcMinValue            (unsigned int columnIndex, unsigned int &rowWithMinValue, unsigned int fromRow, unsigned int toRow);
	varType         calcMinValue            (unsigned int columnIndex, varType considerValuesGreaterThan);

    varType         calcLowerQuartile       (unsigned int columnIndex);
    varType         calcUpperQuartile       (unsigned int columnIndex);
    varType         calcMedian              (unsigned int columnIndex);
	varType         calcMedianStepSize      (unsigned int columnIndex);
    varType         calcAvgTop              (unsigned int columnIndex, varType threshold);
    varType         calcAvgFlop             (unsigned int columnIndex, varType threshold);

	varType         getValue			    (unsigned int columnIndex, unsigned int row);
	varType         getValueOfSortedList    (unsigned int pos);
    varType         getValueOfSortedList    (unsigned int columnIndex, unsigned int pos);
    varType         get_X_valueAt_Y_value   (unsigned int columnIndex_X, unsigned int columnIndex_Y, varType valueY, unsigned int placement, varType skipX);



    // names
    bool            setColumnName           (unsigned int columnIndex, const char *name);
    bool            setRowName              (unsigned int rowIndex,    const char *name);
    void            deleteAllColumnNames    ();

    // setter
    bool            setDatum                (varType value);
    bool            setDatum                (unsigned int columnIndex,  unsigned int rowIndex, varType &value);
	bool			setRow					(unsigned int columnOffset, unsigned int rowIndex, vector<varType> &dataToInsert);
	bool			setColumn				(unsigned int columnIndex, varType value);
	void			setStopOnError			(bool stopOnError);

    // getter
	unsigned int	getNextRowWithValueAbove(unsigned int columnIndex, unsigned int rowOffset, varType threshold, bool belowInsteadOfAbove);
	unsigned int    getRowIndex				(char name[]);
    unsigned int    getColumnIndex          (const char name[]);
    varType     *   getColumn               (char name[]);
	varType     *   getColumn               (unsigned int columnIndex);

    // operations
    void            addColumn               (unsigned int &newColumnIndex, varType value);
    void            addColumn               (unsigned int &newColumnIndex, varType userFunction(varType a), unsigned int columnIndexA);
    void            addColumn               (unsigned int &newColumnIndex, varType userFunction(varType a, varType b), unsigned int columnIndexA, unsigned int columnIndexB);
    void            addColumn               (unsigned int &newColumnIndex, varType userFunction(varType a, varType b, varType c), unsigned int columnIndexA, unsigned int columnIndexB, unsigned int columnIndexC);
    void            addColumn               (char name[], varType value);
    void            addColumn               (char name[], varType userFunction(varType a), unsigned int columnIndexA);
    void            addColumn               (char name[], varType userFunction(varType a, varType b), unsigned int columnIndexA, unsigned int columnIndexB);
    void            addColumn               (char name[], varType userFunction(varType a, varType b, varType c), unsigned int columnIndexA, unsigned int columnIndexB, unsigned int columnIndexC);
    bool            copyRow		            (unsigned int fromRowIndex, unsigned int toRowIndex);
	bool			copyRow					(unsigned int rowIndexInTargetTable, unsigned int rowIndexInSourceTable, table &fromTable);
	bool            copyColumn              (char name[], table &anotherTable);
	bool            copyColumn              (unsigned int colIndexInTargetTable, unsigned int colIndexInSourceTable, table &sourceTable);
    bool            copyColumnNames         (table &anotherTable);
    bool            filterRows              (unsigned int numRowsToKeep, unsigned int rowsToKeep[]);
    bool            filterRows              (bool keepRow(varType a),            char columnNameA[]);
    bool            filterRows              (bool keepRow(varType a, varType b), char columnNameA[], char columnNameB[]);
    void            deleteUnnamedColumns    ();
    bool            sortTable               (unsigned int columnIndex, bool ascending);
    bool            sortTable               (vector <unsigned int> &columnIndeces, vector<bool> &ascending, double delta);
	bool			sortKeyTable			(unsigned int fromCol, unsigned int toCol, vector <unsigned int> &rowIndeces, vector<bool> &ascending, double delta);
    bool            insertTable             (table *t, unsigned int toCol, unsigned int toRow);
    void            redim                   (unsigned int nCols, unsigned int nRows);
    void            redim                   (unsigned int nCols, unsigned int nRows, varType defaultValue);
    void            redim                   (unsigned int nCols, unsigned int nRows, varType *theData);
    void            destroy                 ();
	bool			smoothData				(unsigned int numPointsToAvgOver, unsigned int columnIndex, varType *tmpColumn, unsigned int fromRow, unsigned int toRow);
	bool			mergeTables				(vector<unsigned int> &keyColumns, vector<table<varType>> &tables, double delta, varType maxValue);
	bool			concatenate				(vector<table<varType>> &tables);
	bool			areTheNextvaluesAboveThreshold(unsigned int columnIndex, unsigned int rowOffset, unsigned int numNextValues, unsigned int numOutliers, varType threshold, bool belowInsteadOfAbove);

};

//-----------------------------------------------------------------------------
// Name: table()
// Desc: table class constructor
//-----------------------------------------------------------------------------
template <class varType> table<varType>::table()
{
    columnName			= NULL;
    rowName				= NULL;
    data				= NULL;
    numRows				= 0;
    numColumns			= 0;
	numDigitsOnOutput	= 7;
	stopOnError			= true;
}

//-----------------------------------------------------------------------------
// Name: table()
// Desc: table class constructor
//-----------------------------------------------------------------------------
template <class varType> table<varType>::table(unsigned int nCols, unsigned int nRows)
{
    columnName			= new char*[nCols];   memset(columnName, NULL, sizeof(char*) * nCols);
    rowName				= new char*[nRows];   memset(rowName,    NULL, sizeof(char*) * nRows);
    data				= new varType[nCols*nRows];
    numRows				= nRows;
    numColumns			= nCols;
	numDigitsOnOutput	= 7;
	stopOnError			= true;
}

//-----------------------------------------------------------------------------
// Name: table()
// Desc: table class constructor
//-----------------------------------------------------------------------------
template <class varType> table<varType>::table(unsigned int nCols, unsigned int nRows, varType defaultValue)
{
    columnName			= new char*[nCols];   memset(columnName, 0, sizeof(char*) * nCols);
    rowName				= new char*[nRows];   memset(rowName,    0, sizeof(char*) * nRows);
    data				= new varType[nCols*nRows];
    numRows				= nRows;
    numColumns			= nCols;
	numDigitsOnOutput	= 7;
	stopOnError			= true;

	for (unsigned int curDataPoint=0; curDataPoint<numRows*numColumns; curDataPoint++) {
		data[curDataPoint] = defaultValue;
	}
}

//-----------------------------------------------------------------------------
// Name: table()
// Desc: table class constructor
//-----------------------------------------------------------------------------
template <class varType> table<varType>::table(unsigned int nCols, unsigned int nRows, varType *initialDataArray)
{
    columnName			= new char*[nCols];   memset(columnName, NULL, sizeof(char*) * nCols);
    rowName				= new char*[nRows];   memset(rowName,    NULL, sizeof(char*) * nRows);
    data				= new varType[nCols*nRows];
    numRows				= nRows;
    numColumns			= nCols;
    numDigitsOnOutput	= 7;
	stopOnError			= true;
	if (initialDataArray != NULL) memcpy(data, initialDataArray, sizeof(varType) * nCols * nRows);
}

//-----------------------------------------------------------------------------
// Name: table()
// Desc: table class constructor
//-----------------------------------------------------------------------------
template <class varType> table<varType>::table(string str, bool firstLineAreColumnNames)
{
    // locals
    list<varType>           listData;
    // list<varType>::iterator itr;
    unsigned int            curPos, curColumn, curRow;
    varType                 tmpVar;
    string                  line;

    // replace commas through space
    curPos = str.find(","); 
    while (curPos != string::npos) {
       str.replace(curPos, 1, " ");
       curPos = str.find(",", curPos + 1);
    } 
    istringstream           isStr (str, istringstream::in);

    // determine number of columns and read first line
    numColumns  = 0;
    curPos      = 0;
    getline(isStr, line);
    do {
        curPos = line.find_first_not_of("\t ", curPos);     if (curPos == -1) break;
        curPos = line.find_first_of    ("\t ", curPos);
        numColumns++;
    } while (curPos != -1);
    
    // read data and determine number of rows
    curPos = 0;
    isStr.seekg (0, ios::beg);
    while (!isStr.eof()) {
        curColumn = curPos % numColumns;
        curRow    = curPos / numColumns;
        isStr >> tmpVar;

        // if (isStr.eof()) break;

        listData.push_back(tmpVar);
        curPos++;

        if (isStr.eof()) curRow = curPos / numColumns;
    };
    numRows				= curRow;
	numDigitsOnOutput	= 7;
	stopOnError			= true;

    // allocate mem
    data        = new varType[numRows*numColumns];
    columnName  = new char*[numColumns];   memset(columnName, NULL, sizeof(char*) * numColumns);
    rowName     = new char*[numRows];      memset(rowName,    NULL, sizeof(char*) * numRows);

    // copy columnNames
    if (firstLineAreColumnNames) {
        /* Not implemented yet
        for (curPos=0, curColumn=0; curColumn<numColumns; curColumn++) {
            begPos = lineWithColumnNames.find_first_not_of("\t ", curPos);
            curPos = lineWithColumnNames.find("  ", begPos);        
            if (curPos == -1) curPos = line.length();
            columnName[curColumn] = new char[curPos-begPos+1];
            lineWithColumnNames.copy(columnName[curColumn], curPos-begPos, begPos);
            columnName[curColumn][curPos-begPos] = '\0';
        }*/
    }

    // copy data
    curPos=0;
    for (auto& curData : listData) {
        curColumn                       = curPos % numColumns;
        curRow                          = curPos / numColumns;

        // handle both cases: file ends with and without endl
        if (curPos >= numColumns * numRows) break;

        data[curColumn*numRows+curRow]  = curData;
        curPos++;
    }

    listData.clear();
}

//-----------------------------------------------------------------------------
// Name: ~table()
// Desc: table class destructor
//-----------------------------------------------------------------------------
template <class varType> table<varType>::~table()
{
    destroy();
}

//-----------------------------------------------------------------------------
// Name: deleteDataArray()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> void table<varType>::deleteDataArray()
{
    // delete data
    if (data != NULL) delete [] data;
    data = NULL;
}

//-----------------------------------------------------------------------------
// Name: deleteColumnNameArray()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> void table<varType>::deleteColumnNameArray(bool deleteNames)
{
    // locals
    unsigned int i;

    // delete names
    if (deleteNames) {
        for (i=0; i<numColumns; i++) {
            if (columnName[i] != NULL) {
                delete [] columnName[i];
                columnName[i] = NULL;
            }
        }
    }

    // delete columnName
    if (columnName != NULL) {
        delete [] columnName;
        columnName = NULL;
    }
}

//-----------------------------------------------------------------------------
// Name: deleteRowNameArray()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> void table<varType>::deleteRowNameArray(bool deleteNames)
{
    // locals
    unsigned int i;

    // delete names
    if (deleteNames) {
        for (i=0; i<numRows; i++) {
            if (rowName[i] != NULL) {
                delete [] rowName[i];
                rowName[i] = NULL;
            }
        }
    }

    // delete rowName
    if (rowName != NULL) {
        delete [] rowName;
        rowName = NULL;
    }
}

//-----------------------------------------------------------------------------
// Name: destroy()
// Desc: This function deletes all rows, columns, row names and column names.
//-----------------------------------------------------------------------------
template <class varType> void table<varType>::destroy()
{
    deleteDataArray();
    deleteRowNameArray(true);
    deleteColumnNameArray(true);

    numRows     = 0;
    numColumns  = 0;
}

//-----------------------------------------------------------------------------
// Name: fileExists()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::fileExists(const char *fileName) 
{ 
  struct stat stFileInfo; 
  bool   blnReturn; 
  int    intStat; 

  // Attempt to get the file attributes 
  intStat = stat(fileName, &stFileInfo); 
  
  if(intStat == 0) { 
    // We were able to get the file attributes 
    // so the file obviously exists. 
    blnReturn = true; 
  } else { 
    // We were not able to get the file attributes. 
    // This may mean that we don't have permission to 
    // access the folder which contains this file. If you 
    // need to do that level of checking, lookup the 
    // return values of stat which will give you 
    // more details on why stat failed. 
    blnReturn = false; 
  } 
   
  return blnReturn; 
}

//-----------------------------------------------------------------------------
// Name: convertToBinAndOpenRpt()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::printToConsole(bool printColumnNames, bool printRowNames)
{
	// locals
    unsigned int            curRow, curColumn;
    unsigned int            *colWidth           = new unsigned int[numColumns];
    unsigned int            firstColWidth       = 0;
    unsigned int            minColWidth         = max((int) 16, (int) (2 + numDigitsOnOutput + 5));
    unsigned int            lineCommWidth       = 0;
	char                    columnSeparator[]   = "\t";
    bool                    printColumnIndices  = false;

    // prepare output stream
    cout << setfill(' ');
    cout << setprecision(numDigitsOnOutput);
    // cout << setf(ios::left,        ios::adjustfield);
    // cout << setf(ios::scientific , ios::floatfield);

    // calc maximum row name length
    if (printRowNames) {
        for (curRow=0; curRow<numRows; curRow++) {
            if (rowName[curRow] != NULL) {
                if (firstColWidth < strlen(rowName[curRow]) + 1) {
                    firstColWidth = (unsigned int) strlen(rowName[curRow]) + 1;
                }
            }
        }
    }

    // print column names
    if (printColumnNames) {

        // when row names are printed, than fill with spaces
        if (printRowNames) {
            cout << setw(firstColWidth) << " ";
        }
        
        // print column names
        for (curColumn=0; curColumn<numColumns; curColumn++) {
            if (columnName[curColumn] != NULL) {
                colWidth[curColumn] = (unsigned int) strlen(columnName[curColumn]) + 1; 
                if (colWidth[curColumn] < minColWidth) colWidth[curColumn] = minColWidth;
                cout << setw(colWidth[curColumn]) << columnName[curColumn] << columnSeparator ;
            } else {
                colWidth[curColumn] = minColWidth;
                cout << setw(colWidth[curColumn]) << columnSeparator << " ";
            }
        }
    // if no column names are printed, so set minimum column width for each column
    } else {
        for (curColumn=0; curColumn<numColumns; curColumn++) {
            colWidth[curColumn] = minColWidth;
        }
    }
    cout << "\n";

    // print printColumnIndices
    if (printColumnIndices) {
        
        // when row names are printed, than fill with spaces
        if (printRowNames) {
            cout << setw(firstColWidth) << " ";
        }

        // print column names
        for (curColumn=0; curColumn<numColumns; curColumn++) {
            cout << setw(colWidth[curColumn]) << (curColumn + 1) << columnSeparator;
        }
        cout << "\n";
    }

    // print rows
    for (curRow=0; curRow<numRows; curRow++) {
        
        // print row name
        if (printRowNames) {
            if (rowName[curRow] != NULL) {
                cout << setw(lineCommWidth + firstColWidth) << rowName[curRow];
            } else {
                cout << setw(lineCommWidth + firstColWidth) << " ";
            }
        }

        // print data
        for (curColumn=0; curColumn<numColumns; curColumn++) {
            cout << setw(colWidth[curColumn] + ((curColumn>0||printRowNames)?0:lineCommWidth));
            if (data[curColumn * numRows + curRow] != data[curColumn * numRows + curRow]) {
                cout << "-";
            } else {
                cout << data[curColumn * numRows + curRow];
            }
            if (columnSeparator != NULL) cout << columnSeparator;
        }
        cout << "\n";
    }

    // free mem and leave
    delete [] colWidth;
    return true;
}

//-----------------------------------------------------------------------------
// Name: convertToBinAndOpenRpt()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::convertToBinAndOpenRpt(const char *fileName)
{
    // locals
    char str_rpt[MAX_PATH];
    char str_bin[MAX_PATH];

    sprintf(str_rpt, "%s.rpt", fileName);
    sprintf(str_bin, "%s.bin", fileName);

    // try to open bin-file
    if (fileExists(str_bin)) {
        return readDataFromBinaryFile(str_bin);

    // try to open rpt
    } else {
        
        if (fileExists(str_rpt)) {
            if (!readDataFromRptFile(str_rpt)) {
                return false;
            }
            return writeDataToBinaryFile(str_bin);
        } else {
            return false;
        }
    }
}

//-----------------------------------------------------------------------------
// Name: writeDataToBinaryFile()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::writeDataToBinaryFile(const char * fileName)
{
    // locals
    HANDLE       hFile; 
    DWORD        dwWritten;
    char         nullChar   = '\0';
    unsigned int nameLen;
    unsigned int i;
    
    // open file
    hFile = CreateFileA(fileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 

    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    // write data
    WriteFile(hFile, &numRows,      sizeof(unsigned int),                   &dwWritten, NULL); 
    WriteFile(hFile, &numColumns,   sizeof(unsigned int),                   &dwWritten, NULL); 

    for (i=0; i<numRows; i++) {
        nameLen = (rowName[i] != NULL) ? strlen(rowName[i]) : 0;
        WriteFile(hFile, &nameLen,  sizeof(unsigned int),                   &dwWritten, NULL); 
        if (nameLen) {
            WriteFile(hFile, rowName[i],    nameLen,                        &dwWritten, NULL); 
            WriteFile(hFile, &nullChar,     1,                              &dwWritten, NULL); 
        }
    }

    for (i=0; i<numColumns; i++) {
        nameLen = (columnName[i] != NULL) ? strlen(columnName[i]) : 0;
        WriteFile(hFile, &nameLen,  sizeof(unsigned int),                   &dwWritten, NULL); 
        if (nameLen) {
            WriteFile(hFile, columnName[i], nameLen,                        &dwWritten, NULL); 
            WriteFile(hFile, &nullChar,     1,                              &dwWritten, NULL); 
        }
    }

    WriteFile(hFile, data,          sizeof(varType) * numRows * numColumns, &dwWritten, NULL); 

    // close file
    CloseHandle(hFile);

    return true;
}

//-----------------------------------------------------------------------------
// Name: readDataFromBitmapFile()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::readDataFromBitmapFile(const char *fileName, varType convertFromARGB(unsigned int pixelColor))
{
#ifdef TABLECLASS_BITMAP

    // locals
    unsigned int    height, width, curRow, curColumn;
    unsigned int    *bmpData        = LoadBitmapARGB(fileName, &width, &height);

    // opened file successfully?
    if (bmpData == NULL) {
        return false;
    }

    destroy();

    // read header
    numRows     = height;
    numColumns  = width;

    // allocate mem
    data        = new varType[numRows * numColumns];
    columnName  = new char*[numColumns];   memset(columnName, NULL, sizeof(char*) * numColumns);
    rowName     = new char*[numRows];      memset(rowName,    NULL, sizeof(char*) * numRows);

    // copy data
    for (curRow=0; curRow<numRows; curRow++) {
        for (curColumn=0; curColumn<numColumns; curColumn++) {
            data[curColumn * numRows + curRow] = convertFromARGB(bmpData[curColumn * numRows + curRow]);
        }
    }
    
    // free mem
    delete [] bmpData;

    return true;
#else
    return false;
#endif
}

//-----------------------------------------------------------------------------
// Name: readDataFromBinaryFile()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::readDataFromBinaryFile(const char *fileName)
{
    // locals
    HANDLE          hFile; 
    DWORD           dwRead; 
    unsigned int    nameLen;
    unsigned int    i;
      
    // open file
    hFile = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 

    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    destroy();

    // read header
    ReadFile(hFile, &numRows,      sizeof(unsigned int),                   &dwRead, NULL); 
    ReadFile(hFile, &numColumns,   sizeof(unsigned int),                   &dwRead, NULL); 

    // allocate mem
    data        = new varType[numRows * numColumns];
    columnName  = new char*[numColumns];   memset(columnName, NULL, sizeof(char*) * numColumns);
    rowName     = new char*[numRows];      memset(rowName,    NULL, sizeof(char*) * numRows);

    // read names    
    for (i=0; i<numRows; i++) {
        ReadFile(hFile, &nameLen,  sizeof(unsigned int),                   &dwRead, NULL);      
        if (nameLen) {
            rowName[i] = new char[nameLen+1];
            ReadFile(hFile, rowName[i],      nameLen+1,                    &dwRead, NULL);      
        }
    }
    
    for (i=0; i<numColumns; i++) {
        ReadFile(hFile, &nameLen,  sizeof(unsigned int),                   &dwRead, NULL);      
        if (nameLen) {
            columnName[i] = new char[nameLen+1];
            ReadFile(hFile, columnName[i],      nameLen+1,                 &dwRead, NULL);      
        }
    }

    // read data
    ReadFile(hFile, data,          sizeof(varType) * numRows * numColumns, &dwRead, NULL); 

    // close file
    CloseHandle(hFile);

    return true;
}

//-----------------------------------------------------------------------------
// Name: writeDataToAsciiFile()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::writeDataToAsciiFile(const char *fileName, char *lineComment, bool printColumnNames, bool printRowNames, unsigned int blankLineEvery, char *columnSeparator, bool printColumnIndices)
{
	// locals
    ofstream				myFile(fileName);
    unsigned int            curRow, curColumn;
    unsigned int            *colWidth       = new unsigned int[numColumns];
    unsigned int            firstColWidth   = 0;
    unsigned int            minColWidth     = std::max((int) 16, (int) (2 + numDigitsOnOutput + 5));
    unsigned int            lineCommWidth   = 0;

	if (blankLineEvery  == 0)		blankLineEvery  = 4000000000;
	if (columnSeparator == NULL)	columnSeparator = "\t";

    // open file
    if (myFile.is_open()) {

        // prepare output stream
        myFile.fill(' ');
        myFile.precision(numDigitsOnOutput);
        myFile.setf(ios::left,        ios::adjustfield);
        myFile.setf(ios::scientific , ios::floatfield);

        // calc maximum row name length
        if (printRowNames) {
            for (curRow=0; curRow<numRows; curRow++) {
                if (rowName[curRow] != NULL) {
                    if (firstColWidth < strlen(rowName[curRow]) + 1) {
                        firstColWidth = (unsigned int) strlen(rowName[curRow]) + 1;
                    }
                }
            }
        }

        // print column names
        if (printColumnNames) {

            // write line comment
            if (lineComment != NULL) {
                myFile << lineComment << " ";
                lineCommWidth = (unsigned int) strlen(lineComment) + 1;
            }

            // when row names are printed, than fill with spaces
            if (printRowNames) {
                myFile << setw(firstColWidth) << " ";
            }
            
            // print column names
            for (curColumn=0; curColumn<numColumns; curColumn++) {
                if (columnName[curColumn] != NULL) {
                    colWidth[curColumn] = (unsigned int) strlen(columnName[curColumn]) + 1; 
                    if (colWidth[curColumn] < minColWidth) colWidth[curColumn] = minColWidth;
                    myFile << setw(colWidth[curColumn]) << columnName[curColumn] << columnSeparator ;
               } else {
                    colWidth[curColumn] = minColWidth;
                    myFile << setw(colWidth[curColumn]) << columnSeparator << " ";
               }
            }
        // if no column names are printed, so set minimum column width for each column
        } else {
            for (curColumn=0; curColumn<numColumns; curColumn++) {
                colWidth[curColumn] = minColWidth;
            }
        }
        myFile << "\n";

		// print printColumnIndices
		if (printColumnIndices) {
			
			// write line comment
            if (lineComment != NULL) {
                myFile << lineComment << " ";
                lineCommWidth = (unsigned int) strlen(lineComment) + 1;
            }

			// when row names are printed, than fill with spaces
            if (printRowNames) {
                myFile << setw(firstColWidth) << " ";
            }

			// print column names
            for (curColumn=0; curColumn<numColumns; curColumn++) {
                myFile << setw(colWidth[curColumn]) << (curColumn + 1) << columnSeparator;
            }
			myFile << "\n";
		}

        // print rows
        for (curRow=0; curRow<numRows; curRow++) {
            
            // print row name
            if (printRowNames) {
                if (rowName[curRow] != NULL) {
                    myFile << setw(lineCommWidth + firstColWidth) << rowName[curRow];
                } else {
                    myFile << setw(lineCommWidth + firstColWidth) << " ";
                }
            }

            // print data
            for (curColumn=0; curColumn<numColumns; curColumn++) {
                myFile << setw(colWidth[curColumn] + ((curColumn>0||printRowNames)?0:lineCommWidth));
                if (data[curColumn * numRows + curRow] != data[curColumn * numRows + curRow]) {
                    myFile << "-";
                } else {
                    myFile << data[curColumn * numRows + curRow];
                }
				if (columnSeparator != NULL) myFile << columnSeparator;
            }
            if ((curRow+1) % blankLineEvery == 0) myFile << "\n";
			myFile << "\n";
        }
        
        // close file
        myFile << endl;
		myFile.close();

    } else {
        return false;
    }

    // free mem and leave
    delete [] colWidth;
    return true;
}
      
//-----------------------------------------------------------------------------
// Name: readDataFromRptFile()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::readDataFromRptFile(const char *fileName)
{
	// locals
    ifstream				dataFile (fileName);
    list<varType>           listData;
    // list<varType>::iterator itr;
    varType                 tmpVar;
	string					line;
    string                  lineBefore;
    string                  lineWithColumnNames;
    unsigned int            curPos, curRow, curColumn;
    bool                    success = false; 
    int                     begPos;
    char                    tmpChar = 0;
    char                    tmpString[100];
          
    // delete rows
    destroy();

	if (dataFile.is_open()) {
        
        // jump after ----- line 
        curRow = 0;
        while (true) {
            if (curRow>1) lineWithColumnNames   = lineBefore;
            if (curRow>0) lineBefore            = line;
            getline(dataFile, line);
            if (line.find("--------------") != -1) break;
            if (dataFile.eof()) goto closeFileAndLeave;
            curRow++;
        }

        // determine number of columns and read first line
        numColumns  = 0;
        curPos      = 0;
        begPos      = dataFile.tellg();
        getline(dataFile, line);
        do {
            curPos = line.find_first_not_of("\t ", curPos);     if (curPos == -1) break;
            curPos = line.find_first_of    ("\t ", curPos);
            numColumns++;
        } while (curPos != -1);
        dataFile.seekg(begPos, ios::beg);

        // read data and determine number of rows
        curPos = 0;
        while (!dataFile.eof()) {
            curColumn = curPos % numColumns;
            curRow    = curPos / numColumns;

            // dataFile >> tmpVar;
            dataFile >> tmpString;
            tmpVar = atof(tmpString);

            if (dataFile.eof()) break;
 
            if (dataFile.fail()) {
                // ???
            }

            if (dataFile.bad()) {
                // ???
            }

            listData.push_back(tmpVar);
            curPos++;
        };
        numRows = curRow;

        // allocate mem
        data        = new varType[numRows*numColumns];
        columnName  = new char*[numColumns];   memset(columnName, NULL, sizeof(char*) * numColumns);
        rowName     = new char*[numRows];      memset(rowName,    NULL, sizeof(char*) * numRows);
        
        // copy columnNames
        if (lineWithColumnNames.size()) {
            for (curPos=0, curColumn=0; curColumn<numColumns; curColumn++) {
                begPos = lineWithColumnNames.find_first_not_of("\t ", curPos);
                curPos = lineWithColumnNames.find_first_of    ("\t ", begPos);        
                if (curPos > lineWithColumnNames.length()) curPos = lineWithColumnNames.length();
                columnName[curColumn] = new char[curPos-begPos+1];
                lineWithColumnNames.copy(columnName[curColumn], curPos-begPos, begPos);
                columnName[curColumn][curPos-begPos] = '\0';
            }
        }

        // copy data
        curPos=0;
        for (auto& curData : listData) {
            curColumn                       = curPos % numColumns;
            curRow                          = curPos / numColumns;
            data[curColumn*numRows+curRow]  = curData;
            curPos++;
        }

        listData.clear();
        success = true;

closeFileAndLeave:            
        // close file
		dataFile.close();
	}

    return success;
}

//-----------------------------------------------------------------------------
// Name: readDataFromRptFile()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::readDataFromAsciiFile(const char *fileName)
{
    return readDataFromAsciiFile(fileName, NULL, NULL, NULL);
}

//-----------------------------------------------------------------------------
// Name: readDataFromRptFile()
// Desc: Default lineComment is '#', default decimalSeparator is '.', default columnSeparator is " \t".
//       These parameters must be a terminated string.
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::readDataFromAsciiFile(const char * fileName, const char *decimalSeparator, const char *columnSeparator, const char *lineComment)
{
	// locals
    ifstream				dataFile (fileName);
    list<varType>           listData;
	list<string>			listRowNames;
    varType                 tmpVar;
	string					line;
    string                  lineBefore;
    string                  lineWithColumnNames;
    unsigned int            curPos, curRow, curColumn;
	bool					firstCharacterIsNonNumeric;
	bool					containRowNames;
    bool                    success = false; 
    int                     begPos;
    char                    tmpChar, tmpChar2;
    char					a[1000];
            
    // Set key chars
    char myLineComment      = (lineComment     !=NULL) ? lineComment[0]         : '#';
    char *myColumnSeparator = (columnSeparator !=NULL) ? columnSeparator        : "\t ";
    char myDecimalSeparator = (decimalSeparator!=NULL) ? decimalSeparator[0]    : '.';

    // delete rows
    destroy();

	if (dataFile.is_open()) {

        // skip comment lines and read first line
        do {
            begPos      = (int) dataFile.tellg();
            getline(dataFile, line);
            if (line[0] != myLineComment) break;
        } while (!dataFile.eof());

		// determine number of columns
        numColumns					= 0;
        curPos						= 0;
		containRowNames				= false;
		firstCharacterIsNonNumeric	= !(line.find_first_of("+-0123456789.", 0) == 0);
        do {
            curPos = line.find_first_not_of(myColumnSeparator, curPos);     if (curPos == -1) break;

			// if non-numeric expression then either row names or column names
			if (numColumns==1 && firstCharacterIsNonNumeric) {
				int v = line.substr(curPos, 1).find_first_of("+-0123456789.", 0);
				if (v == string::npos || v != 0) {
					lineWithColumnNames = line;
					begPos				= (int) dataFile.tellg();
					numColumns			= 0;
					curPos				= 0;
					getline(dataFile, line);
					continue;
				} else {
					containRowNames = true;
				}
			}

			curPos = line.find_first_of    (myColumnSeparator, curPos);
            numColumns++;

        } while (curPos != -1);
        dataFile.seekg(begPos, ios::beg);
		if (containRowNames) numColumns--;
		
        // read data and determine number of rows
        curPos = 0;
        while (!dataFile.eof()) {

            // skip comment lines
            do {
                dataFile.get(tmpChar2);
                if (tmpChar2 == 0x13 || tmpChar2 == 0x10) {
                    dataFile.get(tmpChar);
                } else {
                    tmpChar = tmpChar2;
                }
                if (tmpChar != myLineComment) {
                    if (tmpChar != myColumnSeparator[0]) {
                        dataFile.unget();
                    }
                    break;
                } else {
                    getline(dataFile, line);
                }
            } while (!dataFile.eof());

            curColumn = curPos % numColumns;
            curRow    = curPos / numColumns;
            
            // read data
            dataFile >> a;
            if (a[0] == myLineComment) {
                getline(dataFile, line);
                if (dataFile.eof()) break;   
            } else {

				// row name?
				if (a[0]>=65&&a[0]<=122) {
					listRowNames.push_back(a);
				} else {
					tmpVar = (varType) atof(a);

					// end of file reached?
					if (dataFile.eof()) break;   

					// error occured?
					if (dataFile.bad() || dataFile.fail()) {
						tmpVar = 0;
						dataFile.clear();
					}

					listData.push_back(tmpVar);
					curPos++;
				}
            }
        };
        numRows = curRow;

        // allocate mem
        data        = new varType[numRows*numColumns];
        columnName  = new char*[numColumns];   memset(columnName, NULL, sizeof(char*) * numColumns);
        rowName     = new char*[numRows];      memset(rowName,    NULL, sizeof(char*) * numRows);
       
		// copy rowNames
        if (listRowNames.size()) {
            for (curRow=0; curRow<numRows; curRow++) {
				int myLen = strlen(listRowNames.front().c_str());
                rowName[curRow] = new char[myLen+1];
                listRowNames.front().copy(rowName[curRow], myLen, 0);
                rowName[curRow][myLen] = '\0';
				listRowNames.pop_front();
            }
        }     

        // copy columnNames
        if (lineWithColumnNames.size()) {
			for (curPos=0, curColumn=0; curColumn<numColumns; curColumn++) {
                begPos = lineWithColumnNames.find_first_not_of("\t ", curPos);
                curPos = lineWithColumnNames.find_first_of    ("\t ", begPos);        
                if (curPos > lineWithColumnNames.length()) curPos = lineWithColumnNames.length();
                columnName[curColumn] = new char[curPos-begPos+1];
                lineWithColumnNames.copy(columnName[curColumn], curPos-begPos, begPos);
                columnName[curColumn][curPos-begPos] = '\0';
            }
        }

        // copy data
        curPos=0;
        for (auto& curData : listData) {
            curColumn                       = curPos % numColumns;
            curRow                          = curPos / numColumns;
            data[curColumn*numRows+curRow]  = curData;
            curPos++;
        }

		listRowNames.clear();
        listData.clear();
        success = true;

        // close file
		dataFile.close();
	}

    return success;
}

//-----------------------------------------------------------------------------
// Name: addColumn()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> void table<varType>::addColumn(unsigned int &newColumnIndex, varType userFunction(varType a), unsigned int columnIndexA)
{
    // parameters ok?
    if (!numColumns || !numRows) return;

    // locals
    unsigned int            curRow;
    varType                *colA, *colNew;
    varType                *newData         = new varType[(numColumns + 1) * numRows];
    char                  **newColumnName   = new char*  [(numColumns + 1)];

    // copy data to new table
    memcpy(newData,       data,         sizeof(varType) * numRows * numColumns);   
    memcpy(newColumnName, columnName,   sizeof(char*)   * numColumns);
    
    // set name column of new column
    newColumnName[numColumns] = NULL;

    // set pointers
    curRow  = 0;
    colNew  = &newData[numColumns  *numRows+curRow];
    colA    = &newData[columnIndexA*numRows+curRow];

    // calc new column
    for (curRow=0; curRow<numRows; curRow++) {
        *colNew = userFunction(*colA);
        colNew++; colA++; 
    }

    // delete
    deleteDataArray();
    deleteColumnNameArray(false);
        
    newColumnIndex  = numColumns;
    numColumns++;
    data            = newData;
    columnName      = newColumnName;
}

//-----------------------------------------------------------------------------
// Name: addColumn()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> void table<varType>::addColumn(unsigned int &newColumnIndex, varType userFunction(varType a, varType b), unsigned int columnIndexA, unsigned int columnIndexB)
{
    // parameters ok?
    if (!numColumns || !numRows) return;

    // locals
    unsigned int            curRow;
    varType                *colA, *colB, *colNew;
    varType                *newData         = new varType[(numColumns + 1) * numRows];
    char                  **newColumnName   = new char*  [(numColumns + 1)];

    // copy data to new table
    memcpy(newData,       data,         sizeof(varType) * numRows * numColumns);   
    memcpy(newColumnName, columnName,   sizeof(char*)   * numColumns);
    
    // set name column of new column
    newColumnName[numColumns] = NULL;

    // set pointers
    curRow  = 0;
    colNew  = &newData[numColumns  *numRows+curRow];
    colA    = &newData[columnIndexA*numRows+curRow];
    colB    = &newData[columnIndexB*numRows+curRow];

    // calc new column
    for (curRow=0; curRow<numRows; curRow++) {
        *colNew = userFunction(*colA, *colB);
        colNew++; colA++; colB++; 
    }

    // delete
    deleteDataArray();
    deleteColumnNameArray(false);
        
    newColumnIndex  = numColumns;
    numColumns++;
    data            = newData;
    columnName      = newColumnName;
}

//-----------------------------------------------------------------------------
// Name: addColumn()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> void table<varType>::addColumn(unsigned int &newColumnIndex, varType userFunction(varType a, varType b, varType c), unsigned int columnIndexA, unsigned int columnIndexB, unsigned int columnIndexC)
{
    // parameters ok?
    if (!numColumns || !numRows) return;

    // locals
    unsigned int            curRow;
    varType                *colA, *colB, *colC, *colNew;            
    varType                *newData         = new varType[(numColumns + 1) * numRows];
    char                  **newColumnName   = new char*[(numColumns + 1)];
    
    // copy data to new table
    memcpy(newData,       data,         sizeof(varType) * numRows * numColumns);
    memcpy(newColumnName, columnName,   sizeof(char*)             * numColumns);

    // set name column of new column
    newColumnName[numColumns] = NULL;

    // set pointers
    curRow  = 0;
    colNew  = &newData[numColumns  *numRows+curRow];
    colA    = &newData[columnIndexA*numRows+curRow];
    colB    = &newData[columnIndexB*numRows+curRow];
    colC    = &newData[columnIndexC*numRows+curRow];
    
    // calc new column
    for (curRow=0; curRow<numRows; curRow++) {
        *colNew = userFunction(*colA, *colB, *colC);
        colNew++; colA++; colB++; colC++;
    }

    // delete
    deleteDataArray();
    deleteColumnNameArray(false);

    newColumnIndex  = numColumns;
    numColumns++;
    data = newData;
    columnName  = newColumnName;
}

//-----------------------------------------------------------------------------
// Name: addColumn()
// Desc: Adds a column with a constant value.
//-----------------------------------------------------------------------------
template <class varType> void table<varType>::addColumn(unsigned int &newColumnIndex, varType value)
{
    // parameters ok?
    if (!numRows) return;

    // locals
    unsigned int            curRow;
    varType                *colNew;
    varType                *newData         = new varType[(numColumns + 1) * numRows];
    char                  **newColumnName   = new char*[(numColumns + 1)];

    // copy data to new table
    memcpy(newData,       data,         sizeof(varType) * numRows * numColumns);
    memcpy(newColumnName, columnName,   sizeof(char*)             * numColumns);

    // set name column of new column
    newColumnName[numColumns] = NULL;

    // set pointers
    curRow  = 0;
    colNew  = &newData[numColumns*numRows+curRow];

    // calc new column
    for (curRow=0; curRow<numRows; curRow++) {
        *colNew = value;
        colNew++;
    }

    // delete
    deleteDataArray();
    deleteColumnNameArray(false);
        
    newColumnIndex  = numColumns;
    numColumns++;
    data        = newData;
    columnName  = newColumnName;
}

//-----------------------------------------------------------------------------
// Name: addColumn()
// Desc: Adds a column with a constant value.
//-----------------------------------------------------------------------------
template <class varType> void table<varType>::addColumn(char name[], varType value)
{
    // parameters ok?
    if (!numRows) return;

    // locals
    unsigned int            curRow;
    varType                *colNew;
    varType                *newData         = new varType[(numColumns + 1) * numRows];
    char                  **newColumnName   = new char*[(numColumns + 1)];

    // copy data to new table
    memcpy(newData,       data,         sizeof(varType) * numRows * numColumns);
    memcpy(newColumnName, columnName,   sizeof(char*)             * numColumns);

    // set name column of new column
    newColumnName[numColumns] = new char[strlen(name) + 1];
    strcpy(newColumnName[numColumns], name);

    // set pointers
    curRow  = 0;
    colNew  = &newData[numColumns*numRows+curRow];

    // calc new column
    for (curRow=0; curRow<numRows; curRow++) {
        *colNew = value;
        colNew++;
    }

    // delete
    deleteDataArray();
    deleteColumnNameArray(false);
        
    numColumns++;
    data        = newData;
    columnName  = newColumnName;
}

//-----------------------------------------------------------------------------
// Name: addColumn()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> void table<varType>::addColumn(char name[], varType userFunction(varType a), unsigned int columnIndexA)
{
    // parameters ok?
    if (!numColumns || !numRows) return;

    // locals
    unsigned int            curRow;
    varType                *colA, *colNew;
    varType                *newData         = new varType[(numColumns + 1) * numRows];
    char                  **newColumnName   = new char*[(numColumns + 1)];

    // copy data to new table
    memcpy(newData,       data,         sizeof(varType) * numRows * numColumns);   
    memcpy(newColumnName, columnName,   sizeof(char*)   * numColumns);
    
    // set name column of new column
    newColumnName[numColumns] = new char[strlen(name) + 1];
    strcpy(newColumnName[numColumns], name);

    // set pointers
    curRow  = 0;
    colNew  = &newData[numColumns  *numRows+curRow];
    colA    = &newData[columnIndexA*numRows+curRow];

    // calc new column
    for (curRow=0; curRow<numRows; curRow++) {
        *colNew = userFunction(*colA);
        colNew++; colA++; 
    }

    // delete
    deleteDataArray();
    deleteColumnNameArray(false);
        
    numColumns++;
    data        = newData;
    columnName  = newColumnName;
}

//-----------------------------------------------------------------------------
// Name: addColumn()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> void table<varType>::addColumn(char name[], varType userFunction(varType a, varType b), unsigned int columnIndexA, unsigned int columnIndexB)
{
    // parameters ok?
    if (!numColumns || !numRows) return;

    // locals
    unsigned int            curRow;
    varType                *colA, *colB, *colNew;            
    varType                *newData         = new varType[(numColumns + 1) * numRows];
    char                  **newColumnName   = new char*[(numColumns + 1)];
    
    // copy data to new table
    memcpy(newData,       data,         sizeof(varType) * numRows * numColumns);
    memcpy(newColumnName, columnName,   sizeof(char*)             * numColumns);
    
    // set name column of new column
    newColumnName[numColumns] = new char[strlen(name) + 1];
    strcpy(newColumnName[numColumns], name);

    // set pointers
    curRow  = 0;
    colNew  = &newData[numColumns  *numRows+curRow];
    colA    = &newData[columnIndexA*numRows+curRow];
    colB    = &newData[columnIndexB*numRows+curRow];

    // calc new column
    for (curRow=0; curRow<numRows; curRow++) {
        *colNew = userFunction(*colA, *colB);
        colNew++; colA++; colB++;
    }

    // delete
    deleteDataArray();
    deleteColumnNameArray(false);

    numColumns++;
    data = newData;
    columnName  = newColumnName;
}

//-----------------------------------------------------------------------------
// Name: addColumn()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> void table<varType>::addColumn(char name[], varType userFunction(varType a, varType b, varType c), unsigned int columnIndexA, unsigned int columnIndexB, unsigned int columnIndexC)
{
    // parameters ok?
    if (!numColumns || !numRows) return;

    // locals
    unsigned int            curRow;
    varType                *colA, *colB, *colC, *colNew;            
    varType                *newData         = new varType[(numColumns + 1) * numRows];
    char                  **newColumnName   = new char*[(numColumns + 1)];
    
    // copy data to new table
    memcpy(newData,       data,         sizeof(varType) * numRows * numColumns);
    memcpy(newColumnName, columnName,   sizeof(char*)             * numColumns);

    // set name column of new column
    newColumnName[numColumns] = new char[strlen(name) + 1];
    strcpy(newColumnName[numColumns], name);

    // set pointers
    curRow  = 0;
    colNew  = &newData[numColumns  *numRows+curRow];
    colA    = &newData[columnIndexA*numRows+curRow];
    colB    = &newData[columnIndexB*numRows+curRow];
    colC    = &newData[columnIndexC*numRows+curRow];
    
    // calc new column
    for (curRow=0; curRow<numRows; curRow++) {
        *colNew = userFunction(*colA, *colB, *colC);
        colNew++; colA++; colB++; colC++;
    }

    // delete
    deleteDataArray();
    deleteColumnNameArray(false);

    numColumns++;
    data = newData;
    columnName  = newColumnName;
}

//-----------------------------------------------------------------------------
// Name: setColumnName()
// Desc: assigns a name to a column
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::setColumnName(unsigned int columnIndex, const char *name)
{
    // parameters ok?
    if (columnIndex>=numColumns) return false;

    // delete old name
    if (columnName[columnIndex]!=NULL) delete [] columnName[columnIndex];

    // copy string
	if (name!=NULL) {
		columnName[columnIndex] = new char[strlen(name) + 1];
		strcpy(columnName[columnIndex], name);
	}

    return true;
}

//-----------------------------------------------------------------------------
// Name: setColumnName()
// Desc: assigns a name to a column
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::setRowName(unsigned int rowIndex, const char *name)
{
    // parameters ok?
    if (rowIndex>=numRows) return false;

    // delete old name
    if (rowName[rowIndex]!=NULL) delete [] rowName[rowIndex];

    // copy string
    rowName[rowIndex] = new char[strlen(name) + 1];
    strcpy(rowName[rowIndex], name);

    return true;
}

//-----------------------------------------------------------------------------
// Name: deleteAllColumnNames()
// Desc: Set all column names to NULL.
//-----------------------------------------------------------------------------
template <class varType> void table<varType>::deleteAllColumnNames()
{
    // locals
    unsigned int columnIndex;

    // delete old name
    for (columnIndex=0; columnIndex<numColumns; columnIndex++) {
        if (columnName[columnIndex]!=NULL) {
            delete [] columnName[columnIndex];
            columnName[columnIndex] = NULL;
        }
    }
}

//-----------------------------------------------------------------------------
// Name: deleteUnnamedColumns()
// Desc: delete the unnamed columns
//-----------------------------------------------------------------------------
template <class varType> void table<varType>::deleteUnnamedColumns()
{
    // locals
    unsigned int curColumnInOldData, curColumnInNewData, newNumColumns;
    varType      *newData;
    char        **newColumnName;

    // determine new number of columns
    for (curColumnInOldData=0, newNumColumns=numColumns; curColumnInOldData<numColumns; curColumnInOldData++) {
        if (columnName[curColumnInOldData]==NULL || strlen(columnName[curColumnInOldData])==0) newNumColumns--;
    }

    // allocate new array
    newData         = new varType[newNumColumns * numRows];
    newColumnName   = new char*[newNumColumns];

    // copy columns
    for (curColumnInOldData=0, curColumnInNewData=0; curColumnInOldData<numColumns; curColumnInOldData++) {
        if (columnName[curColumnInOldData]!=NULL && strlen(columnName[curColumnInOldData])!=0) {
            newColumnName[curColumnInNewData] = columnName[curColumnInOldData];
            memcpy(&newData[curColumnInNewData*numRows],    &data[curColumnInOldData*numRows], sizeof(varType) * numRows);
            curColumnInNewData++;
        }
    }

    // delete old array
    deleteDataArray();
    deleteColumnNameArray(false);
        
    // update vars
    numColumns  = newNumColumns;
    data        = newData;
    columnName  = newColumnName;
}

//-----------------------------------------------------------------------------
// Name: calcMaxValue()
// Desc: returns the maximum value of a column
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcMaxValue(char name[])
{
    return calcMaxValue(getColumnIndex(name));
}

//-----------------------------------------------------------------------------
// Name: calcMinValue()
// Desc: returns the Minimum value of a column
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcMinValue(char name[], varType considerValuesGreaterThan)
{
    return calcMinValue(getColumnIndex(name), considerValuesGreaterThan);
}

//-----------------------------------------------------------------------------
// Name: calcMinValue()
// Desc: returns the Minimum value of a column
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcMinValue(char name[])
{
    return calcMinValue(getColumnIndex(name));
}

//-----------------------------------------------------------------------------
// Name: calcAvgTop()
// Desc: returns the mean value of the highest 100*threshold-percent of a column
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcAvgTop(char name[], varType threshold)
{
    return calcAvgTop(getColumnIndex(name), threshold);
}

//-----------------------------------------------------------------------------
// Name: calcAvgFlop()
// Desc: returns the mean value of the lowest 100*threshold-percent of a column
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcAvgFlop(char name[], varType threshold)
{
    return calcAvgFlop(getColumnIndex(name), threshold);
}

//-----------------------------------------------------------------------------
// Name: calcMinValue()
// Desc: returns the minimum value of the whole table
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcMinValue()
{
    // parameters ok?
    if (numRows*numColumns == 0) return 0;

    // locals
    unsigned int    curIndex;
    varType         minValue        = data[0];

    for (curIndex=0; curIndex<numRows*numColumns; curIndex++) {
        if (data[curIndex] < minValue) {
            minValue = data[curIndex];
        }
    }

    return minValue;
}

//-----------------------------------------------------------------------------
// Name: calcMaxValue()
// Desc: returns the maximum value of the whole table
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcMaxValue()
{
    // parameters ok?
    if (numRows*numColumns == 0) return 0;

    // locals
    unsigned int    curIndex;
    varType         maxValue        = data[0];

    for (curIndex=0; curIndex<numRows*numColumns; curIndex++) {
        if (data[curIndex] > maxValue) {
            maxValue = data[curIndex];
        }
    }

    return maxValue;
}

//-----------------------------------------------------------------------------
// Name: calcMaxValue()
// Desc: returns the maximum value of a column
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcMaxValue(unsigned int columnIndex)
{
    // parameters ok?
    if (numRows*numColumns == 0) return 0;
    if (columnIndex>=numColumns) return 0;

    // locals
    unsigned int    curRow;
    varType         maxValue        = data[columnIndex*numRows+0];

    for (curRow=0; curRow<numRows; curRow++) {
        if (data[columnIndex * numRows + curRow] > maxValue) {
            maxValue = data[columnIndex * numRows + curRow];
        }
    }

    return maxValue;
}

//-----------------------------------------------------------------------------
// Name: calcMaxValue()
// Desc: returns the maximum value of a column considering the rows fromRow (incl.) to toRow (incl.)
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcMaxValue(unsigned int columnIndex, unsigned int &rowWithMaxValue)
{
	return calcMaxValue(columnIndex, rowWithMaxValue, 0, numRows-1);
}

//-----------------------------------------------------------------------------
// Name: calcMaxValue()
// Desc: returns the maximum value of a column considering the rows fromRow (incl.) to toRow (incl.)
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcMaxValue(unsigned int columnIndex, unsigned int &rowWithMaxValue, unsigned int fromRow, unsigned int toRow)
{
    // parameters ok?
    if (numRows*numColumns == 0) return 0;
    if (columnIndex>=numColumns) return 0;
	if (fromRow	   >=numRows   ) return 0;
	if (toRow	   >=numRows   ) return 0;

    // locals
    unsigned int    curRow;
    varType         maxValue        = data[columnIndex*numRows+fromRow];
					rowWithMaxValue	= fromRow;

    for (curRow=fromRow; curRow<=toRow; curRow++) {
        if (data[columnIndex * numRows + curRow] > maxValue) {
            maxValue = data[columnIndex * numRows + curRow];
			rowWithMaxValue = curRow;
        }
    }

    return maxValue;
}

//-----------------------------------------------------------------------------
// Name: calcMinValue()
// Desc: returns the Minimum value of a column
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcMinValue(unsigned int columnIndex, varType considerValuesGreaterThan)
{
    // parameters ok?
    if (numRows*numColumns == 0) return 0;
    if (columnIndex>=numColumns) return 0;
	varType curValue; 

    // locals
    unsigned int    curRow;
    varType         MinValue        = 1e67;

    for (curRow=0; curRow<numRows; curRow++) {
		curValue = data[columnIndex * numRows + curRow];
        if (curValue >= considerValuesGreaterThan && curValue < MinValue) {
            MinValue = curValue;
        }
    }

    return MinValue;
}

//-----------------------------------------------------------------------------
// Name: calcMinValue()
// Desc: returns the Minimum value of a column
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcMinValue(unsigned int columnIndex)
{
    // parameters ok?
    if (numRows*numColumns == 0) return 0;
    if (columnIndex>=numColumns) return 0;

    // locals
    unsigned int    curRow;
    varType         MinValue        = data[columnIndex*numRows+0];

    for (curRow=0; curRow<numRows; curRow++) {
        if (data[columnIndex * numRows + curRow] < MinValue) {
            MinValue = data[columnIndex * numRows + curRow];
        }
    }

    return MinValue;
}

//-----------------------------------------------------------------------------
// Name: calcMinValue()
// Desc: returns the Minimum value of a column
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcMinValue(unsigned int columnIndex, unsigned int &rowWithMinValue)
{
    // parameters ok?
    if (numRows*numColumns == 0) return 0;
    if (columnIndex>=numColumns) return 0;

    // locals
    unsigned int    curRow;
    varType         MinValue        = data[columnIndex*numRows+0];
					rowWithMinValue	= 0;

    for (curRow=0; curRow<numRows; curRow++) {
        if (data[columnIndex * numRows + curRow] < MinValue) {
            MinValue = data[columnIndex * numRows + curRow];
			rowWithMinValue = curRow;
        }
    }

    return MinValue;
}

//-----------------------------------------------------------------------------
// Name: calcMinValue()
// Desc: returns the Minimum value of a column considering the rows fromRow (incl.) to toRow (incl.)
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcMinValue(unsigned int columnIndex, unsigned int &rowWithMinValue, unsigned int fromRow, unsigned int toRow)
{
    // parameters ok?
    if (numRows*numColumns == 0) return 0;
    if (columnIndex>=numColumns) return 0;
	if (fromRow	   >=numRows   ) return 0;
	if (toRow	   >=numRows   ) return 0;

    // locals
    unsigned int    curRow;
    varType         MinValue        = data[columnIndex*numRows+fromRow];
					rowWithMinValue	= fromRow;

    for (curRow=fromRow; curRow<=toRow; curRow++) {
        if (data[columnIndex * numRows + curRow] < MinValue) {
            MinValue = data[columnIndex * numRows + curRow];
			rowWithMinValue = curRow;
        }
    }

    return MinValue;
}

//-----------------------------------------------------------------------------
// Name: calcSum()
// Desc: returns the sum of the whole table
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcSum()
{
    // parameters ok?
    if (numRows*numColumns == 0) return 0;

    // locals
    unsigned int    curDatapoint;
    varType         sumValue        = 0;

    for (curDatapoint=0; curDatapoint<numRows*numColumns; curDatapoint++) {
		sumValue += data[curDatapoint];
    }

    return sumValue;
}

//-----------------------------------------------------------------------------
// Name: calcLowerQuartile()
// Desc: returns the lower quartile of the column values
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcLowerQuartile(char name[])
{
    return calcLowerQuartile(getColumnIndex(name));
}

//-----------------------------------------------------------------------------
// Name: calcUpperQuartile()
// Desc: returns the upper quartile of the column values
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcUpperQuartile(char name[])
{
    return calcUpperQuartile(getColumnIndex(name));
}

//-----------------------------------------------------------------------------
// Name: calcMedian()
// Desc: returns the median of the column values
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcMedian(char name[])
{
    return calcMedian(getColumnIndex(name));
}

//-----------------------------------------------------------------------------
// Name: calcLowerQuartile()
// Desc: returns the lower quartile of the column values
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcLowerQuartile(unsigned int columnIndex)
{
    return getValueOfSortedList(columnIndex, (unsigned int) (0.25f*numRows));
}

//-----------------------------------------------------------------------------
// Name: calcLowerQuartile()
// Desc: returns the lower quartile of the column values
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcUpperQuartile(unsigned int columnIndex)
{
    return getValueOfSortedList(columnIndex, (unsigned int) (0.75f*numRows));
}

//-----------------------------------------------------------------------------
// Name: calcLowerQuartile()
// Desc: returns the median of the column values
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcMedian(unsigned int columnIndex)
{
    return getValueOfSortedList(columnIndex, (unsigned int) (0.5f*numRows));
}

//-----------------------------------------------------------------------------
// Name: calcLowerQuartile()
// Desc: returns the median of the whole table
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcMedian()
{
    return getValueOfSortedList((unsigned int) (0.5f*numRows));
}

//-----------------------------------------------------------------------------
// Name: calcLowerQuartile()
// Desc: returns the lower quartile of the whole table
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcLowerQuartile()
{
    return getValueOfSortedList((unsigned int) (0.25f*numRows));
}

//-----------------------------------------------------------------------------
// Name: calcLowerQuartile()
// Desc: returns the lower quartile of the whole table
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcUpperQuartile()
{
    return getValueOfSortedList((unsigned int) (0.75f*numRows));
}

//-----------------------------------------------------------------------------
// Name: getValueOfSortedList()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::getValue(unsigned int columnIndex, unsigned int row)
{
	if (columnIndex >= numColumns) return NAN;
	if (row         >= numRows   ) return NAN;
	return this->data[columnIndex * numRows + row];
}

//-----------------------------------------------------------------------------
// Name: getValueOfSortedList()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::getValueOfSortedList(unsigned int pos)
{
    // locals
    unsigned int            curPos, posInList;
    list<varType>           listValues;

    // parameters ok?
    if (numRows==0)			return 0;
    if (numColumns==0)		return 0;

    // put values of the list in an array
    for (curPos=0; curPos<numRows*numColumns; curPos++) {
        listValues.push_back(data[curPos]);
    }
   
    // sort values
    listValues.sort();

    // sum up top values
    posInList=0;
    for (auto& curVal : listValues) {
        if (posInList>=pos) return curVal;
        posInList++;
    }

    return listValues.back();
}

//-----------------------------------------------------------------------------
// Name: getValueOfSortedList()
// Desc: returns the lower quartile of the column values
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::getValueOfSortedList(unsigned int columnIndex, unsigned int pos)
{
    // locals
    unsigned int            curRow, posInList;
    list<varType>           listValues;

    // parameters ok?
    if (numRows == 0)			 return 0;
    if (columnIndex>=numColumns) return 0;

    // put values of the list in an array
    for (curRow=0; curRow<numRows; curRow++) {
        listValues.push_back(data[columnIndex * numRows + curRow]);
    }
   
    // sort values
    listValues.sort();

    // sum up top values
    posInList=0;
    for (auto& curVal : listValues) {
        if (posInList>=pos) return curVal;
        posInList++;
    }

    return listValues.back();
}

//-----------------------------------------------------------------------------
// Name: calcMedianStepSize()
// Desc: returns the median of the column value change from row to row
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcMedianStepSize(unsigned int columnIndex)
{
    // parameters ok?
    if (numRows <= 1)			 return 0;
    if (columnIndex>=numColumns) return 0;

	// locals
	table <varType> myTable(1, this->numRows - 1);
	unsigned int curRow;

	// fill temporary table with differences from row to row
	for (curRow=0; curRow<myTable.numRows; curRow++) {
		myTable.data[curRow] = this->data[columnIndex * this->numRows + curRow + 1] - this->data[columnIndex * this->numRows + curRow];
	}

	// return median of temporary table
    return myTable.calcMedian();
}

//-----------------------------------------------------------------------------
// Name: calcSum()
// Desc: returns the sum of the column
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcSum(unsigned int columnIndex)
{
    // locals
    unsigned int curRow;
    varType  sum = 0;

    // sum up values
    for (curRow=0; curRow<numRows; curRow++) {
        sum += data[columnIndex*numRows+curRow];
    }

    return sum;
}

//-----------------------------------------------------------------------------
// Name: calcAvgTop()
// Desc: returns the mean value
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcAverage(unsigned int columnIndex)
{
    // locals
    unsigned int curRow;
    varType  sum = 0;

    // sum up values
    for (curRow=0; curRow<numRows; curRow++) {
        sum += data[columnIndex*numRows+curRow];
    }

    return sum / numRows;
}

//-----------------------------------------------------------------------------
// Name: calcAvgTop()
// Desc: returns the mean value of the highest 100*threshold-percent of a column
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcAvgTop(unsigned int columnIndex, varType threshold)
{
    // locals
    unsigned int            curRow, posInList;
    unsigned int            numTopValues        = (unsigned int) (threshold * numRows);
    varType                 sumTopValues        = 0;
    list<varType>           listValues;
    typename list<varType>::iterator listIter;

    // parameters ok?
    if (numRows == 0) return 0;
    if (columnIndex>=numColumns) return 0;
    if (numTopValues==0) return calcMaxValue(columnIndex);

    // put values of the list in an array
    for (curRow=0; curRow<numRows; curRow++) {
        listValues.push_back(data[columnIndex * numRows + curRow]);
    }
   
    // sort values
    listValues.sort();

    // sum up top values
    for (sumTopValues=0, posInList=0, listIter=listValues.end(), listIter--; listIter != listValues.begin(); listIter--, posInList++) {
        if (posInList >= numTopValues) break;
        sumTopValues += (*listIter);
    }

    return sumTopValues / numTopValues;
}

//-----------------------------------------------------------------------------
// Name: calcAvgFlop()
// Desc: returns the mean value of the lowest 100*threshold-percent of a column
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::calcAvgFlop(unsigned int columnIndex, varType threshold)
{
    // locals
    unsigned int            curColumn, curRow, posInList;
    unsigned int            numFlopValues       = (unsigned int) (threshold * numRows);
    varType                 sumFlopValues       = 0;
    list<varType>           listValues;
    typename list<varType>::iterator listIter;

    // parameters ok?
    if (numRows == 0) return 0;
    if (columnIndex>=numColumns) return 0;
    if (numFlopValues==0) return calcMinValue(columnIndex);

    // put values of the list in an array
    for (curRow=0; curRow<numRows; curRow++) {
        listValues.push_back(data[columnIndex * numRows + curRow]);
    }
   
    // sort values
    listValues.sort();

    // sum up Flop values
    for (sumFlopValues=0, posInList=0, listIter=listValues.begin(); listIter != listValues.end(); listIter++, posInList++) {
        if (posInList >= numFlopValues) break;
        sumFlopValues += (*listIter);
    }

    return sumFlopValues / numFlopValues;
}

//-----------------------------------------------------------------------------
// Name: getRowIndex()
// Desc: returns the index a row with this name
//-----------------------------------------------------------------------------
template <class varType> unsigned int table<varType>::getRowIndex(char name[])
{
    // locals
    unsigned int curRow;

    for (curRow=0; curRow<numRows; curRow++) {
        if (rowName[curRow]!=NULL) {
            if (strcmp(rowName[curRow], name) == 0) {
                return curRow;
            }
        }
    }

	if (stopOnError) {
		cout << "ERROR: table::getRowIndex(): Row with name '" << name << "' not found!" << endl;
		while (true);
	} else {
		return curRow;
	}
}

//-----------------------------------------------------------------------------
// Name: getColumnIndex()
// Desc: returns the index a column with this name
//-----------------------------------------------------------------------------
template <class varType> unsigned int table<varType>::getColumnIndex(const char name[])
{
    // locals
    unsigned int curColumn;

    for (curColumn=0; curColumn<numColumns; curColumn++) {
        if (columnName[curColumn]!=NULL) {
            if (strcmp(columnName[curColumn], name) == 0) {
                break;
            }
        }
    }
  
    return curColumn;
}

//-----------------------------------------------------------------------------
// Name: getColumn()
// Desc: returns a pointer to the column with this index
//-----------------------------------------------------------------------------
template <class varType> varType *table<varType>::getColumn(unsigned int columnIndex)
{
    // locals
    unsigned int curColumn = columnIndex;

    if (curColumn <= numColumns) {
        return &data[curColumn*numRows+0];
    } else {
        return NULL;
    }
}


//-----------------------------------------------------------------------------
// Name: getColumn()
// Desc: returns a pointer to the column with this name
//-----------------------------------------------------------------------------
template <class varType> varType *table<varType>::getColumn(char name[])
{
    // locals
    unsigned int curColumn = getColumnIndex(name);

    if (curColumn <= numColumns) {
        return &data[curColumn*numRows+0];
    } else {
        return NULL;
    }
}

//-----------------------------------------------------------------------------
// Name: getColumn()
// Desc: returns a pointer to the column with this name
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::copyRow(unsigned int fromRowIndex, unsigned int toRowIndex)
{
	// locals
	unsigned int curColumn;

	// parameters ok?
    if (fromRowIndex>=numRows)	return false; 
	if (toRowIndex>=numRows)	return false; 

	for (curColumn=0; curColumn<numColumns; curColumn++) {
		data[curColumn * numRows + toRowIndex]	= data[curColumn * numRows + fromRowIndex];
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: copyRow()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::copyRow(unsigned int rowIndexInTargetTable, unsigned int rowIndexInSourceTable, table &sourceTable)
{
	// locals
	unsigned int curColumn;

	// parameters ok?
    if (rowIndexInTargetTable   >= numRows			  )	return false; 
	if (rowIndexInSourceTable   >= sourceTable.numRows)	return false; 
	if (sourceTable.numColumns	!= numColumns		  )	return false; 

	for (curColumn=0; curColumn<numColumns; curColumn++) {
		data[curColumn * numRows + rowIndexInTargetTable]	= sourceTable.data[curColumn * numRows + rowIndexInSourceTable];
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: copyColumn()
// Desc: copy the values of a whole column from another table. Both tables must have the same number of rows.
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::copyColumn(unsigned int colIndexInTargetTable, unsigned int colIndexInSourceTable, table &sourceTable)
{
	if (colIndexInTargetTable	!= numColumns			 )	return false;
	if (colIndexInSourceTable	!= sourceTable.numColumns)	return false;
	if (numRows					!= sourceTable.numRows)		return false;

	memcpy(&data[colIndexInTargetTable*numRows], &sourceTable.data[colIndexInSourceTable*numRows], sizeof(varType)*numRows);
}

//-----------------------------------------------------------------------------
// Name: copyColumn()
// Desc: add a new column by copying it from another table
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::copyColumn(char name[], table &anotherTable)
{
    // locals
    varType                *newData                 = new varType[(numColumns + 1) * numRows];
    char                  **newColumnName           = new char*  [(numColumns + 1)];
    unsigned int            colIndexAnotherTable    = anotherTable.getColumnIndex(name);

    // parameters ok?
    if (colIndexAnotherTable>=anotherTable.numColumns) return false; 

    // copy data to new table
    memcpy(newData,       data,         sizeof(varType) * numRows * numColumns);   
    memcpy(newColumnName, columnName,   sizeof(char*)             * numColumns);
    
    // copy data
    if (anotherTable.columnName[colIndexAnotherTable] != NULL) {
        newColumnName[numColumns]   = new char[strlen(anotherTable.columnName[colIndexAnotherTable])+1];
        strcpy(newColumnName[numColumns], anotherTable.columnName[colIndexAnotherTable]);
    } else {
        newColumnName[numColumns]   = NULL;
    }
    memcpy(&newData[numColumns*numRows+0], &anotherTable.data[colIndexAnotherTable*anotherTable.numRows+0], sizeof(varType) * numRows);

    // delete
    deleteDataArray();
    deleteColumnNameArray(false);
        
    numColumns++;
    data        = newData;
    columnName  = newColumnName;

    return true;
}

//-----------------------------------------------------------------------------
// Name: copyColumnNames()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::copyColumnNames(table &anotherTable)
{
    // parameters ok?
    if (anotherTable.numColumns != numColumns) return false;

    // locals
    unsigned int curCol;

    for (curCol=0; curCol<numColumns; curCol++) {
        if (anotherTable.columnName[curCol] != NULL) {
            setColumnName(curCol, anotherTable.columnName[curCol]);
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
// Name: smoothData()
// Desc: The range includes the row 'fromRow', but not the row 'toRow' anymore.
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::smoothData(unsigned int numPointsToAvgOver, unsigned int columnIndex, varType *tmpColumn, unsigned int fromRow, unsigned int toRow)
{
    // parameters ok?
    if (!numColumns || !numRows)	return false;
	if (columnIndex >= numColumns)	return false;
	if (numPointsToAvgOver <= 1)	return false;
	if (tmpColumn == NULL)			return false;
	if (fromRow >= numRows)			return false;
	if (toRow > numRows)			return false;
	
	// locals
	unsigned int	startPoint;
	unsigned int	endPoint;
	unsigned int	curPoint;
	unsigned int	curRow;
	unsigned int    halfNumPointsToAvgOver = numPointsToAvgOver / 2;
	varType			avg;

	// copy values to temporary column
	for (curRow = fromRow; curRow < toRow; curRow++) {
		tmpColumn[curRow - fromRow] = data[columnIndex * numRows + curRow];
	}
	
	// calculate average for each value
	for (curRow = fromRow; curRow < toRow; curRow++) {

		// first few datapoints
		if (curRow < fromRow + halfNumPointsToAvgOver) {
			startPoint	= fromRow;
			endPoint	= curRow + halfNumPointsToAvgOver;
		//  center range
		} else if (curRow < toRow - halfNumPointsToAvgOver) {
			startPoint	= curRow - halfNumPointsToAvgOver;
			endPoint	= curRow + halfNumPointsToAvgOver;
		// last few points
		} else {
			startPoint	= curRow - halfNumPointsToAvgOver;
			endPoint	= toRow - 1;
		}

		// calc average
		for (avg=0,curPoint=startPoint; curPoint<=endPoint; curPoint++) {
			avg += tmpColumn[curPoint - fromRow];
		}
		data[columnIndex * numRows + curRow] = avg / (endPoint-startPoint+1);
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: filterRows()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::filterRows(unsigned int numRowsToKeep, unsigned int rowsToKeep[])
{
    // parameters ok?
    if (!numColumns || !numRows) return false;

    // locals
    unsigned int curRowInOldData, curRowInNewData, curColumn;
    varType      *newData;
    char        **newRowName;
    bool         *keepList  = new bool[numRows];

    // make keepList
    for (curRowInOldData=0; curRowInOldData<numRows; curRowInOldData++) {
        keepList[curRowInOldData] = false;
    }
    for (curRowInNewData=0; curRowInNewData<numRowsToKeep; curRowInNewData++) {
        keepList[rowsToKeep[curRowInNewData]] = true;
    }
    //recount keepList since values in 'rowsToKeep' can occur several times
    for (curRowInOldData=0, numRowsToKeep=0; curRowInOldData<numRows; curRowInOldData++) {
        if (keepList[curRowInOldData]) numRowsToKeep++;
    }

    // allocate new array
    newData         = new varType[numColumns * numRowsToKeep];
    newRowName      = new char*[numRowsToKeep];

    // copy rows
    for (curRowInOldData=0, curRowInNewData=0; curRowInOldData<numRows; curRowInOldData++) {
        if (keepList[curRowInOldData]) {
            newRowName[curRowInNewData] = rowName[curRowInOldData];
            for (curColumn=0; curColumn<numColumns; curColumn++) {
                newData[curColumn*numRowsToKeep+curRowInNewData] = data[curColumn*numRows+curRowInOldData];
            }
            curRowInNewData++;
        } else {
            if (rowName[curRowInOldData]!=NULL) {
                delete [] rowName[curRowInOldData];
                rowName[curRowInOldData] = NULL;
            }
        }
    }

    // delete old array
    deleteDataArray();
    deleteRowNameArray(false);
        
    // update vars
    numRows     = numRowsToKeep;
    data        = newData;
    rowName     = newRowName;   
    delete [] keepList;

    return true;
}

//-----------------------------------------------------------------------------
// Name: filterRows()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::filterRows(bool keepRow(varType a), char columnNameA[])
{
    // parameters ok?
    if (!numColumns || !numRows) return false;

    // locals
    unsigned int curRowInOldData, curRowInNewData, newNumRows, curColumn;
    varType      *colA;
    varType      *newData;
    char        **newRowName;
    bool         *keepList  = new bool[numRows];
    unsigned int  colIndexA = getColumnIndex(columnNameA);
    
    // parameters ok?
    if (colIndexA >= numColumns) return false;

    colA    = &data[colIndexA*numRows];

    // determine new number of columns
    for (curRowInOldData=0, newNumRows=0; curRowInOldData<numRows; curRowInOldData++) {
        if (keepList[curRowInOldData] = keepRow(*colA)) {
            newNumRows++;
        }
        colA++; 
    }

    // allocate new array
    newData         = new varType[numColumns * newNumRows];
    newRowName      = new char*[newNumRows];

    // copy rows
    for (curRowInOldData=0, curRowInNewData=0; curRowInOldData<numRows; curRowInOldData++) {
        if (keepList[curRowInOldData]) {
            newRowName[curRowInNewData] = rowName[curRowInOldData];
            for (curColumn=0; curColumn<numColumns; curColumn++) {
                newData[curColumn*newNumRows+curRowInNewData] = data[curColumn*numRows+curRowInOldData];
            }
            curRowInNewData++;
        } else {
            if (rowName[curRowInOldData]!=NULL) {
                delete [] rowName[curRowInOldData];
                rowName[curRowInOldData] = NULL;
            }
        }
    }

    // delete old array
    deleteDataArray();
    deleteRowNameArray(false);
        
    // update vars
    numRows     = newNumRows;
    data        = newData;
    rowName     = newRowName;  
    delete [] keepList;

    return true;
}

//-----------------------------------------------------------------------------
// Name: filterRows()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::filterRows(bool keepRow(varType a, varType b), char columnNameA[], char columnNameB[])
{
    // parameters ok?
    if (!numColumns || !numRows) return false;

    // locals
    unsigned int curRowInOldData, curRowInNewData, newNumRows, curColumn;
    varType      *colA, *colB;
    varType      *newData;
    char        **newRowName;
    bool         *keepList  = new bool[numRows];
    unsigned int  colIndexA = getColumnIndex(columnNameA);
    unsigned int  colIndexB = getColumnIndex(columnNameB);
    
    // parameters ok?
    if (colIndexA >= numColumns) return false;
    if (colIndexB >= numColumns) return false;

    colA    = &data[colIndexA*numRows];
    colB    = &data[colIndexB*numRows];

    // determine new number of columns
    for (curRowInOldData=0, newNumRows=0; curRowInOldData<numRows; curRowInOldData++) {
        if (keepList[curRowInOldData] = keepRow(*colA, *colB)) {
            newNumRows++;
        }
        colA++; 
        colB++;
    }

    // allocate new array
    newData         = new varType[numColumns * newNumRows];
    newRowName      = new char*[newNumRows];

    // copy rows
    for (curRowInOldData=0, curRowInNewData=0; curRowInOldData<numRows; curRowInOldData++) {
        if (keepList[curRowInOldData]) {
            newRowName[curRowInNewData] = rowName[curRowInOldData];
            for (curColumn=0; curColumn<numColumns; curColumn++) {
                newData[curColumn*newNumRows+curRowInNewData] = data[curColumn*numRows+curRowInOldData];
            }
            curRowInNewData++;
        } else {
            if (rowName[curRowInOldData]!=NULL) {
                delete [] rowName[curRowInOldData];
                rowName[curRowInOldData] = NULL;
            }
        }
    }

    // delete old array
    deleteDataArray();
    deleteRowNameArray(false);
        
    // update vars
    numRows     = newNumRows;
    data        = newData;
    rowName     = newRowName;   
    delete [] keepList;

    return true;
}

//-----------------------------------------------------------------------------
// Name: makeArrayFromList()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> void makeArrayFromList(unsigned int &arraySize, varType *&myArray, list<varType> &myList)
{
    arraySize = 0;
    myArray   = new varType[myList.size()];

    for (auto& listItem : myList) {
        myArray[arraySize] = (varType) listItem;
        arraySize++;
    }
}

//-----------------------------------------------------------------------------
// Name: table()
// Desc: Distribute a 1D-Array into bins.
//-----------------------------------------------------------------------------
template <class varType> table<varType>::table(unsigned int numData, varType theData[], unsigned int numBins, varType minimumValue, varType binSize, bool normalize)
{
    // locals
    unsigned int            *binCounts   = new unsigned int[numBins];
    varType                 *binFraction = new varType[numBins];
    varType                  totalSum, numBinnedValues;
    unsigned int             curData, curBin;
    char                     str[100];

    numRows				= numBins;
    numColumns			= 3;
    columnName			= new char*[numColumns];    memset(columnName, NULL, sizeof(char*) * numColumns);
    rowName				= new char*[numRows];       memset(rowName,    NULL, sizeof(char*) * numRows);
    data				= new varType[numColumns*numRows];
	numDigitsOnOutput	= 7;
	stopOnError			= true;

    for (curBin=0; curBin<numBins; curBin++) {
        binCounts[curBin] = 0;
    }

    // calc stuff
    for (totalSum=0, numBinnedValues=0, curData=0; curData<numData; curData++) {
        for (curBin=0; curBin<numBins; curBin++) {
            if (theData[curData] >= minimumValue && theData[curData] - minimumValue <= binSize*(curBin+1)) {
                binCounts[curBin]++;
                totalSum += theData[curData] - minimumValue;
                numBinnedValues++;
                break;
            }
        }
    }

    // normalize
    if (normalize) {
        for (curBin=0; curBin<numBins; curBin++) {
            binFraction[curBin] = (varType) binCounts[curBin] / numBinnedValues;
        }
    }

    // column name
    setColumnName(0, "lower bound");
    setColumnName(1, "upper bound");
    setColumnName(2, (normalize) ? "fraction" : "counts");

    // write into data-array
    for (curBin=0; curBin<numBins; curBin++) {

        // copy data
        if (!normalize) data[2*numBins+curBin] = binCounts  [curBin];
        else            data[2*numBins+curBin] = binFraction[curBin];

        // lower bound
        data[0*numBins+curBin] = binSize*(curBin+0) + minimumValue;

        // upper bound
        data[1*numBins+curBin] = binSize*(curBin+1) + minimumValue;

        // row name
        sprintf(str, "%e", binSize*(curBin+1));
        rowName[curBin]  = new char[strlen(str)+1];
        strcpy(rowName[curBin], str);
    }

    delete [] binCounts;
    delete [] binFraction;
}

//-----------------------------------------------------------------------------
// Name: get_X_valueAt_Y_value()
// Desc: Uses to columns as input and returns the value of the x-axis where the
//       y-axis reaches the value 'valueY'. When 'placement' is i than the (i-1)-th
//       occurence will be returned. After each occurence a distance of 'skipX'
//       is skipped on the x-axis.
//-----------------------------------------------------------------------------
template <class varType> varType table<varType>::get_X_valueAt_Y_value(unsigned int columnIndex_X, unsigned int columnIndex_Y, varType valueY, unsigned int placement, varType skipX)
{
    // locals
    unsigned int curOccurence = 0;
    unsigned int curRow;
    varType      curValueY;
    varType      lastValueY = data[columnIndex_Y * numRows + 0];

    for (curRow=1; curRow<numRows; curRow++) {
        curValueY = data[columnIndex_Y * numRows + curRow];
        if (valueY > curValueY && valueY < lastValueY || valueY < curValueY && valueY > lastValueY) {
            if (curOccurence == placement) {
                return data[columnIndex_X * numRows + curRow];
            } else {
                curOccurence++;
            }
        }
        lastValueY = curValueY;
    }

    return 0;
}

//-----------------------------------------------------------------------------
// Name: transposeArray()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> void table<varType>::transposeArray(unsigned int numRows, unsigned int numColumns, varType* data, varType* transposedData)
{
    // locals
    unsigned int curRow, curColumn;

    for (curRow=0; curRow<numRows; curRow++) {
        for (curColumn=0; curColumn<numColumns; curColumn++) {
            transposedData[curRow * numColumns + curColumn] = data[curColumn * numRows + curRow];
        }
    }
}

//-----------------------------------------------------------------------------
// Name: swapColumns()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> void table<varType>::swapColumns(unsigned int numRows, unsigned int numColumns, unsigned int colA, unsigned int colB, varType* data, varType* dataWithSwappedCol)
{
    // locals
    unsigned int curRow, curColumn;

    for (curRow=0; curRow<numRows; curRow++) {
        for (curColumn=0; curColumn<numColumns; curColumn++) {
            if (curColumn == colA) {
                dataWithSwappedCol[colB * numRows + curRow] = data[colA * numRows + curRow];
            } else if (curColumn == colB) {
                dataWithSwappedCol[colA * numRows + curRow] = data[colB * numRows + curRow];
            } else {
                dataWithSwappedCol[curColumn * numRows + curRow] = data[curColumn * numRows + curRow];
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Name: compareRowAscending()
// Desc: a<b -> -1
//       a=b ->  0
//       a>b ->  1
//-----------------------------------------------------------------------------
template <class varType> int table<varType>::compareRowAscending(const void * a, const void * b)
{
	// return (int) ( *((varType*)a) - *((varType*)b) );	// does not work if values are smaller than one
	double x = *((varType*)a);
	double y = *((varType*)b);

	if (x==y) {
		return 0;
	} else if (x<y) {
		return -1;
	} else {
		return 1;
	}
}

//-----------------------------------------------------------------------------
// Name: compareRowDescending()
// Desc: a<b ->  1
//       a=b ->  0
//       a>b -> -1
//-----------------------------------------------------------------------------
template <class varType> int table<varType>::compareRowDescending(const void * a, const void * b)
{
    // return (int) ( *((varType*)b) - *((varType*)a) )		// does not work if values are smaller than one
	double x = *((varType*)a);
	double y = *((varType*)b);

	if (x==y) {
		return 0;
	} else if (x<y) {
		return 1;
	} else {
		return -1;
	}
}

//-----------------------------------------------------------------------------
// Name: sortTable()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::sortTable(unsigned int columnIndex, bool ascending)
{
    // param ok?
    if (columnIndex >= numColumns) return false;

    // locals
    varType *tmpData = new varType[numColumns * numRows];

    // change columns
    swapColumns(numRows, numColumns, 0, columnIndex, data, tmpData);

    // transposes table
    transposeArray(numRows, numColumns, tmpData, data);

    // sort
    if (ascending) {
        qsort(data, numRows, sizeof(varType)*numColumns, compareRowAscending);
    } else {
        qsort(data, numRows, sizeof(varType)*numColumns, compareRowDescending);
    }

    // copy transposed table back
    transposeArray(numColumns, numRows, data, tmpData);

    // change columns
    swapColumns(numRows, numColumns, 0, columnIndex, tmpData, data);

    // free mem
    delete [] tmpData;

    // everything ok
    return true;
}

//-----------------------------------------------------------------------------
// Name: sortTable()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::sortTable(vector <unsigned int> &columnIndeces, vector<bool> &ascending, double delta)
{
    // param ok?
    if (columnIndeces.size() == 0)						return false;
	if (ascending.size()     != columnIndeces.size())	return false;

    // locals
	size_t			curProcessedIndex;
	unsigned int	curRow;
	unsigned int	tmpColumn			= 0;
	unsigned int	sortedIndicesColumn = 1;
	unsigned int	numControlColumns	= 2;
	table<varType>	keyTable(numRows, (unsigned int) columnIndeces.size()+numControlColumns);	// this table is transposed because of the qsort()-algorithmn
	vector<unsigned int> myColumnIndeces(columnIndeces.size());

	// create a temporary table only containing the original row indices and the columns to sort
	for (curProcessedIndex=0; curProcessedIndex<columnIndeces.size(); curProcessedIndex++) {
		for (curRow=0; curRow<numRows; curRow++) {
			keyTable.data[curRow*keyTable.numRows+curProcessedIndex+numControlColumns] = data[columnIndeces[curProcessedIndex]*numRows+curRow];
		}
		myColumnIndeces[curProcessedIndex] = numControlColumns + (unsigned int) curProcessedIndex;
	}
	for (curRow=0; curRow<numRows; curRow++) {
		keyTable.data[curRow*keyTable.numRows+tmpColumn]			= 0;
		keyTable.data[curRow*keyTable.numRows+sortedIndicesColumn]	= curRow;
	}

	// sort the key table
	keyTable.sortKeyTable(0, numRows, myColumnIndeces, ascending, delta);
	
	// sort whole table using a temperary copy
	table<varType> tmpTable(numColumns, numRows, &data[0]);
	for (curRow=0; curRow<numRows; curRow++) {
		copyRow(curRow, (unsigned int) keyTable.data[curRow*keyTable.numRows+sortedIndicesColumn], tmpTable);
	}
 
    // everything ok
    return true;
}

//-----------------------------------------------------------------------------
// Name: sortKeyTable()
// Desc: CAUTION: table must be transposed!!!
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::sortKeyTable(unsigned int fromCol, unsigned int toCol, vector <unsigned int> &rowIndeces, vector<bool> &ascending, double delta)
{
	// locals
	vector <unsigned int> myRowIndeces(rowIndeces.begin()+1, rowIndeces.end());
	vector <bool        > myAscending (ascending .begin()+1, ascending .end());
	unsigned int curCol, myFromCol, myToCol;
	unsigned int curRowToSort = rowIndeces[0];
	varType curValue;
	
	// copy requested row to first row
	copyRow(curRowToSort, 0);

	// sort
	qsort(&data[fromCol*numRows+0], toCol-fromCol, sizeof(varType)*numRows, ascending[0] ? compareRowAscending : compareRowDescending);
	
	// sort next column if one exists
	if (rowIndeces.size()>1) {
		for (myFromCol=fromCol,myToCol=fromCol,curCol=fromCol; curCol<toCol; curCol++) {
			
			// first column
			if (curCol==fromCol) {
				curValue = data[curCol*numRows+curRowToSort];

			// values equal ?
			} else if (isEqual(curValue, data[curCol*numRows+curRowToSort], delta)) {
				myToCol = curCol;

			// values differ
			} else {
				if (myFromCol!=myToCol) {
					sortKeyTable(myFromCol, myToCol+1, myRowIndeces, myAscending, delta);
				}
				curValue	= data[curCol*numRows+curRowToSort];
				myFromCol	= curCol;
				myToCol		= curCol;
			}
		}
		// last column
		if (myFromCol!=myToCol) {
			sortKeyTable(myFromCol, myToCol+1, myRowIndeces, myAscending, delta);
		}	
	}
	    
	// everything ok
    return true;
}

//-----------------------------------------------------------------------------
// Name: redim()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> void table<varType>::redim(unsigned int nCols, unsigned int nRows)
{
    // delete table
    destroy();

    columnName  = new char*[nCols];   memset(columnName, NULL, sizeof(char*) * nCols);
    rowName     = new char*[nRows];   memset(rowName,    NULL, sizeof(char*) * nRows);
    data        = new varType[nCols*nRows];
    numRows     = nRows;
    numColumns  = nCols;
}

//-----------------------------------------------------------------------------
// Name: redim()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> void table<varType>::redim(unsigned int nCols, unsigned int nRows, varType defaultValue)
{
    // delete table
    destroy();

    columnName  = new char*[nCols];   memset(columnName, NULL, sizeof(char*) * nCols);
    rowName     = new char*[nRows];   memset(rowName,    NULL, sizeof(char*) * nRows);
    data        = new varType[nCols*nRows];
    numRows     = nRows;
    numColumns  = nCols;

	for (unsigned int curDataPoint=0; curDataPoint<numRows*numColumns; curDataPoint++) {
		data[curDataPoint] = defaultValue;
	}
}

//-----------------------------------------------------------------------------
// Name: redim()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> void table<varType>::redim(unsigned int nCols, unsigned int nRows, varType *theData)
{
    // delete table
    destroy();

    columnName  = new char*[nCols];   memset(columnName, NULL, sizeof(char*) * nCols);
    rowName     = new char*[nRows];   memset(rowName,    NULL, sizeof(char*) * nRows);
    data        = new varType[nCols*nRows];
    numRows     = nRows;
    numColumns  = nCols;
    memcpy(data, theData, sizeof(varType) * nCols * nRows);
}

//-----------------------------------------------------------------------------
// Name: insertTable()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::insertTable(table *t, unsigned int toCol, unsigned int toRow)
{
    // table big enough?
    if (this->numColumns < toCol + t->numColumns) return false;
    if (this->numRows    < toRow + t->numRows   ) return false;

    // locals
    unsigned int curCol;

    for (curCol=0; curCol<t->numColumns; curCol++) {
        memcpy(&this->data[(toCol + curCol) * this->numRows + toRow], &t->data[curCol * t->numRows + 0], sizeof(varType) * t->numRows);
    }

    return true;
}

//-----------------------------------------------------------------------------
// Name: setDatum()
// Desc: Updates the value of all cells in the table.
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::setDatum(varType value)
{
	unsigned int columnIndex, rowIndex;
	for (columnIndex = 0; columnIndex < numColumns; columnIndex++) {
		for (rowIndex = 0; rowIndex < numRows; rowIndex++) {
			data[columnIndex * numRows + rowIndex] = value;
		}
	}
    return true;
}

//-----------------------------------------------------------------------------
// Name: setDatum()
// Desc: Updates the value of all cells in the table.
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::setColumn(unsigned int columnIndex, varType value)
{
	if (columnIndex >= numColumns) return false;
	unsigned int rowIndex;
	for (rowIndex = 0; rowIndex < numRows; rowIndex++) {
		data[columnIndex * numRows + rowIndex] = value;
	}
    return true;
}

//-----------------------------------------------------------------------------
// Name: setDatum()
// Desc: Updates the value of a single cell in the table.
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::setDatum(unsigned int columnIndex, unsigned int rowIndex, varType &value)
{
    if (columnIndex >= numColumns) return false;
    if (rowIndex    >= numRows   ) return false;
    data[columnIndex * numRows + rowIndex] = value;
    return true;
}

//-----------------------------------------------------------------------------
// Name: setRow()
// Desc: Updates the value of a single row in the table.
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::setRow(unsigned int columnOffset, unsigned int rowIndex, vector<varType> &dataToInsert)
{
	size_t numCells = dataToInsert.size();
    if (columnOffset+numCells > numColumns) return false;
    if (rowIndex			 >= numRows   ) return false;
	for (unsigned int curCol=0; curCol<numCells; curCol++) {
		data[(curCol+columnOffset) * numRows + rowIndex] = dataToInsert[curCol];
	}
    return true;
}

//-----------------------------------------------------------------------------
// Name: setStopOnError()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> void table<varType>::setStopOnError(bool stopOnError)
{
	this->stopOnError	= stopOnError;
}

//-----------------------------------------------------------------------------
// Name: areKeyValuesLessOrEqual()
// Desc: 
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::areKeyValuesLessOrEqual(vector<unsigned int> &keyColumns, table<varType> &curTable, vector<varType> &keyValuesToCompare, unsigned int curRow, double delta)
{
	// locals
	size_t curKeyCol;								// iterator
	size_t numKeyColumns = 	 keyColumns.size();		//
	varType curValue;								// current considered value

	if (curRow>=curTable.numRows) return false;

	// compare each key column value
	for (curKeyCol=0; curKeyCol<numKeyColumns; curKeyCol++) {

		curValue = curTable.data[keyColumns[curKeyCol]*curTable.numRows+curRow];

		if (curValue+delta < keyValuesToCompare[curKeyCol]) {
			curKeyCol=numKeyColumns;
			break;
		} else if (isEqual(curValue, (double) keyValuesToCompare[curKeyCol], delta)) {
			continue;
		} else {
			break;
		}
	}
		
	return (curKeyCol==numKeyColumns);
}

//-----------------------------------------------------------------------------
// Name: concatenate()
// Desc: Append all tables below each other. All tables must have the same number of columns.
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::concatenate(vector<table<varType>> &tables)
{
	if (!tables.size()) return false;

	// locals
	unsigned int curRow			= 0;
	unsigned int numColumns		= tables[0].numColumns;
	unsigned int totalNumRows	= 0;

	for (int curTableId=0; curTableId<(int)tables.size(); curTableId++) {
		totalNumRows += tables[curTableId].numRows;
		if (numColumns != tables[curTableId].numColumns) return false;
	}

	redim(numColumns, totalNumRows);
	copyColumnNames(tables[0]);

	for (int curTableId=0; curTableId<(int)tables.size(); curTableId++) {
		insertTable(&tables[curTableId], 0, curRow);
		curRow += tables[curTableId].numRows;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: mergeTables()
// Desc: The keyColumns as well as all other rows and columns of all tables are inserted. Everytime the keyColumns have the same values rows from different files are merged together, but not rows within the same file.
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::mergeTables(vector<unsigned int> &keyColumns, vector<table<varType>> &tables, double delta, varType maxValue)
{
	// locals
	size_t curProcessedTable;												// iterator
	size_t numTables							= 	 tables.size();			//
	size_t numKeyColumns						= 	 keyColumns.size();		//
	unsigned int curRowInFinalTable				= 0;						// 
	unsigned int curColInFinalTable				= 0;						// 
	unsigned int numTablesProcessed				= 0;						// 
	size_t curKeyCol;														// iterator
	vector<bool>ascending(keyColumns.size(), true);							// 
	vector<varType> curKeyValues;											// current considered key values
	vector<varType> nextKeyValues;											// next potentially key values 
	vector<unsigned int> curRow;											// current processed row in each table
	vector<bool> isKeyCol;													// 
	vector<vector<unsigned int>> rowsToInsert;								// 

	// sort all tables for faster processing
	for (curProcessedTable=0; curProcessedTable<tables.size(); curProcessedTable++) {
		tables[curProcessedTable].sortTable(keyColumns, ascending, delta);

		/* for debugging
		stringstream ss;
		ss.str(""); ss << "D:\\" << curProcessedTable << ".dat";
		tables[curProcessedTable].writeDataToAsciiFile(ss.str().c_str(), "#", true, false, 0, NULL, true);*/
	}

	// init
	curKeyValues .resize(numKeyColumns, maxValue);
	nextKeyValues.resize(numKeyColumns, maxValue);
	curRow       .resize(numTables, 0);
	rowsToInsert .resize(numTables);
	for (curProcessedTable=0; curProcessedTable<numTables; curProcessedTable++) {
		rowsToInsert[curProcessedTable].resize(tables[curProcessedTable].numRows);
	}

	// search first key values
	for (curProcessedTable=0; curProcessedTable<numTables; curProcessedTable++) {

		// number of columns must be the same in all tables
		for (curKeyCol=0; curKeyCol<numKeyColumns; curKeyCol++) {
			if (keyColumns[curKeyCol] >= tables[curProcessedTable].numColumns) return false;
		}

		// if smaller, then set nextKeyValues to current values
		if (areKeyValuesLessOrEqual(keyColumns, tables[curProcessedTable], curKeyValues, curRow[curProcessedTable], delta)) {
			for (curKeyCol=0; curKeyCol<numKeyColumns; curKeyCol++) {
				curKeyValues[curKeyCol] = tables[curProcessedTable].data[keyColumns[curKeyCol]*tables[curProcessedTable].numRows+curRow[curProcessedTable]];
			}
		}
	}

	// loop until all rows of all tables are processed
	do {

		// process each table in each outer loop
		for (curProcessedTable=0; curProcessedTable<numTables; curProcessedTable++) {
		
			// continue if there are still rows to process
			if (curRow[curProcessedTable] >= tables[curProcessedTable].numRows) continue;

			// current row of the considered table has the current key values, so insert data and goto next row
			if (areKeyValuesLessOrEqual(keyColumns, tables[curProcessedTable], curKeyValues, curRow[curProcessedTable], delta)) {
				rowsToInsert[curProcessedTable][curRow[curProcessedTable]] = curRowInFinalTable;
				curRow[curProcessedTable]++;

				// table completely processed?
				if (curRow[curProcessedTable] >= tables[curProcessedTable].numRows) numTablesProcessed++;
			}

			// continue if there are still rows to process
			if (curRow[curProcessedTable] >= tables[curProcessedTable].numRows) continue;

			// current key values are smaller, so look if current row of table is appropriate as next key value set
			if (areKeyValuesLessOrEqual(keyColumns, tables[curProcessedTable], nextKeyValues, curRow[curProcessedTable], delta)) {
				for (curKeyCol=0; curKeyCol<numKeyColumns; curKeyCol++) { nextKeyValues[curKeyCol] = tables[curProcessedTable].data[keyColumns[curKeyCol]*tables[curProcessedTable].numRows+curRow[curProcessedTable]]; }
			}
		}

		// prepare next key values
		curKeyValues .assign(nextKeyValues.begin(), nextKeyValues.end());
		nextKeyValues.assign(numKeyColumns, maxValue);
		curRowInFinalTable++;

	// all tables processed completely?
	} while (numTablesProcessed < numTables);

	// estimate final table size
	for (curColInFinalTable=0, curProcessedTable=0; curProcessedTable<numTables; curProcessedTable++) {
		curColInFinalTable += tables[curProcessedTable].numColumns - (unsigned int) numKeyColumns;
	}
	this->redim((unsigned int)numKeyColumns+curColInFinalTable, curRowInFinalTable, (varType) 0);

	// copy data as determined in 'rowsToInsert'
	for (curColInFinalTable=(unsigned int) numKeyColumns, curProcessedTable=0; curProcessedTable<numTables; curProcessedTable++) {

		// remember all key columns
		isKeyCol.resize(tables[curProcessedTable].numColumns, false);
		for (curKeyCol=0; curKeyCol<numKeyColumns; curKeyCol++) {
			isKeyCol[keyColumns[curKeyCol]] = true;
		}
		
		// copy each row of the current considered table
		for (curRow[curProcessedTable]=0; curRow[curProcessedTable]<tables[curProcessedTable].numRows; curRow[curProcessedTable]++) {
		
			// copy key columns
			for (curKeyCol=0; curKeyCol<numKeyColumns; curKeyCol++) {
				this->data[curKeyCol*this->numRows+rowsToInsert[curProcessedTable][curRow[curProcessedTable]]] = tables[curProcessedTable].data[keyColumns[curKeyCol]*tables[curProcessedTable].numRows+curRow[curProcessedTable]];
				if (curProcessedTable==0 && curRow[curProcessedTable]==0) this->setColumnName(curKeyCol, tables[curProcessedTable].columnName[curKeyCol]);
			}

			// copy other columns
			for (unsigned int ommitedKeyCols=0, curKeyCol=0; curKeyCol<tables[curProcessedTable].numColumns; curKeyCol++) {
				if (isKeyCol[curKeyCol]) {
					ommitedKeyCols++;
					continue;
				}
				this->data[(curColInFinalTable+curKeyCol-ommitedKeyCols)*this->numRows+rowsToInsert[curProcessedTable][curRow[curProcessedTable]]] = tables[curProcessedTable].data[curKeyCol*tables[curProcessedTable].numRows+curRow[curProcessedTable]];
				if (curRow[curProcessedTable]==0) this->setColumnName((curColInFinalTable+curKeyCol-ommitedKeyCols), tables[curProcessedTable].columnName[curKeyCol]);
			}
		}
		curColInFinalTable += tables[curProcessedTable].numColumns - (unsigned int) numKeyColumns;
	}
		
	return true;
}

//-----------------------------------------------------------------------------
// Name: areTheNextvaluesAboveThreshold()
// Desc: Checks if the next 'numNextValues' values in the considered column are higher than the threshold value.
//-----------------------------------------------------------------------------
template <class varType> bool table<varType>::areTheNextvaluesAboveThreshold(unsigned int columnIndex, unsigned int rowOffset, unsigned int numNextValues, unsigned int numOutliers, varType threshold, bool belowInsteadOfAbove)
{
	// locals
	unsigned int curValue, curOutliers;
	varType * myColumn = &data[columnIndex*numRows + rowOffset];

	// cap
	if (rowOffset + numNextValues > numRows) numNextValues = numRows - rowOffset;

	// check each row
	for (curValue=0, curOutliers=0; curValue<numNextValues; curValue++) {
		if (belowInsteadOfAbove) {
			if (myColumn[curValue] < threshold) continue;
		} else {
			if (myColumn[curValue] > threshold) continue;
		}
		curOutliers++;
		if (curOutliers>numOutliers) {
			return false;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: areTheNextvaluesAboveThreshold()
// Desc: Checks if the next 'numNextValues' values in the considered column are higher than the threshold value.
//-----------------------------------------------------------------------------
template <class varType> unsigned int table<varType>::getNextRowWithValueAbove(unsigned int columnIndex, unsigned int rowOffset, varType threshold, bool belowInsteadOfAbove)
{
	// locals
	unsigned int	curRow		= rowOffset;
	varType *		myColumn	= &data[columnIndex*numRows];

	while (curRow<numRows) {
		if (belowInsteadOfAbove) {
			if (myColumn[curRow] < threshold) break;
		} else {
			if (myColumn[curRow] > threshold) break;
		}
		curRow++;
	}

	return curRow;
}

#endif


