/*********************************************************************
	databaseFile.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#include "databaseFile.h"

#pragma region uncompressed database - uncompFile
//-----------------------------------------------------------------------------
// Name: uncompFile()
// Desc: Constructor
//-----------------------------------------------------------------------------
miniMax::database::uncompFile::uncompFile(gameInterface* game, logger& log) :
	genericFile{game, log}
{
}

//-----------------------------------------------------------------------------
// Name: ~uncompFile()
// Desc: Destructor
//-----------------------------------------------------------------------------
miniMax::database::uncompFile::~uncompFile()
{
	closeDatabase();
}

//-----------------------------------------------------------------------------
// Name: openDatabase()
// Desc: Opens the database allowing read and write operations.
// 		 Fails if the database is already open.
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::openDatabase(wstring const &fileDirectory)
{
	// Fails if the database is already open.
	if (isOpen()) return log.log(logger::logLevel::error, L"Database is already open.");

	// remember the directory
	this->fileDirectory = fileDirectory;

	// get file name
	wstringstream	ssDatabaseFile;
	ssDatabaseFile << fileDirectory << (fileDirectory.size()?"\\":"") << "shortKnotValue.dat";
	log << "Open short knot value file: " << fileDirectory << (fileDirectory.size()?"\\":"") << "shortKnotValue.dat" << "\n";	
	
	// Open Database-File (FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_RANDOM_ACCESS)
	hFileShortKnotValues = CreateFile(ssDatabaseFile.str().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// opened file succesfully?
	if (hFileShortKnotValues == INVALID_HANDLE_VALUE) return log.log(logger::logLevel::error, L"Failed to open short knot value file.");

	// get file name
	wstringstream	ssFile;
	ssFile << fileDirectory << (fileDirectory.size()?"\\":"") << "plyInfo.dat";
	log << "Open ply info file: " << ssFile.str().c_str() << "\n";
	
	// Open Database-File (FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_RANDOM_ACCESS)
	hFilePlyInfo = CreateFile(ssFile.str().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// opened file succesfully?
	if (hFilePlyInfo == INVALID_HANDLE_VALUE) {
		CloseHandle(hFileShortKnotValues);
		return log.log(logger::logLevel::error, L"Failed to open ply info file.");
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: removeFile()
// Desc: Removes the database files from the disk.
//		 Close the database, if necessary, before deletion.
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::removeFile(wstring const &fileDirectory)
{
	wstringstream ssDatabaseFile;
	
	if (isOpen()) closeDatabase();

	log << "Remove database files in folder: " << fileDirectory << "\n";
	
	ssDatabaseFile.str(L""); ssDatabaseFile << fileDirectory << (fileDirectory.size()?"\\":"") << "shortKnotValue.dat";
	if (GetFileAttributes(ssDatabaseFile.str().c_str()) != INVALID_FILE_ATTRIBUTES) {
		_wremove(ssDatabaseFile.str().c_str());
	}

	// check if files are deleted
	if (GetFileAttributes(ssDatabaseFile.str().c_str()) != INVALID_FILE_ATTRIBUTES) {
		return log.log(logger::logLevel::error, L"Failed to delete database files.");
	}
	log << "Database files deleted successfully.\n";

	ssDatabaseFile.str(L""); ssDatabaseFile << fileDirectory << (fileDirectory.size()?"\\":"") << "plyInfo.dat";
	if (GetFileAttributes(ssDatabaseFile.str().c_str()) != INVALID_FILE_ATTRIBUTES) {
		_wremove(ssDatabaseFile.str().c_str());
	}

	// check if files are deleted
	if (GetFileAttributes(ssDatabaseFile.str().c_str()) != INVALID_FILE_ATTRIBUTES) {
		return log.log(logger::logLevel::error, L"Failed to delete database files.");
	}
	log << "Database files deleted successfully.\n";

	return true;
}

//-----------------------------------------------------------------------------
// Name: loadHeader()
// Desc: Loads the header and stats from the database files into memory.
//		 New files are created if they do not exist.
//		 The header is checked for consistency with the loaded files.
//       dbStats and layerStats will be completely overwritten.
//		 Returns false if the header is not loaded.
// 		 dbStats: 	 general information about the database
// 		 layerStats: layer specific information 
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::loadHeader(databaseStatsStruct& dbStats, vector<layerStatsStruct>& layerStats)
{
	if (!isOpen()) return log.log(logger::logLevel::error, L"Cannot load header, since database is not open.");
	if (!openSkvFile(dbStats, layerStats)) return false;
	if (!openPlyInfoFile(layerStats)) return false;
	log << "Database header loaded.\n";
	return true;
}

//-----------------------------------------------------------------------------
// Name: saveHeader()
// Desc: Saves the header and stats to the database files.
//		 They are NOT checked for consistency with the written skv and plyInfo.
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::saveHeader(const databaseStatsStruct& dbStats, const vector<layerStatsStruct>& layerStats)
{
	if (!isOpen()) return log.log(logger::logLevel::error, L"Cannot save header, since database is not open.");
	log << "Save database header.\n";

	skvfHeader.completed			= dbStats.completed;
	plyInfoHeader.plyInfoCompleted	= dbStats.completed;
	skvfHeader.numLayers			= dbStats.numLayers;

	// copy infos from generic layer stats to myLayerStats and plyInfos
	myLayerStats.resize(skvfHeader.numLayers);
	for (unsigned int i=0; i<skvfHeader.numLayers; i++) {	
		myLayerStats[i].layerIsCompletedAndInFile	= layerStats[i].completedAndInFile;
		myLayerStats[i].numWonStates				= layerStats[i].numWonStates	;
		myLayerStats[i].numLostStates				= layerStats[i].numLostStates	;
		myLayerStats[i].numDrawnStates				= layerStats[i].numDrawnStates	;
		myLayerStats[i].numInvalidStates			= layerStats[i].numInvalidStates;
		plyInfos[i].plyInfoIsCompletedAndInFile		= layerStats[i].completedAndInFile;
	}

	if (!saveSkvHeader(skvfHeader, myLayerStats)) return false;
	if (!savePlyHeader(plyInfoHeader, plyInfos)) return false;
	return true;
}

//-----------------------------------------------------------------------------
// Name: isOpen()
// Desc: Returns true if the handles to both database files are not NULL.
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::isOpen()
{
	return hFileShortKnotValues != INVALID_HANDLE_VALUE && hFilePlyInfo != INVALID_HANDLE_VALUE;
}

//-----------------------------------------------------------------------------
// Name: readSkv()
// Desc: Read all short knot values from the database for a given layer.
//		 The passed vector must have the correct size myLayerStats[layerNum].sizeInBytes, which can be 0.
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::readSkv(unsigned int layerNum, vector<twoBit>& skv)
{
	if (layerNum >= myLayerStats.size()) return log.log(logger::logLevel::error, L"readSkv() failed. Layer number out of range.");
	if (skv.size() != myLayerStats[layerNum].sizeInBytes) return log.log(logger::logLevel::error, L"readSkv() failed. Size of passed vector does not match size of layer.");
	if (hFileShortKnotValues == INVALID_HANDLE_VALUE) return log.log(logger::logLevel::error, L"readSkv() failed. Database file not open.");
	if (skvfHeader.headerAndStatsSize == 0) return log.log(logger::logLevel::error, L"readSkv() failed. Header not loaded.");
	if (myLayerStats[layerNum].sizeInBytes == 0) return log.log(logger::logLevel::error, L"readSkv() failed. Layer has no knots.");
	if (!myLayerStats[layerNum].layerIsCompletedAndInFile) return log.log(logger::logLevel::error, L"readSkv() failed. Layer is not in file.");
	return loadBytesFromFile(hFileShortKnotValues, skvfHeader.headerAndStatsSize + myLayerStats[layerNum].layerOffset, myLayerStats[layerNum].sizeInBytes, &skv[0]);
}

//-----------------------------------------------------------------------------
// Name: readSkv()
// Desc: Read a short knot value from the database for a given layer and state number. 
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::readSkv(unsigned int layerNum, twoBit& databaseByte, unsigned int stateNumber)
{
	if (layerNum >= myLayerStats.size()) return log.log(logger::logLevel::error, L"readSkv() failed. Layer number out of range.");
	if (hFileShortKnotValues == INVALID_HANDLE_VALUE) return log.log(logger::logLevel::error, L"readSkv() failed. Database file not open.");
	if (skvfHeader.headerAndStatsSize == 0) return log.log(logger::logLevel::error, L"readSkv() failed. Header not loaded.");
	if (stateNumber >= myLayerStats[layerNum].knotsInLayer) return log.log(logger::logLevel::error, L"readSkv() failed. State number out of range.");
	if (myLayerStats[layerNum].sizeInBytes == 0) return log.log(logger::logLevel::error, L"readSkv() failed. Layer has no knots.");
	if (myLayerStats[layerNum].sizeInBytes <= stateNumber / 4) return log.log(logger::logLevel::error, L"readSkv() failed. State number out of range.");
	if (!myLayerStats[layerNum].layerIsCompletedAndInFile) return log.log(logger::logLevel::error, L"readSkv() failed. Layer is not in file.");
	return loadBytesFromFile(hFileShortKnotValues, skvfHeader.headerAndStatsSize + myLayerStats[layerNum].layerOffset + stateNumber / 4, sizeof(twoBit), &databaseByte);
}

//-----------------------------------------------------------------------------
// Name: writeSkv()
// Desc: Write all short knot values to the database for a given layer. 
//		 The passed vector must have the correct size myLayerStats[layerNum].sizeInBytes, which can be 0.
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::writeSkv(unsigned int layerNum, const vector<twoBit>& skv)
{
	if (layerNum >= myLayerStats.size()) return log.log(logger::logLevel::error, L"writeSkv() failed. Layer number out of range.");
	if (skv.size() != myLayerStats[layerNum].sizeInBytes) return log.log(logger::logLevel::error, L"writeSkv() failed. Size of passed vector does not match size of layer.");
	if (hFileShortKnotValues == INVALID_HANDLE_VALUE) return log.log(logger::logLevel::error, L"writeSkv() failed. Database file not open.");
	if (skvfHeader.headerAndStatsSize == 0) return log.log(logger::logLevel::error, L"writeSkv() failed. Header not loaded.");
	if (myLayerStats[layerNum].sizeInBytes == 0) return log.log(logger::logLevel::error, L"writeSkv() failed. Layer has no knots.");
	myLayerStats[layerNum].layerIsCompletedAndInFile = true;
	return saveBytesToFile(hFileShortKnotValues, skvfHeader.headerAndStatsSize + myLayerStats[layerNum].layerOffset, myLayerStats[layerNum].sizeInBytes, &skv[0]);
}

//-----------------------------------------------------------------------------
// Name: readPlyInfo()
// Desc: Read all ply information from the database for a given layer. 
// 		 The passed vector must have the correct size plyInfos[layerNum].sizeInBytes, which can be 0.
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::readPlyInfo(unsigned int layerNum, vector<plyInfoVarType>& plyInfo)
{
	if (layerNum >= plyInfos.size()) return log.log(logger::logLevel::error, L"readPlyInfo() failed. Layer number out of range.");
	if (plyInfo.size() != plyInfos[layerNum].knotsInLayer) return log.log(logger::logLevel::error, L"readPlyInfo() failed. Size of passed vector does not match size of layer.");
	if (hFilePlyInfo == INVALID_HANDLE_VALUE) return log.log(logger::logLevel::error, L"readPlyInfo() failed. Database file not open.");
	if (plyInfoHeader.headerAndPlyInfosSize == 0) return log.log(logger::logLevel::error, L"readPlyInfo() failed. Header not loaded.");
	if (plyInfos[layerNum].sizeInBytes == 0) return log.log(logger::logLevel::error, L"readPlyInfo() failed. Layer has no knots.");
	if (!plyInfos[layerNum].plyInfoIsCompletedAndInFile) return log.log(logger::logLevel::error, L"readPlyInfo() failed. Layer is not in file.");
	return loadBytesFromFile(hFilePlyInfo, plyInfoHeader.headerAndPlyInfosSize + plyInfos[layerNum].layerOffset, plyInfos[layerNum].sizeInBytes, &plyInfo[0]);
}

//-----------------------------------------------------------------------------
// Name: readPlyInfo()
// Desc: Read the ply information from the database for a given layer and state number. 
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::readPlyInfo(unsigned int layerNum, plyInfoVarType& singlePlyInfo, unsigned int stateNumber)
{
	if (layerNum >= plyInfos.size()) return log.log(logger::logLevel::error, L"readPlyInfo() failed. Layer number out of range.");
	if (hFilePlyInfo == INVALID_HANDLE_VALUE) return log.log(logger::logLevel::error, L"readPlyInfo() failed. Database file not open.");
	if (plyInfoHeader.headerAndPlyInfosSize == 0) return log.log(logger::logLevel::error, L"readPlyInfo() failed. Header not loaded.");
	if (stateNumber >= plyInfos[layerNum].knotsInLayer) return log.log(logger::logLevel::error, L"readPlyInfo() failed. State number out of range.");
	if (plyInfos[layerNum].sizeInBytes == 0) return log.log(logger::logLevel::error, L"readPlyInfo() failed. Layer has no knots.");
	if (!plyInfos[layerNum].plyInfoIsCompletedAndInFile) return log.log(logger::logLevel::error, L"readPlyInfo() failed. Layer is not in file.");
	return loadBytesFromFile(hFilePlyInfo, plyInfoHeader.headerAndPlyInfosSize + plyInfos[layerNum].layerOffset + sizeof(plyInfoVarType) * stateNumber, sizeof(plyInfoVarType), &singlePlyInfo);
}

//-----------------------------------------------------------------------------
// Name: writePlyInfo()
// Desc: Write all ply information to the database for a given layer.
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::writePlyInfo(unsigned int layerNum, const vector<plyInfoVarType>& plyInfo)
{
	if (layerNum >= plyInfos.size()) return log.log(logger::logLevel::error, L"writePlyInfo() failed. Layer number out of range.");
	if (plyInfo.size() != plyInfos[layerNum].knotsInLayer) return log.log(logger::logLevel::error, L"writePlyInfo() failed. Size of passed vector does not match size of layer.");
	if (hFilePlyInfo == INVALID_HANDLE_VALUE) return log.log(logger::logLevel::error, L"writePlyInfo() failed. Database file not open.");
	if (plyInfoHeader.headerAndPlyInfosSize == 0) return log.log(logger::logLevel::error, L"writePlyInfo() failed. Header not loaded.");
	if (plyInfos[layerNum].sizeInBytes == 0) return log.log(logger::logLevel::error, L"writePlyInfo() failed. Layer has no knots.");
	plyInfos[layerNum].plyInfoIsCompletedAndInFile = true;
	return saveBytesToFile(hFilePlyInfo, plyInfoHeader.headerAndPlyInfosSize + plyInfos[layerNum].layerOffset,	plyInfos[layerNum].sizeInBytes,	&plyInfo[0]);
}

//-----------------------------------------------------------------------------
// Name: closeDatabase()
// Desc: Close the database files.
// 		 The data in the memory is not affected. 
//-----------------------------------------------------------------------------
void miniMax::database::uncompFile::closeDatabase()
{
	log << "Close database.\n";

	// close database
	if (hFileShortKnotValues != INVALID_HANDLE_VALUE) {
		CloseHandle(hFileShortKnotValues);
		hFileShortKnotValues = INVALID_HANDLE_VALUE;
	}

	// close ply information file
	if (hFilePlyInfo != INVALID_HANDLE_VALUE) {
		CloseHandle(hFilePlyInfo);
		hFilePlyInfo = INVALID_HANDLE_VALUE;
	}
	myLayerStats.clear();
	plyInfos	.clear();
	plyInfoHeader.numLayers 				= 0;
	plyInfoHeader.headerAndPlyInfosSize 	= 0;
	plyInfoHeader.plyInfoCompleted 			= false;
	skvfHeader.completed 					= false;
	skvfHeader.numLayers 					= 0;
	skvfHeader.headerAndStatsSize 			= 0;	
}

//-----------------------------------------------------------------------------
// Name: saveBytesToFile()
// Desc: Write a number of bytes to a file at a given offset. 
//		 If operation fails, the function waits for 1 second and tries again.
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::saveBytesToFile(HANDLE hFile, long long offset, unsigned int numBytes, const void *pBytes)
{
	if (hFile == NULL) return log.log(logger::logLevel::error, L"ERROR: saveBytesToFile() failed. Database file not open.");

	DWORD			dwBytesWritten;
	LARGE_INTEGER	liDistanceToMove;
	unsigned int	restingBytes	= numBytes;
	const void *	myPointer		= pBytes;

	liDistanceToMove.QuadPart = offset;

	while (!SetFilePointerEx(hFile, liDistanceToMove, NULL, FILE_BEGIN)) { 
		log.log(logger::logLevel::error, wstring{L"ERROR: SetFilePointerEx failed!"}); 
		Sleep(1000);
	}
	
	while (restingBytes > 0) {
		if (WriteFile(hFile, myPointer, restingBytes, &dwBytesWritten, NULL) == TRUE) {
			restingBytes -= dwBytesWritten;
			myPointer	  = (void*) (((unsigned char*) myPointer) + dwBytesWritten);
			if (restingBytes > 0) {
				log << L"Still " << restingBytes << L" to write!\n";
			}
		} else {
			log.log(logger::logLevel::error, wstring{L"ERROR: WriteFile Failed!"});
			Sleep(1000);
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: loadBytesFromFile()
// Desc: Read a number of bytes from a file at a given offset. 
//		 If operation fails, the function waits for 1 second and tries again.
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::loadBytesFromFile(HANDLE hFile, long long offset, unsigned int numBytes, void *pBytes)
{
	if (hFile == NULL) return log.log(logger::logLevel::error, L"ERROR: loadBytesFromFile() failed. Database file not open.");

	DWORD			dwBytesRead;
	LARGE_INTEGER	liDistanceToMove;
	unsigned int	restingBytes	= numBytes;
	void *			myPointer		= pBytes;

	liDistanceToMove.QuadPart = offset;

	while (!SetFilePointerEx(hFile, liDistanceToMove, NULL, FILE_BEGIN)) { 
		log.log(logger::logLevel::error, wstring{L"ERROR: SetFilePointerEx failed!"}); 
		Sleep(1000);
	}
	
	while (restingBytes > 0) {
		if (ReadFile(hFile, pBytes, restingBytes, &dwBytesRead, NULL) == TRUE) {
			restingBytes -= dwBytesRead;
			myPointer	  = (void*) (((unsigned char*) myPointer) + dwBytesRead);
			if (restingBytes > 0) {
				log << L"Still " << restingBytes << L" bytes to read!\n";
			}
		} else {
			log.log(logger::logLevel::error, wstring{L"ERROR: ReadFile Failed!"});
			Sleep(1000);
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: saveSkvHeader()
// Desc: Store the header and stats in the short knot value file.
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::saveSkvHeader(const skvFileHeaderStruct& dbH, const vector<skvFileLayerStruct> &lStats)
{
	if (hFileShortKnotValues == INVALID_HANDLE_VALUE) return log.log(logger::logLevel::error, L"saveSkvHeader() failed. Database file not open.");
	if (lStats.size() != dbH.numLayers) return log.log(logger::logLevel::error, L"saveSkvHeader() failed. Number of layers does not match.");
	if (!saveBytesToFile(hFileShortKnotValues, 0, sizeof(skvFileHeaderStruct), &dbH)) return false;
	for (unsigned int i=0; i<dbH.numLayers; i++) {
		if (!lStats[i].saveToFile(hFileShortKnotValues)) return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: savePlyHeader()
// Desc: Store the header and stats in the ply info file. 
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::savePlyHeader(const plyInfoFileHeaderStruct&piH, const vector<plyInfoFileLayerStruct>& pInfo)
{
	if (hFilePlyInfo == INVALID_HANDLE_VALUE) return log.log(logger::logLevel::error, L"savePlyHeader() failed. Database file not open.");
	if (pInfo.size() != piH.numLayers) return log.log(logger::logLevel::error, L"savePlyHeader() failed. Number of layers does not match.");
	if (!saveBytesToFile(hFilePlyInfo, 0, 								sizeof(plyInfoFileHeaderStruct), 		&piH)		) return false;
	if (!saveBytesToFile(hFilePlyInfo, sizeof(plyInfoFileHeaderStruct), sizeof(plyInfoFileLayerStruct) * piH.numLayers, 	&pInfo[0])	) return false;
	return true;
}

//-----------------------------------------------------------------------------
// Nanme: createAndWriteEmptySkvHeader()
// Desc: Create an empty short knot value file with the header only.
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::createAndWriteEmptySkvHeader()
{
	// create default header
	skvfHeader.completed			= false;
	skvfHeader.numLayers			= game->getNumberOfLayers();
	skvfHeader.headerCode			= SKV_FILE_HEADER_CODE;
	skvfHeader.headerAndStatsSize	= sizeof(skvFileHeaderStruct);
	myLayerStats.resize(skvfHeader.numLayers);
	myLayerStats[0].layerOffset		= 0;

	// locals
	for (unsigned int i=0; i<skvfHeader.numLayers; i++) {	
		game->getSuccLayers(i, myLayerStats[i].succLayers);
		myLayerStats[i].partnerLayers 				= game->getPartnerLayers(i);
		myLayerStats[i].knotsInLayer				= game->getNumberOfKnotsInLayer(i);
		myLayerStats[i].sizeInBytes					= (myLayerStats[i].knotsInLayer + 3) / 4;
		myLayerStats[i].layerIsCompletedAndInFile	= false;
		myLayerStats[i].numWonStates				= 0;
		myLayerStats[i].numLostStates				= 0;
		myLayerStats[i].numDrawnStates				= 0;
		myLayerStats[i].numInvalidStates			= 0;
	}
	
	// calculate layer offsets, based on the size of the previous layer
	for (unsigned int i=1; i<skvfHeader.numLayers; i++) {
		myLayerStats[i].layerOffset				= myLayerStats[i-1].layerOffset + myLayerStats[i-1].sizeInBytes;
	}
	
	// add the size of all layers to the header size
	for (auto& layer : myLayerStats) {
		skvfHeader.headerAndStatsSize += layer.getSizeInBytes();
	}

	// write header
	return saveSkvHeader(skvfHeader, myLayerStats);
}

//-----------------------------------------------------------------------------
// Name: openSkvFile()
// Desc: Open the short knot value file and read the header and stats into the memory.
//	 	 If the file is invalid or does not exist, a new file with an empty header is created.
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::openSkvFile(databaseStatsStruct& dbStats, vector<layerStatsStruct>& layerStats)
{
	// locals
	size_t			fileSize;
	unsigned int	i;

	// is file already open?
	if (hFileShortKnotValues == INVALID_HANDLE_VALUE) return log.log(logger::logLevel::error, L"Database file not open.");

	// Get the file size
	fileSize = GetFileSize(hFileShortKnotValues, NULL);

	// invalid file size ?
	if (fileSize < sizeof(skvFileHeaderStruct)) {
		
		// create empty header
		log << "Create empty short knot value file.\n";
		if (!createAndWriteEmptySkvHeader()) {
			return false;
		}

	// read layer stats
	} else {

		// database complete ?
		if (!loadBytesFromFile(hFileShortKnotValues, 0, sizeof(skvFileHeaderStruct), &skvfHeader)) {
			return false;
		}

		// invalid file ?
		if (skvfHeader.headerCode != SKV_FILE_HEADER_CODE) return log.log(logger::logLevel::error, L"Invalid short knot value file header.");

		// read layer stats
		myLayerStats.resize(skvfHeader.numLayers);
		for (auto& layer : myLayerStats) {
			if (!layer.loadFromFile(hFileShortKnotValues)) {
				return false;
			}
		}
	}

	// Translation to layerStats and dbStats
	dbStats.completed	= skvfHeader.completed;
	dbStats.numLayers	= skvfHeader.numLayers;
	layerStats.resize(skvfHeader.numLayers);
	for (i=0; i<skvfHeader.numLayers; i++) {	
		layerStats[i].completedAndInFile	= myLayerStats[i].layerIsCompletedAndInFile	;
		layerStats[i].knotsInLayer			= myLayerStats[i].knotsInLayer				;
		layerStats[i].numWonStates			= myLayerStats[i].numWonStates				;
		layerStats[i].numLostStates			= myLayerStats[i].numLostStates				;
		layerStats[i].numDrawnStates		= myLayerStats[i].numDrawnStates			;
		layerStats[i].numInvalidStates		= myLayerStats[i].numInvalidStates			;
		layerStats[i].partnerLayer			= myLayerStats[i].partnerLayers[0]			;
		layerStats[i].skv	 .clear();
		layerStats[i].plyInfo.clear();
		layerStats[i].succLayers 			= myLayerStats[i].succLayers;
		layerStats[i].partnerLayers 		= myLayerStats[i].partnerLayers;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Nanme: createAndWriteEmptyPlyHeader()
// Desc: Create an empty ply info file with the header only.
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::createAndWriteEmptyPlyHeader()
{
	log << "Create empty ply info file.\n";

	// create default header
	plyInfoHeader.plyInfoCompleted		= false;
	plyInfoHeader.numLayers				= game->getNumberOfLayers();
	plyInfoHeader.headerCode			= PLYINFO_HEADER_CODE;
	plyInfoHeader.headerAndPlyInfosSize = sizeof(plyInfoFileLayerStruct) * plyInfoHeader.numLayers + sizeof(plyInfoHeader);
	plyInfos.resize(plyInfoHeader.numLayers);
	plyInfos[0].layerOffset				= 0;

	for (unsigned int i=0; i<plyInfoHeader.numLayers; i++) {	
		plyInfos[i].knotsInLayer				= game->getNumberOfKnotsInLayer(i);
		plyInfos[i].plyInfoIsCompletedAndInFile	= false;
		plyInfos[i].sizeInBytes					= plyInfos[i].knotsInLayer * sizeof(plyInfoVarType);
	}
	
	for (unsigned int i=1; i<plyInfoHeader.numLayers; i++) {
		plyInfos[i].layerOffset					= plyInfos[i-1].layerOffset + plyInfos[i-1].sizeInBytes;
	}

	// write header
	return savePlyHeader(plyInfoHeader, plyInfos);
}

//-----------------------------------------------------------------------------
// Name: openPlyInfoFile()
// Desc: Open the ply info file and read the header and stats into the memory. 
//		 If the file is invalid or does not exist, a new file with an empty header is created.
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::openPlyInfoFile(vector<layerStatsStruct>& layerStats)
{
	// locals
	DWORD			fileSize;
	unsigned int	i;

	// is file already open?
	if (hFilePlyInfo == INVALID_HANDLE_VALUE) return log.log(logger::logLevel::error, L"Database file not open.");

	// Get the file size
	fileSize = GetFileSize(hFilePlyInfo, NULL);

	// invalid file ?
	if (fileSize < sizeof(plyInfoHeader) + sizeof(plyInfoFileLayerStruct) * plyInfoHeader.numLayers) {
		
		// create empty header
		if (!createAndWriteEmptyPlyHeader()) {
			return false;
		}

	// read layer stats
	} else {
		// database complete ?
		if (!loadBytesFromFile(hFilePlyInfo, 0, sizeof(plyInfoHeader), &plyInfoHeader)) {
			return false;
		}

		// invalid file ?
		if (plyInfoHeader.headerCode != PLYINFO_HEADER_CODE) return log.log(logger::logLevel::error, L"Invalid ply info file header.");

		// read layer stats
		plyInfos.resize(plyInfoHeader.numLayers);
		if (!loadBytesFromFile(hFilePlyInfo, sizeof(plyInfoFileHeaderStruct), sizeof(plyInfoFileLayerStruct) * plyInfoHeader.numLayers, plyInfos.data())) {
			return false;
		}
	}
	return true;
}

#pragma region skvFileLayerStruct

//-----------------------------------------------------------------------------
// Name: getSizeInBytes()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int miniMax::database::uncompFile::skvFileLayerStruct::getSizeInBytes() const
{
	return  sizeof(layerIsCompletedAndInFile) + 
			sizeof(layerOffset				) + 
			sizeof(knotsInLayer				) + 
			sizeof(numWonStates				) + 
			sizeof(numLostStates			) + 
			sizeof(numDrawnStates			) + 
			sizeof(numInvalidStates			) + 
			sizeof(sizeInBytes				) + 
			sizeof(unsigned int) * succLayers	.size() + sizeof(unsigned int) +
			sizeof(unsigned int) * partnerLayers.size() + sizeof(unsigned int);
}

//-----------------------------------------------------------------------------
// Name: loadFromFile()
// Desc: Load the layer stats from the file at the current file pointer position.
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::skvFileLayerStruct::loadFromFile(HANDLE hFile)
{
	DWORD dwBytesRead;	
	if (!ReadFile(hFile, &layerIsCompletedAndInFile, 	sizeof(layerIsCompletedAndInFile), 	&dwBytesRead, NULL)) return false;
	if (!ReadFile(hFile, &layerOffset, 					sizeof(layerOffset), 				&dwBytesRead, NULL)) return false;
	if (!ReadFile(hFile, &knotsInLayer, 				sizeof(knotsInLayer), 				&dwBytesRead, NULL)) return false;
	if (!ReadFile(hFile, &numWonStates, 				sizeof(numWonStates), 				&dwBytesRead, NULL)) return false;
	if (!ReadFile(hFile, &numLostStates, 				sizeof(numLostStates), 				&dwBytesRead, NULL)) return false;
	if (!ReadFile(hFile, &numDrawnStates, 				sizeof(numDrawnStates), 			&dwBytesRead, NULL)) return false;
	if (!ReadFile(hFile, &numInvalidStates, 			sizeof(numInvalidStates), 			&dwBytesRead, NULL)) return false;
	if (!ReadFile(hFile, &sizeInBytes, 					sizeof(sizeInBytes), 				&dwBytesRead, NULL)) return false;
	if (!loadVectorFromFile(hFile, succLayers)) 	return false;
	if (!loadVectorFromFile(hFile, partnerLayers)) 	return false;
	return true;
}

//-----------------------------------------------------------------------------
// Name: saveToFile()
// Desc: Save the layer stats to the file at the current file pointer position.
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::skvFileLayerStruct::saveToFile(HANDLE hFile) const
{
	DWORD dwBytesWritten;
	if (!WriteFile(hFile, &layerIsCompletedAndInFile, 	sizeof(layerIsCompletedAndInFile), 	&dwBytesWritten, NULL)) return false;
	if (!WriteFile(hFile, &layerOffset, 				sizeof(layerOffset), 				&dwBytesWritten, NULL)) return false;
	if (!WriteFile(hFile, &knotsInLayer, 				sizeof(knotsInLayer), 				&dwBytesWritten, NULL)) return false;
	if (!WriteFile(hFile, &numWonStates, 				sizeof(numWonStates), 				&dwBytesWritten, NULL)) return false;
	if (!WriteFile(hFile, &numLostStates, 				sizeof(numLostStates), 				&dwBytesWritten, NULL)) return false;
	if (!WriteFile(hFile, &numDrawnStates, 				sizeof(numDrawnStates), 			&dwBytesWritten, NULL)) return false;
	if (!WriteFile(hFile, &numInvalidStates, 			sizeof(numInvalidStates), 			&dwBytesWritten, NULL)) return false;
	if (!WriteFile(hFile, &sizeInBytes, 				sizeof(sizeInBytes), 				&dwBytesWritten, NULL)) return false;
	if (!saveVectorToFile(hFile, succLayers)) 		return false;
	if (!saveVectorToFile(hFile, partnerLayers)) 	return false;
	return true;
}

//-----------------------------------------------------------------------------
// Name: loadVectorFromFile()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::skvFileLayerStruct::loadVectorFromFile(HANDLE hFile, vector<unsigned int>& buffer)
{
	DWORD dwBytesRead;
	unsigned int bytesToRead;
	if (!ReadFile(hFile, &bytesToRead, sizeof(unsigned int), &dwBytesRead, NULL)) return false;
	buffer.resize(bytesToRead / sizeof(unsigned int));
	return ReadFile(hFile, buffer.data(), bytesToRead, &dwBytesRead, NULL) && dwBytesRead == bytesToRead;
}

//-----------------------------------------------------------------------------
// Name: saveVectorToFile()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::database::uncompFile::skvFileLayerStruct::saveVectorToFile(HANDLE hFile, const vector<unsigned int>& buffer) const
{
	DWORD dwBytesWritten;
	unsigned int bytesToWrite = buffer.size() * sizeof(unsigned int);
	if (!WriteFile(hFile, &bytesToWrite, sizeof(unsigned int), &dwBytesWritten, NULL)) return false;
	if (!WriteFile(hFile, buffer.data(), bytesToWrite, &dwBytesWritten, NULL)) return false;
	return dwBytesWritten == bytesToWrite;
}

#pragma endregion

#pragma endregion

#pragma region comppressed database - compFile
//-----------------------------------------------------------------------------
// Name: compFile()
// Desc: 
//-----------------------------------------------------------------------------
miniMax::database::compFile::compFile(gameInterface* game, logger& log) : genericFile{game, log}
{
	file.setBlockSize(blockSizeInBytes);
}

//-----------------------------------------------------------------------------
// Name: updateFileName()
// Desc: 
//-----------------------------------------------------------------------------
void miniMax::database::compFile::updateFileName()
{
	wstringstream ssDatabaseFilePath;
	ssDatabaseFilePath << fileDirectory << (fileDirectory.size()?"\\":"") << L"database.dat";
	fileName = ssDatabaseFilePath.str();
}

//-----------------------------------------------------------------------------
// Name: openDatabase()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::database::compFile::openDatabase(wstring const &fileDirectory)
{
	log << "Open database from folder " << std::filesystem::absolute(fileDirectory) << "\n";
	this->fileDirectory = fileDirectory;
	updateFileName();
	if (!file.open(fileName, false)) return log.log(logger::logLevel::error, L"Failed to open database file.");
	fileOpened = true;
	return true;
}

//-----------------------------------------------------------------------------
// Name: closeDatabase()
// Desc: 
//-----------------------------------------------------------------------------
void miniMax::database::compFile::closeDatabase()
{
	log << "Close database.\n";
	file.close();
	fileOpened = false;

	// reset cache
	dbStatsCache.completed	= false;
	dbStatsCache.numLayers	= 0;
	layerStatsCache.clear();
}

//-----------------------------------------------------------------------------
// Name: removeFile()
// Desc: Deletes the database file. 
//       If 'fileDirectory' is omitted, the function will attempt to remove the file at the current 'fileDirectory' path.
//-----------------------------------------------------------------------------
bool miniMax::database::compFile::removeFile(wstring const &fileDirectory)
{
	log << "Remove database file.\n";
	if (isOpen()) closeDatabase();
	this->fileDirectory = fileDirectory;
	updateFileName();
	_wremove(fileName.c_str());

	// return false, if file still exists
	if (std::filesystem::exists(fileName)) return log.log(logger::logLevel::error, L"Failed to remove database file.");

	log << "Database file " << fileName << " removed successfully.\n";
	this->fileDirectory = L"";
	fileOpened = false;
	return true;
}

//-----------------------------------------------------------------------------
// Name: updateCache()
// Desc:
//-----------------------------------------------------------------------------
void miniMax::database::compFile::updateCache(const databaseStatsStruct& dbStats, const vector<layerStatsStruct>& layerStats)
{
	dbStatsCache 	= dbStats;
	layerStatsCache.resize(layerStats.size());
	for (size_t i=0; i<layerStats.size(); i++) {
		layerStatsCache[i].completedAndInFile 	= layerStats[i].completedAndInFile;	
		layerStatsCache[i].partnerLayer		 	= layerStats[i].partnerLayer;		
		layerStatsCache[i].knotsInLayer		 	= layerStats[i].knotsInLayer;		
		layerStatsCache[i].numWonStates		 	= layerStats[i].numWonStates;		
		layerStatsCache[i].numLostStates		= layerStats[i].numLostStates;		
		layerStatsCache[i].numDrawnStates		= layerStats[i].numDrawnStates;		
		layerStatsCache[i].numInvalidStates	 	= layerStats[i].numInvalidStates;	
		layerStatsCache[i].succLayers			= layerStats[i].succLayers;		
		layerStatsCache[i].partnerLayers		= layerStats[i].partnerLayers;	
	}
}

//-----------------------------------------------------------------------------
// Name: readSection()
// Desc:
//-----------------------------------------------------------------------------
bool miniMax::database::compFile::readSection(const wstring& key, vector<unsigned int>& buffer)
{
	if (!fileOpened) return log.log(logger::logLevel::error, L"Cannot read section, since database is not open.");
	unsigned int bytesToRead = file.getSizeOfUncompressedSection(key);
	if (bytesToRead == 0) return log.log(logger::logLevel::error, L"Section does not exist.");
	buffer.resize(bytesToRead / sizeof(unsigned int));
	return file.read(key, 0, bytesToRead, buffer.data());
}

//-----------------------------------------------------------------------------
// Name: loadHeader()
// Desc:
//-----------------------------------------------------------------------------
bool miniMax::database::compFile::loadHeader(databaseStatsStruct& dbStats, vector<layerStatsStruct>& layerStats)
{
	if (!fileOpened) return log.log(logger::logLevel::error, L"Cannot load header, since database is not open.");
	log << "Load database header.\n";

	if (file.getSizeOfCompressedSection(L"dbStats") > 0) {
		file.read(L"dbStats", 0, sizeof(dbStats), &dbStats);
		layerStats.resize(dbStats.numLayers);
		for (unsigned int i=0; i<dbStats.numLayers; i++) {
			wstring baseKeyName = wstring(L"layerStats") + to_wstring(i);	// remember the base key name for later use
			if (!file.read(wstring(L"layerStats") + to_wstring(i), 0, layerStatsStruct::numBytesLayerStatsHeader, &layerStats[i])) return false;
			if (!layerStats[i].knotsInLayer) {
				continue;
			}
			if (!readSection(baseKeyName + wstring(L".succLayers"), layerStats[i].succLayers)) return false;
			// only new databases have multiple partnerLayers
			if (file.doesKeyExist(baseKeyName + wstring(L".partnerLayers"))) {
				if (!readSection(baseKeyName + wstring(L".partnerLayers"), layerStats[i].partnerLayers)) return false;
			// old databases have only one partnerLayer
			} else {
				layerStats[i].partnerLayers.assign({layerStats[i].partnerLayer});
			}
			layerStats[i].skv.clear();
			layerStats[i].plyInfo.clear();
		}
	} else {
		dbStats.completed	= false;
		dbStats.numLayers	= game->getNumberOfLayers();
		layerStats.resize(dbStats.numLayers);
		for (unsigned int i=0; i<dbStats.numLayers; i++) {
			game->getSuccLayers(i, layerStats[i].succLayers);
			layerStats[i].completedAndInFile	= false;
			layerStats[i].knotsInLayer			= game->getNumberOfKnotsInLayer(i);
			layerStats[i].partnerLayers			= game->getPartnerLayers(i);
			layerStats[i].partnerLayer			= layerStats[i].partnerLayers[0];		// only one partner layer in legacy databases
			layerStats[i].numWonStates			= 0;
			layerStats[i].numLostStates			= 0;
			layerStats[i].numDrawnStates		= 0;
			layerStats[i].numInvalidStates		= 0;
		}
	}
	updateCache(dbStats, layerStats);
	return true;
}

//-----------------------------------------------------------------------------
// Name: saveHeader()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::database::compFile::saveHeader(const databaseStatsStruct& dbStats, const vector<layerStatsStruct>& layerStats)
{
	if (!isOpen() || !file.isOpen()) return log.log(logger::logLevel::error, L"Cannot save header, since database is not open.");
	log << "Save database header.\n";
	if (!file.write(L"dbStats", 0, sizeof(dbStats), &dbStats)) {
		return log.log(logger::logLevel::error, L"Failed to save database stats.");
	}
	for (unsigned int i=0; i<dbStats.numLayers; i++) {
		if (!file.write(wstring(L"layerStats") + to_wstring(i), 0, layerStatsStruct::numBytesLayerStatsHeader, &layerStats[i])) {
			return log.log(logger::logLevel::error, L"Failed to save layer stats.");
			}
		if (layerStats[i].succLayers.size()) {
			if (!file.write(wstring(L"layerStats") + to_wstring(i) + wstring(L".succLayers"), 0, sizeof(unsigned int) * layerStats[i].succLayers.size(), layerStats[i].succLayers.data())) {
				return log.log(logger::logLevel::error, L"Failed to save layer stats.");
			}
		}
		if (layerStats[i].partnerLayers.size()) {
			if (!file.write(wstring(L"layerStats") + to_wstring(i) + wstring(L".partnerLayers"), 0, sizeof(unsigned int) * layerStats[i].partnerLayers.size(), layerStats[i].partnerLayers.data())) {
				return log.log(logger::logLevel::error, L"Failed to save layer stats.");
			}
		}
	}
	updateCache(dbStats, layerStats);
	return true;
}

//-----------------------------------------------------------------------------
// Name: isOpen()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::database::compFile::isOpen()
{
	return fileOpened;
}

//-----------------------------------------------------------------------------
// Name: readSkv()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::database::compFile::readSkv(unsigned int layerNum, vector<twoBit>& skv)
{
	if (!isOpen()) return log.log(logger::logLevel::error, L"Cannot read skv, since database is not open.");
	if (layerNum >= layerStatsCache.size()) return log.log(logger::logLevel::error, L"Layer number out of range.");
	if (!layerStatsCache[layerNum].completedAndInFile) return log.log(logger::logLevel::error, L"Layer is not in file.");
	if (skv.size() != layerStatsCache[layerNum].getLayerSizeInBytesForSkv()) return log.log(logger::logLevel::error, L"Size of passed vector does not match size of layer.");
	if (!file.read(wstring(L"skv") + to_wstring(layerNum), 0, skv.size() * sizeof(twoBit), &skv[0])) return log.log(logger::logLevel::error, L"Failed to read skv.");
	return true;
}

//-----------------------------------------------------------------------------
// Name: readSkv()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::database::compFile::readSkv(unsigned int layerNum, twoBit& databaseByte, unsigned int stateNumber)
{
	if (!isOpen()) return log.log(logger::logLevel::error, L"Cannot read skv, since database is not open.");
	if (layerNum >= layerStatsCache.size()) return log.log(logger::logLevel::error, L"Layer number out of range.");
	if (!layerStatsCache[layerNum].completedAndInFile) return log.log(logger::logLevel::error, L"Layer is not in file.");
	if (stateNumber >= layerStatsCache[layerNum].knotsInLayer) return log.log(logger::logLevel::error, L"State number out of range.");
	if (!file.read(wstring(L"skv") + to_wstring(layerNum), (stateNumber/4) * sizeof(twoBit), 1, &databaseByte)) return log.log(logger::logLevel::error, L"Failed to read skv.");
	return true;
}

//-----------------------------------------------------------------------------
// Name: writeSkv()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::database::compFile::writeSkv(unsigned int layerNum, const vector<twoBit>& skv)
{
	if (!isOpen()) return log.log(logger::logLevel::error, L"Cannot read skv, since database is not open.");
	if (layerNum >= layerStatsCache.size()) return log.log(logger::logLevel::error, L"Layer number out of range.");
	if (skv.size() != layerStatsCache[layerNum].getLayerSizeInBytesForSkv()) return log.log(logger::logLevel::error, L"Size of passed vector does not match size of layer.");
	if (!file.write(wstring(L"skv") + to_wstring(layerNum), 0, skv.size() * sizeof(twoBit), &skv[0])) return log.log(logger::logLevel::error, L"Failed to write skv.");
	layerStatsCache[layerNum].completedAndInFile = true;
	return true;
}

//-----------------------------------------------------------------------------
// Name: readPlyInfo()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::database::compFile::readPlyInfo(unsigned int layerNum, vector<plyInfoVarType>& plyInfo)
{
	if (!isOpen()) return log.log(logger::logLevel::error, L"Cannot read skv, since database is not open.");
	if (layerNum >= layerStatsCache.size()) return log.log(logger::logLevel::error, L"Layer number out of range.");
	if (!layerStatsCache[layerNum].completedAndInFile) return log.log(logger::logLevel::error, L"Layer is not in file.");
	if (plyInfo.size() != layerStatsCache[layerNum].knotsInLayer) return log.log(logger::logLevel::error, L"Size of passed vector does not match size of layer.");
	if (!file.read(wstring(L"plyInfo") + to_wstring(layerNum), 0, plyInfo.size() * sizeof(plyInfoVarType), &plyInfo[0])) return log.log(logger::logLevel::error, L"Failed to read ply info.");
	return true;
}

//-----------------------------------------------------------------------------
// Name: readPlyInfo()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::database::compFile::readPlyInfo(unsigned int layerNum, plyInfoVarType& singlePlyInfo, unsigned int stateNumber)
{
	if (!isOpen()) return log.log(logger::logLevel::error, L"Cannot read skv, since database is not open.");
	if (layerNum >= layerStatsCache.size()) return log.log(logger::logLevel::error, L"Layer number out of range.");
	if (!layerStatsCache[layerNum].completedAndInFile) return log.log(logger::logLevel::error, L"Layer is not in file.");
	if (stateNumber >= layerStatsCache[layerNum].knotsInLayer) return log.log(logger::logLevel::error, L"State number out of range.");
	if (!file.read(wstring(L"plyInfo") + to_wstring(layerNum), stateNumber * sizeof(plyInfoVarType), sizeof(plyInfoVarType), &singlePlyInfo)) return log.log(logger::logLevel::error, L"Failed to read ply info.");
	return true;
}

//-----------------------------------------------------------------------------
// Name: writePlyInfo()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::database::compFile::writePlyInfo(unsigned int layerNum, const vector<plyInfoVarType>& plyInfo)
{
	if (!isOpen()) return log.log(logger::logLevel::error, L"Cannot read skv, since database is not open.");
	if (layerNum >= layerStatsCache.size()) return log.log(logger::logLevel::error, L"Layer number out of range.");
	if (!layerStatsCache[layerNum].completedAndInFile) return log.log(logger::logLevel::error, L"Layer is not in file.");
	if (plyInfo.size() != layerStatsCache[layerNum].knotsInLayer) return log.log(logger::logLevel::error, L"Size of passed vector does not match size of layer.");
	if (!file.write(wstring(L"plyInfo") + to_wstring(layerNum), 0, plyInfo.size() * sizeof(plyInfoVarType), &plyInfo[0])) return log.log(logger::logLevel::error, L"Failed to write ply info.");
	layerStatsCache[layerNum].completedAndInFile = true;
	return true;
}
#pragma endregion
