/*********************************************************************
	database.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "database.h"
#include <intrin.h>

#pragma region database
//-----------------------------------------------------------------------------
// Name: database()
// Desc: constructor 
//-----------------------------------------------------------------------------
miniMax::database::database::database(gameInterface& gi, logger& log)
	: game(&gi), log(log), arrayInfos(log)
{
}

//-----------------------------------------------------------------------------
// Name: ~database()
// Desc: destructor 
//-----------------------------------------------------------------------------
miniMax::database::database::~database()
{
	closeDatabase();
}

//-----------------------------------------------------------------------------
// Name: setAsComplete()
// Desc: Set the database as completly calculated 
//-----------------------------------------------------------------------------
bool miniMax::database::database::setAsComplete()
{
	if (!isOpen()) {
		return log.log(logger::logLevel::error, L"ERROR: No database file open!");
	}
	// check if all layers are set as complete
	for (unsigned int layerNumber=0; layerNumber<dbStats.numLayers; layerNumber++) {
		if (!getNumberOfKnots(layerNumber)) {
			continue;
		}
		if (!isLayerCompleteAndInFile(layerNumber)) {
			return log.log(logger::logLevel::error, L"ERROR: Layer " + to_wstring(layerNumber) + L" is not completely calculated and saved in the file!");
		}
	}
	log.log(logger::logLevel::info, L"Database is set as complete.");
	dbStats.completed = true;
	return true;
}

//-----------------------------------------------------------------------------
// Name: setLoadingOfFullLayerOnRead()
// Desc: When a reading operation is performed, the full layer is loaded into memory. 
//-----------------------------------------------------------------------------
bool miniMax::database::database::setLoadingOfFullLayerOnRead()
{
    loadFullLayerOnRead = true;
	return true;
}

//-----------------------------------------------------------------------------
// Name: unload()
// Desc: The database file is kept open, the header information stays, but the data is unloaded from memory.
//-----------------------------------------------------------------------------
void miniMax::database::database::unload()
{
	for (unsigned int layerNumber=0; layerNumber<dbStats.numLayers; layerNumber++) {
		layerStatsStruct& myLss = layerStats[layerNumber];
		
		arrayInfos.removeArray(layerNumber, arrayInfoStruct::arrayType::layerStats, myLss.skv.size() *  sizeof(twoBit), 0);
		myLss.skv.clear();
		myLss.skv.shrink_to_fit();
		myLss.isSkvResized = false;
		
		arrayInfos.removeArray(layerNumber, arrayInfoStruct::arrayType::plyInfos, myLss.plyInfo.size() * sizeof(plyInfoVarType), 0);
		myLss.plyInfo.clear();
		myLss.plyInfo.shrink_to_fit();
		myLss.isPlyInfoResized = false;
	}
	log.log(logger::logLevel::info, L"Database data unloaded from memory.");
}

//-----------------------------------------------------------------------------
// Name: resizePlyInfo()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::database::database::resizeSkv(layerStatsStruct& myLss, unsigned int layerNumber)
{
	// lock the database mutex to prevent other threads from accessing the database while resizing
	std::lock_guard<std::mutex> lock(csDatabaseMutex);

	// check again if layer is already loaded, since another thread might have loaded it in the meantime
	if (!myLss.isSkvResized) {

		// reserve memory for this layer & create array for skv with default value
		myLss.skv.resize((myLss.knotsInLayer + 3) / 4, SKV_WHOLE_BYTE_IS_INVALID);

		// if layer is in database and completed, then load layer from file into memory, set default value otherwise
		if (myLss.completedAndInFile) {
			if (!file->readSkv(layerNumber, myLss.skv)) {
				return log.log(logger::logLevel::error, L"ERROR: Reading skv of layer " + to_wstring(layerNumber) + L" from file failed!");
			}
		}

		if (!arrayInfos.addArray(layerNumber, arrayInfoStruct::arrayType::layerStats, myLss.skv.size() * sizeof(twoBit), 0)) {
			return log.log(logger::logLevel::error, L"ERROR: Adding array to arrayInfos failed!");
		}

		myLss.isSkvResized = true;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: resizePlyInfo()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::database::database::resizePlyInfo(layerStatsStruct& myLss, unsigned int layerNumber)
{
	// lock the database mutex to prevent other threads from accessing the database while resizing
	std::lock_guard<std::mutex> lock(csDatabaseMutex);
	
	// check again if layer is already loaded, since another thread might have loaded it in the meantime
	if (!myLss.isPlyInfoResized) {
		
		// reserve memory for this layer & create array for ply info with default value
		myLss.plyInfo.resize(myLss.knotsInLayer, PLYINFO_VALUE_UNCALCULATED);	
		
		// if layer is in database and completed, then load layer from file into memory; set default value otherwise
		if (myLss.completedAndInFile) {
			if (!file->readPlyInfo(layerNumber, myLss.plyInfo)) {
				return log.log(logger::logLevel::error, L"ERROR: Reading ply info of layer " + to_wstring(layerNumber) + L" from file failed!");
			}
		}

		// statistics
		if (!arrayInfos.addArray(layerNumber, arrayInfoStruct::arrayType::plyInfos, myLss.plyInfo.size() * sizeof(plyInfoVarType), 0)) {
			return log.log(logger::logLevel::error, L"ERROR: Adding array to arrayInfos failed!");
		}

		myLss.isPlyInfoResized = true;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: openDatabase()
// Desc: Open the database file and load the header information.
//-----------------------------------------------------------------------------
bool miniMax::database::database::openDatabase(wstring const &fileDirectory, bool useCompFileIfBothExist)
{
	// do not open the database if it is already open
	if (file) {
		return log.log(logger::logLevel::error, L"ERROR: Database file already open!");
	}

	// check if database path is valid
	if (fileDirectory.size() && !filesystem::exists(fileDirectory)) {
		return log.log(logger::logLevel::error, wstring{L"ERROR: Database path "} + fileDirectory + wstring{L" not valid!"});
	}

	// check which database file exists
	bool hasComp 	= filesystem::exists(fileDirectory + L"\\database.dat");
	bool hasUncomp 	= filesystem::exists(fileDirectory + L"\\shortKnotValue.dat") && filesystem::exists(fileDirectory + L"\\plyInfo.dat");
	if (hasComp && (!hasUncomp || (hasUncomp && useCompFileIfBothExist))) {
		log.log(logger::logLevel::info, L"Using compressed database file.");
		file = new compFile{game, log};
	} else if (hasUncomp && (!hasComp || (hasComp && !useCompFileIfBothExist))) {
		log.log(logger::logLevel::info, L"Using uncompressed database files.");
		file = new uncompFile{game, log};
	} else {
		log.log(logger::logLevel::info, L"No database file found, a new one will be created.");
		file = new uncompFile{game, log};
	}

	// open and load header
	if (!file->openDatabase(fileDirectory)) {
		delete file;
		file = nullptr;
		return log.log(logger::logLevel::error, L"ERROR: Opening database file " + fileDirectory + L" failed!");
	}
	if (!file->loadHeader(dbStats, layerStats)) {
		delete file;
		file = nullptr;
		return log.log(logger::logLevel::error, L"ERROR: Loading database header failed!");
	}
	arrayInfos.init(getNumLayers());
	log.log(logger::logLevel::info, L"Database opened.");

	return true;
}

//-----------------------------------------------------------------------------
// Name: closeDatabase()
// Desc: Close the database file and unload all data from memory.
//-----------------------------------------------------------------------------
bool miniMax::database::database::closeDatabase()
{
    if (!file) { 
		log.log(logger::logLevel::info, L"Skipping closeDatabase(): No database file open!");
		return false;
	}
	delete file; 
	file = nullptr; 
	unload(); 
	return true; 
}

//-----------------------------------------------------------------------------
// Name: saveHeader()
// Desc: Save the header information to the database file.
//-----------------------------------------------------------------------------
bool miniMax::database::database::saveHeader()
{
    if (!file) { 
		return log.log(logger::logLevel::error, L"ERROR: No database file open!");
	}
	return file->saveHeader(dbStats, layerStats);
}

//-----------------------------------------------------------------------------
// Name: removeDatabaseFiles()
// Desc: Remove the database files from the file system.
//-----------------------------------------------------------------------------
bool miniMax::database::database::removeDatabaseFiles()
{
    if (!file) { 
		return log.log(logger::logLevel::error, L"ERROR: No database file open!");
	}
	return file->removeFile(file->getFileDirectory());
}
#pragma endregion

#pragma region statistics
//-----------------------------------------------------------------------------
// Name: updateLayerStats()
// Desc: Count the number of states in the layer - the number of won, lost, drawn and invalid states
//-----------------------------------------------------------------------------
bool miniMax::database::database::updateLayerStats(unsigned int layerNumber)
{
	// checks
	if (layerNumber >= layerStats.size()) {
		return log.log(logger::logLevel::error, L"ERROR: Layer " + to_wstring(layerNumber) + L" does not exist!");
	}

	// locals
	stateAdressStruct	curState;
	unsigned int		statsValueCounter[]	= {0,0,0,0};
	twoBit		  		curStateValue;

	// calc and show statistics
	for (curState.layerNumber=layerNumber, curState.stateNumber=0; curState.stateNumber<getNumberOfKnots(curState.layerNumber); curState.stateNumber++) {
		
		// get state value
		if (!readKnotValueFromDatabase(curState.layerNumber, curState.stateNumber, curStateValue)) {
			return log.log(logger::logLevel::error, L"ERROR: Reading knot value from database failed!");
		}
		statsValueCounter[curStateValue]++;
	}

	// store statistics
	layerStats[layerNumber].numWonStates		= statsValueCounter[SKV_VALUE_GAME_WON  ];
	layerStats[layerNumber].numLostStates		= statsValueCounter[SKV_VALUE_GAME_LOST ];
	layerStats[layerNumber].numDrawnStates		= statsValueCounter[SKV_VALUE_GAME_DRAWN];
	layerStats[layerNumber].numInvalidStates	= statsValueCounter[SKV_VALUE_INVALID   ];

	log << L"Statistics of layer " << layerNumber << L" updated." << "\n";
	return true;
}

//-----------------------------------------------------------------------------
// Name: showLayerStats()
// Desc: Print the statistics of the layer to the log
//-----------------------------------------------------------------------------
void miniMax::database::database::showLayerStats(unsigned int layerNumber) 
{
	if (layerNumber >= layerStats.size()) {
		log.log(logger::logLevel::error, L"ERROR: showLayerStats(): Layer " + to_wstring(layerNumber) + L" does not exist!");
		return;
	}

	log << "STATISTICS OF LAYER " << layerNumber << "\n";
	log << (game->getOutputInformation(layerNumber)) << "\n";
	log << " number  states: " << getNumberOfKnots(layerNumber) << "\n";
	log << " won     states: " << getNumWonStates(layerNumber) << "\n";
	log << " lost    states: " << getNumLostStates(layerNumber) << "\n";
	log << " draw    states: " << getNumDrawnStates(layerNumber) << "\n";
	log << " invalid states: " << getNumInvalidStates(layerNumber) << "\n";
}
#pragma endregion

#pragma region getter
//-----------------------------------------------------------------------------
// Name: isComplete()
// Desc: Returns true if all layers of the database are completely calculated.
//-----------------------------------------------------------------------------
bool miniMax::database::database::isComplete()
{
	return dbStats.completed;
}

//-----------------------------------------------------------------------------
// Name: isLayerCompleteAndInFile()
// Desc: Returns true if the layer is completely calculated and saved in the file. 
//-----------------------------------------------------------------------------
bool miniMax::database::database::isLayerCompleteAndInFile(unsigned int layerNumber)
{
	if (layerNumber >= layerStats.size()) {
		return log.log(logger::logLevel::error, L"ERROR: Layer " + to_wstring(layerNumber) + L" does not exist!");
	}
	return layerStats[layerNumber].completedAndInFile;
}

//-----------------------------------------------------------------------------
// Name: getNumberOfKnots()
// Desc: Returns the number of knots in the layer.
//-----------------------------------------------------------------------------
unsigned int miniMax::database::database::getNumberOfKnots(unsigned int layerNumber)
{
	if (layerNumber >= layerStats.size()) return 0;
	return layerStats[layerNumber].knotsInLayer;
}

//-----------------------------------------------------------------------------
// Name: getLayerSizeInBytes()
// Desc: Returns the size of the layer in bytes, which might differ from the number of knots.
//-----------------------------------------------------------------------------
long long miniMax::database::database::getLayerSizeInBytes(unsigned int layerNum)
{
	if (layerNum >= layerStats.size()) return 0;
	return layerStats[layerNum].getLayerSizeInBytesForSkv() + layerStats[layerNum].getLayerSizeInBytesForPlyInfo();
}

//-----------------------------------------------------------------------------
// Name: getNumWonStates()
// Desc: Returns the number of won states in the layer. 
//-----------------------------------------------------------------------------
miniMax::stateNumberVarType miniMax::database::database::getNumWonStates(unsigned int layerNum)
{
	if (layerNum >= layerStats.size()) return 0;
	return layerStats[layerNum].numWonStates;
}

//-----------------------------------------------------------------------------
// Name: getNumLostStates()
// Desc: Returns the number of lost states in the layer.
//-----------------------------------------------------------------------------
miniMax::stateNumberVarType miniMax::database::database::getNumLostStates(unsigned int layerNum)
{
	if (layerNum >= layerStats.size()) return 0;
	return layerStats[layerNum].numLostStates;
}

//-----------------------------------------------------------------------------
// Name: getNumDrawnStates()
// Desc: Returns the number of drawn states in the layer.
//-----------------------------------------------------------------------------
miniMax::stateNumberVarType miniMax::database::database::getNumDrawnStates(unsigned int layerNum)
{
	if (layerNum >= layerStats.size()) return 0;
	return layerStats[layerNum].numDrawnStates;
}

//-----------------------------------------------------------------------------
// Name: getNumInvalidStates()
// Desc: Returns the number of invalid states in the layer.
//-----------------------------------------------------------------------------
miniMax::stateNumberVarType miniMax::database::database::getNumInvalidStates(unsigned int layerNum)
{
	if (layerNum >= layerStats.size()) return 0;
	return layerStats[layerNum].numInvalidStates;
}
#pragma endregion

#pragma region read and write operations
//-----------------------------------------------------------------------------
// Name: saveLayerToFile()
// Desc: Save the layer to the file, which is already in memory.
//		 The layer is marked as completed and saved in the file.
// 		 The layer is not saved if it is empty.
//		 The header is not saved, and must be saved separately.
//-----------------------------------------------------------------------------
bool miniMax::database::database::saveLayerToFile(unsigned int layerNumber)
{
	// checks
	if (!file || !isOpen()) {
		return log.log(logger::logLevel::error, L"ERROR: No database file open!");
	}
	if (layerNumber >= layerStats.size()) {
		return log.log(logger::logLevel::error, L"ERROR: Layer " + to_wstring(layerNumber) + L" does not exist!");
	}

	// don't save layer and header when only preparing layers
	layerStatsStruct& myLss = layerStats[layerNumber];

	// save layer if there are any states
	if (!myLss.skv.size()) {
		return log.log(logger::logLevel::error, L"ERROR: Layer " + to_wstring(layerNumber) + L" is empty!");
	}
	if (!myLss.plyInfo.size()) {
		return log.log(logger::logLevel::error, L"ERROR: Ply info of layer " + to_wstring(layerNumber) + L" is empty!");
	}

	curAction = activity::savingLayerToFile;
	log << L"Saving layer " << layerNumber << L" to file..." << "\n";

	// write layer to file
	if (!file->writeSkv    (layerNumber, myLss.skv    )) {
		return log.log(logger::logLevel::error, L"ERROR: Writing skv of layer " + to_wstring(layerNumber) + L" to file failed!");
	}
	if (!file->writePlyInfo(layerNumber, myLss.plyInfo)) {
		return log.log(logger::logLevel::error, L"ERROR: Writing ply info of layer " + to_wstring(layerNumber) + L" to file failed!");
	}

	// mark layer as completed
	log << L"Layer " << layerNumber << L" saved to file." << "\n";
	myLss.completedAndInFile = true;
	return true;
}

//-----------------------------------------------------------------------------
// Name: loadLayerFromFile()
// Desc: Load the layer from the file into memory.
//		 The layer must be marked as completed and already saved in the file.
// 		 The layer is not load if it not complete.
//		 The header is not loaded, and must be loaded in advance.
//-----------------------------------------------------------------------------
bool miniMax::database::database::loadLayerFromFile(unsigned int layerNumber)
{
	// checks
	if (!file || !isOpen()) {
		return log.log(logger::logLevel::error, L"ERROR: No database file open!");
	}
	if (layerNumber >= layerStats.size()) {
		return log.log(logger::logLevel::error, L"ERROR: Layer " + to_wstring(layerNumber) + L" does not exist!");
	}

	layerStatsStruct& myLss = layerStats[layerNumber];

	// don't load layer if not complete
	if (!myLss.completedAndInFile) {
		return log.log(logger::logLevel::error, L"ERROR: Layer " + to_wstring(layerNumber) + L" is not completely calculated and saved in the file!");
	}

	curAction = activity::loadingLayerFromFile;
	log << L"Loading layer " << layerNumber << L" from file..." << "\n";

	// load layer from file
	resizeSkv(myLss, layerNumber);
	resizePlyInfo(myLss, layerNumber);

	log << L"Layer " << layerNumber << L" loaded from file." << "\n";
	return true;
}

//-----------------------------------------------------------------------------
// Name: readKnotValueFromDatabase()
// Desc: Read the knot value from the database.
//	  	 If the layer is in memory, the data is read from memory.
//	  	 If the layer is not in memory, the data is loaded from the file into memory.
//       Apart from changes in the header information, reading is thread safe.
//-----------------------------------------------------------------------------
bool miniMax::database::database::readKnotValueFromDatabase(unsigned int layerNumber, unsigned int stateNumber, twoBit &knotValue)
{
	// checks
	if (layerNumber >= layerStats.size() || layerNumber > dbStats.numLayers) {
		knotValue = SKV_VALUE_INVALID;
		return log.log(logger::logLevel::error, L"ERROR: INVALID layerNumber in readKnotValueFromDatabase()!");
	}
	if (stateNumber >= layerStats[layerNumber].knotsInLayer) {
		knotValue = SKV_VALUE_INVALID;
		return log.log(logger::logLevel::error, L"ERROR: INVALID stateNumber in readKnotValueFromDatabase()!");
	}

	// locals
    twoBit				databaseByte;
	twoBit				defValue	= SKV_WHOLE_BYTE_IS_INVALID;
	layerStatsStruct&  	myLss		= layerStats[layerNumber];

	// valid state and layer number ?
	if (stateNumber >= myLss.knotsInLayer) {
		knotValue = SKV_VALUE_INVALID;
		return log.log(logger::logLevel::error, L"ERROR: INVALID stateNumber in readKnotValueFromDatabase()!");
	}

	//  if database is complete get just single byte from file directly
	if ((dbStats.completed || myLss.completedAndInFile) && !loadFullLayerOnRead) {
		std::lock_guard<std::mutex> lock(csDatabaseMutex);
		file->readSkv(layerNumber, databaseByte, stateNumber);
	} else {

		// if layer not already loaded
		if (!myLss.isSkvResized) {
			resizeSkv(myLss, layerNumber);
		}

		// read knot value from array
		databaseByte = myLss.skv[stateNumber / 4];
	
		// measure io-operations per second
		if (MEASURE_IOPS) speedoReadSkv.measureIops();
	}

    // make half byte
    knotValue    = _rotr8(databaseByte, 2 * (stateNumber % 4)) & 3;

	return true;	
}

//-----------------------------------------------------------------------------
// Name: readPlyInfoFromDatabase()
// Desc: Read the ply info from the database.
//	  	 If the layer is in memory, the data is read from memory.
//	  	 If the layer is not in memory, the data is loaded from the file into memory.
//       Apart from changes in the header information, reading is thread safe.
//-----------------------------------------------------------------------------
bool miniMax::database::database::readPlyInfoFromDatabase(unsigned int layerNumber, unsigned int stateNumber, plyInfoVarType &value)
{
	// checks
	if (layerNumber >= layerStats.size() || layerNumber > dbStats.numLayers) {
		value = PLYINFO_VALUE_INVALID;
		return log.log(logger::logLevel::error, L"ERROR: INVALID layerNumber in readPlyInfoFromDatabase()!");
	}
	if (stateNumber >= layerStats[layerNumber].knotsInLayer) {
		value = PLYINFO_VALUE_INVALID;
		return log.log(logger::logLevel::error, L"ERROR: INVALID stateNumber in readPlyInfoFromDatabase()!");
	}

	// locals
	layerStatsStruct&  	myLss = layerStats[layerNumber];

	// valid state and layer number ?
	if (stateNumber > myLss.knotsInLayer) {
		value = PLYINFO_VALUE_INVALID;
		return log.log(logger::logLevel::error, L"ERROR: INVALID stateNumber in readPlyInfoFromDatabase()!");
	}

	// if database is complete get whole byte from file
	if ((dbStats.completed || myLss.completedAndInFile) && !loadFullLayerOnRead) {
		std::lock_guard<std::mutex> lock(csDatabaseMutex);
		file->readPlyInfo(layerNumber, value, stateNumber);
	} else {

		// is layer already in memory?
		if (!myLss.isPlyInfoResized) {
			resizePlyInfo(myLss, layerNumber);
		}

		// read ply info from array
		value = myLss.plyInfo[stateNumber];

		// measure io-operations per second
		if (MEASURE_IOPS) speedoReadPly.measureIops();
	}

	return true;	
}

//-----------------------------------------------------------------------------
// Name: writeKnotValueInDatabase()
// Desc: Save the knot value in the database.
//	  	 If the layer is in memory, the data is saved to memory.
//	  	 If the layer is not in memory, the data is loaded from the file into memory.
//       Apart from changes in the header information, writing is thread safe.
//       If the layer is already completed and in the file, the function returns false.
//-----------------------------------------------------------------------------
bool miniMax::database::database::writeKnotValueInDatabase(unsigned int layerNumber, unsigned int stateNumber, twoBit knotValue)
{
	// checks
	if (layerNumber >= layerStats.size() || layerNumber > dbStats.numLayers) {
		return log.log(logger::logLevel::error, L"ERROR: INVALID layerNumber in writeKnotValueInDatabase()!");
	}
	if (knotValue >= SKV_NUM_VALUES) {
		return log.log(logger::logLevel::error, L"ERROR: INVALID knotValue in writeKnotValueInDatabase()!");
	}
	if (stateNumber >= layerStats[layerNumber].knotsInLayer) {
		return log.log(logger::logLevel::error, L"ERROR: INVALID stateNumber in writeKnotValueInDatabase()!");
	}

	// locals
	twoBit				defValue	= SKV_WHOLE_BYTE_IS_INVALID;
	layerStatsStruct&  	myLss		= layerStats[layerNumber];

	// valid state and layer number ?
	if (stateNumber >= myLss.knotsInLayer) {
		return log.log(logger::logLevel::error, L"ERROR: INVALID stateNumber in writeKnotValueInDatabase()!");
	}

	// is layer already completed ?
	if (myLss.completedAndInFile) {
		return log.log(logger::logLevel::error, L"ERROR: layer already completed and in file! function: writeKnotValueInDatabase()!");
	}

    // is layer already loaded?
	if (!myLss.skv.size()) {
		resizeSkv(myLss, layerNumber);
	}
	
	// set value
	long *	pShortKnotValue	= ((long*) &myLss.skv[0]) + stateNumber / ((sizeof(long)*8) / 2);
	long	numBitsToShift	= 2 * (stateNumber % ((sizeof(long)*8) / 2));		// little-endian byte-order
	long	mask			= 0x00000003 << numBitsToShift;
	long	curShortKnotValueLong, newShortKnotValueLong;

	do {
		curShortKnotValueLong	= *pShortKnotValue;
		newShortKnotValueLong	= (curShortKnotValueLong & (~mask)) + (knotValue << numBitsToShift);
	} while (InterlockedCompareExchange(pShortKnotValue, newShortKnotValueLong, curShortKnotValueLong) != curShortKnotValueLong);

	// measure io-operations per second
	if (MEASURE_IOPS) speedoWriteSkv.measureIops();

	return true;
}

//-----------------------------------------------------------------------------
// Name: writePlyInfoInDatabase()
// Desc: Save the ply info in the database.
//	  	 If the layer is in memory, the data is saved to memory.
//	  	 If the layer is not in memory, the data is loaded from the file into memory.
//       Apart from changes in the header information, writing is thread safe.
//       If the layer is already completed and in the file, the function returns false.
//-----------------------------------------------------------------------------
bool miniMax::database::database::writePlyInfoInDatabase(unsigned int layerNumber, unsigned int stateNumber, plyInfoVarType value)
{
	// checks
	if (layerNumber >= layerStats.size() || layerNumber > dbStats.numLayers) {
		return log.log(logger::logLevel::error, L"ERROR: INVALID layerNumber in writePlyInfoInDatabase()!");
	}
	if (stateNumber >= layerStats[layerNumber].knotsInLayer) {
		return log.log(logger::logLevel::error, L"ERROR: INVALID stateNumber in writePlyInfoInDatabase()!");
	}
	if ((value > PLYINFO_EXP_VALUE && value < PLYINFO_VALUE_DRAWN) || value > PLYINFO_VALUE_INVALID) {
		return log.log(logger::logLevel::error, L"ERROR: INVALID value in writePlyInfoInDatabase()!");
	}

	// locals
	layerStatsStruct&  	myLss = layerStats[layerNumber];

	// valid state and layer number ?
	if (stateNumber >= myLss.knotsInLayer) {
		return log.log(logger::logLevel::error, L"ERROR: INVALID stateNumber in writePlyInfoInDatabase()!");
	}

	// is layer already completed ?
	if (myLss.completedAndInFile) {
		return log.log(logger::logLevel::error, L"ERROR: layer already completed and in file! function: writePlyInfoInDatabase()!");
	}

    // is layer already loaded
	if (!myLss.plyInfo.size()) {
		resizePlyInfo(myLss, layerNumber);
	}

	// set value
	myLss.plyInfo[stateNumber] = value;

	// measure io-operations per second
	if (MEASURE_IOPS) speedoWritePly.measureIops();

	return true;
}
#pragma endregion
