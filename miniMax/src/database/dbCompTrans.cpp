/*********************************************************************
	dbCompTrans.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "dbCompTrans.h"

namespace miniMax
{

//-----------------------------------------------------------------------------
// Name: dbCompTrans()
// Desc: 
//-----------------------------------------------------------------------------
dbCompTrans::dbCompTrans()
{
}

//-----------------------------------------------------------------------------
// Name: ~dbCompTrans()
// Desc: 
//-----------------------------------------------------------------------------
dbCompTrans::~dbCompTrans()
{
}

//-----------------------------------------------------------------------------
// Name: calcCheckSum()
// Desc: 
//-----------------------------------------------------------------------------
long long dbCompTrans::calcCheckSum(void* pByte, size_t numBytes)
{
	long long	checkSum	= 0;
	long long*	pLongLong	= reinterpret_cast<long long*>(pByte);
	size_t      padBytes    = numBytes % sizeof(long long);

	for (size_t p = 0; p < numBytes - padBytes; p += sizeof(long long), pLongLong++) {
		checkSum += *pLongLong;
	}
	char*		pChar		= reinterpret_cast<char*>(pLongLong);

	for (size_t p = 0; p < padBytes; p += sizeof(char), pChar++) {
		checkSum += (long long) *pChar;
	}

	return checkSum;
}

//-----------------------------------------------------------------------------
// Name: calcCheckSum()
// Desc:  
//-----------------------------------------------------------------------------
bool dbCompTrans::readFromFileAndCalcCheckSum(database::genericFile& file, unsigned int layerNum, vector<database::layerStatsStruct>& layerStats, long long& checkSumSkv, long long& checkSumPlyInfo)
{
	wcout << endl << "Read short knot value from layer " << layerNum << endl;
	wcout << "  Number states:   " << layerStats[layerNum].knotsInLayer << endl;
	layerStats[layerNum].skv.assign((layerStats[layerNum].knotsInLayer + 3) / 4, SKV_WHOLE_BYTE_IS_INVALID);
	wcout << "  Number of bytes: " << layerStats[layerNum].skv.size() * sizeof(twoBit) << endl;
	if (!file.readSkv (layerNum, layerStats[layerNum].skv)) {
		wcout << "ERROR: Could not read skv of layer " << layerNum << endl;
		return false;
	}
	checkSumSkv = calcCheckSum(&layerStats[layerNum].skv[0], layerStats[layerNum].skv.size() * sizeof(twoBit));

	wcout << "Read ply info from layer " << layerNum << endl;
	wcout << "  Number states:   " << layerStats[layerNum].knotsInLayer << endl;
	layerStats[layerNum].plyInfo.assign(layerStats[layerNum].knotsInLayer, PLYINFO_VALUE_INVALID);
	wcout << "  Number of bytes: " << layerStats[layerNum].plyInfo.size() * sizeof(plyInfoVarType) << endl;
	if (!file.readPlyInfo (layerNum, layerStats[layerNum].plyInfo)) {
		wcout << "ERROR: Could not read ply info of layer " << layerNum << endl;
		return false;
	}
	checkSumPlyInfo = calcCheckSum(&layerStats[layerNum].plyInfo[0], layerStats[layerNum].plyInfo.size() * sizeof(plyInfoVarType));
	return true;
}

//-----------------------------------------------------------------------------
// Name: printError()
// Desc: 
//-----------------------------------------------------------------------------
bool dbCompTrans::printError(wstring const& message)
{
	wcout << endl << message << endl;
	return false;
}

//-----------------------------------------------------------------------------
// Name: checkIfEqual()
// Desc: Template version to make file types configurable
//-----------------------------------------------------------------------------
template <typename F, typename T>
bool dbCompTrans::checkIfEqual(wstring const& fromFilepath, wstring const& toFilepath)
{
	// locals
	long long							checkSumSkvA,		checkSumSkvB;
	long long							checkSumPlyInfoA,	checkSumPlyInfoB;
	database::databaseStatsStruct 		dbStatsA, dbStatsB;
	vector<database::layerStatsStruct> 	layerStatsA, layerStatsB;
	gameInterface						game;
	logger								log		(logger::logLevel::info, logger::logType::console, L"");

	F fileA(&game, log);
	T fileB(&game, log);

	wcout << "Compare two databases ..." << endl;
	wcout << "  Database A: " << fromFilepath << endl;
	wcout << "  Database B: " << toFilepath   << endl;

	// open databases
	if (!fileA.openDatabase(fromFilepath)) 			return printError(wstring(L"ERROR: Could not open database A for comparison."));
	if (!fileB.openDatabase(toFilepath)) 			return printError(wstring(L"ERROR: Could not open database B for comparison."));
	if (!fileA.loadHeader(dbStatsA, layerStatsA)) 	return printError(wstring(L"ERROR: Could not load header of database A."));
	if (!fileB.loadHeader(dbStatsB, layerStatsB)) 	return printError(wstring(L"ERROR: Could not load header of database B."));

	// compare main database properties
	if (dbStatsA.completed != dbStatsB.completed) 	return printError(wstring(L"ERROR: dbStatsB.completed differ!"));
	if (dbStatsA.numLayers != dbStatsB.numLayers) 	return printError(wstring(L"ERROR: dbStatsB.numLayers differ!"));
	if (layerStatsA.size() != layerStatsB.size()) 	return printError(wstring(L"ERROR: Number of layers differ!"));

	// compare each layer
	for (unsigned int layerNum = 0; layerNum < layerStatsA.size(); layerNum++) {

		// skip empty layers
		if (layerStatsB[layerNum].knotsInLayer == 0 || layerStatsA[layerNum].knotsInLayer == 0) continue;

		// compare layer properties
		if (layerStatsA[layerNum].completedAndInFile != layerStatsB[layerNum].completedAndInFile ) return printError(wstring(L"ERROR: layerStatsB[layerNum].completedAndInFile  of layer ") + to_wstring(layerNum) + wstring(L" differ!"));
		if (layerStatsA[layerNum].knotsInLayer		 != layerStatsB[layerNum].knotsInLayer		 ) return printError(wstring(L"ERROR: layerStatsB[layerNum].knotsInLayer		of layer ") + to_wstring(layerNum) + wstring(L" differ!"));
		if (layerStatsA[layerNum].partnerLayer		 != layerStatsB[layerNum].partnerLayer		 ) return printError(wstring(L"ERROR: layerStatsB[layerNum].partnerLayer		of layer ") + to_wstring(layerNum) + wstring(L" differ!"));
		if (layerStatsA[layerNum].partnerLayers		 != layerStatsB[layerNum].partnerLayers		 ) return printError(wstring(L"ERROR: layerStatsB[layerNum].partnerLayers		of layer ") + to_wstring(layerNum) + wstring(L" differ!"));
		if (layerStatsA[layerNum].succLayers		 != layerStatsB[layerNum].succLayers		 ) return printError(wstring(L"ERROR: layerStatsB[layerNum].succLayers          of layer ") + to_wstring(layerNum) + wstring(L" differ!"));
		if (layerStatsA[layerNum].numWonStates		 != layerStatsB[layerNum].numWonStates		 ) return printError(wstring(L"ERROR: layerStatsB[layerNum].numWonStates		of layer ") + to_wstring(layerNum) + wstring(L" differ!"));
		if (layerStatsA[layerNum].numLostStates		 != layerStatsB[layerNum].numLostStates		 ) return printError(wstring(L"ERROR: layerStatsB[layerNum].numLostStates		of layer ") + to_wstring(layerNum) + wstring(L" differ!"));
		if (layerStatsA[layerNum].numDrawnStates	 != layerStatsB[layerNum].numDrawnStates	 ) return printError(wstring(L"ERROR: layerStatsB[layerNum].numDrawnStates	    of layer ") + to_wstring(layerNum) + wstring(L" differ!"));
		if (layerStatsA[layerNum].numInvalidStates	 != layerStatsB[layerNum].numInvalidStates	 ) return printError(wstring(L"ERROR: layerStatsB[layerNum].numInvalidStates	of layer ") + to_wstring(layerNum) + wstring(L" differ!"));

		// process only non-empty layers
		if (layerStatsB[layerNum].knotsInLayer == 0 || layerStatsA[layerNum].knotsInLayer == 0) continue;

		// read from file to generate checksum
		readFromFileAndCalcCheckSum(fileA, layerNum, layerStatsA, checkSumSkvA, checkSumPlyInfoA);
		readFromFileAndCalcCheckSum(fileB, layerNum, layerStatsB, checkSumSkvB, checkSumPlyInfoB);

		// free memory
		vector<twoBit        >().swap( layerStatsA[layerNum].skv     );
		vector<plyInfoVarType>().swap( layerStatsA[layerNum].plyInfo );
		vector<twoBit        >().swap( layerStatsB[layerNum].skv     );
		vector<plyInfoVarType>().swap( layerStatsB[layerNum].plyInfo );

		// compare checksums
		if (checkSumSkvA	 != checkSumSkvB    ) return printError(wstring(L"ERROR: Short knot value of layer ") + to_wstring(layerNum) + wstring(L" differ!"));
		if (checkSumPlyInfoA != checkSumPlyInfoB) return printError(wstring(L"ERROR: Ply info of layer ")         + to_wstring(layerNum) + wstring(L" differ!"));
		wcout << "Checksums for layer " << layerNum << " are equal." << endl;
	}

	// close databases
	fileA.closeDatabase();
	fileB.closeDatabase();

	wcout << "Both databases are equal." << endl;
	return true;
}

//-----------------------------------------------------------------------------
// Name: compressDataBase()
// Desc: Compress a uncompressed database
//-----------------------------------------------------------------------------
bool dbCompTrans::compressDataBase(wstring const& fromFilepath, wstring const& toFilepath)
{
	return transferDataBase<database::uncompFile, database::compFile>(fromFilepath, toFilepath);
}

//-----------------------------------------------------------------------------
// Name: transferDataBase()
// Desc: Template version to make file types configurable
//-----------------------------------------------------------------------------
template <typename F, typename T> 
bool dbCompTrans::transferDataBase(wstring const& fromFilepath, wstring const& toFilepath)
{
	// locals
	gameInterface						game;
	logger								log										(logger::logLevel::info, logger::logType::console, L"");
	F									fileFrom								(&game, log);
	T									fileTo  								(&game, log);

	return transferDataBase<F, T>(fromFilepath, toFilepath, fileFrom, fileTo);
}

//-----------------------------------------------------------------------------
// Name: compressDataBase()
// Desc: Transfer data from one database to another
//-----------------------------------------------------------------------------
template <typename F, typename T>
bool dbCompTrans::transferDataBase(wstring const& fromFilepath, wstring const& toFilepath, database::genericFile& fileFrom, database::genericFile& fileTo)
{
	// locals
	database::databaseStatsStruct		dbStats;		// general information about the database
	vector<database::layerStatsStruct>	layerStats;		// information about each layer of the database

	// remove destination database
	fileTo.removeFile(toFilepath);

	// open both databases
	if (!fileTo  .openDatabase(toFilepath)) 			return printError(wstring(L"ERROR: Could not open destination database in folder ") + toFilepath);
	if (!fileFrom.openDatabase(fromFilepath)) 			return printError(wstring(L"ERROR: Could not open source database in folder ") + fromFilepath);
	if (!fileTo  .loadHeader(dbStats, layerStats)) 		return printError(wstring(L"ERROR: Could not load header of destination database."));
	if (!fileFrom.loadHeader(dbStats, layerStats)) 		return printError(wstring(L"ERROR: Could not load header of source database."));

	// save the header
	if (!fileTo  .saveHeader(dbStats,   layerStats)) 	return printError(wstring(L"ERROR: Could not save header of destination database."));

	// show some information
	wcout << "From database:    " << fromFilepath << endl;
	wcout << "To   database:    " << toFilepath   << endl;
	wcout << "Number of layers: " << dbStats.numLayers << endl;

	// copy each short know value layer by layer
	for (unsigned int layerNum = 0; layerNum < layerStats.size(); layerNum++) {

		// process only non-empty layers
		if (!layerStats[layerNum].knotsInLayer) continue;

		// skip layer if it only contains invalid states
		if (layerStats[layerNum].numInvalidStates == layerStats[layerNum].knotsInLayer) {
			wcout << endl << "Skip layer " << layerNum << " since it only contains invalid states." << endl;
			layerStats[layerNum].knotsInLayer = 0;
			layerStats[layerNum].numInvalidStates = 0;
			continue;
		}

		// read
		long long checkSumSkv, checkSumPlyInfo;
		readFromFileAndCalcCheckSum(fileFrom, layerNum, layerStats, checkSumSkv, checkSumPlyInfo);

		// write
		wcout << "Write short knot value to  layer " << layerNum << endl;
		fileTo  .writeSkv(layerNum, layerStats[layerNum].skv);
		wcout << "Write ply info to  layer " << layerNum << endl;
		fileTo  .writePlyInfo(layerNum, layerStats[layerNum].plyInfo);

		// free memory
		vector<twoBit        >().swap( layerStats[layerNum].skv     );
		vector<plyInfoVarType>().swap( layerStats[layerNum].plyInfo );
	}

	// save the header
	if (!fileTo.saveHeader(dbStats, layerStats)) return printError(wstring(L"ERROR: Could not save header of destination database after writing layers."));

	// close both databases
	fileFrom.closeDatabase();
	fileTo  .closeDatabase();
	layerStats.clear();

	// compare both databases
	if (!checkIfEqual<F, T>(fromFilepath, toFilepath)) return printError(wstring(L"ERROR: Database checksum error. Leaving program."));

	wcout << endl << "Copying database finished." << endl;
	return true;
}

} // namespace miniMax
