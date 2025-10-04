/*********************************************************************\
	databaseFile.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#pragma once

#include <vector>
#include <functional>

#include "compressor/src/compLib_winCompApi.h"
#include "weaselEssentials/src/logger.h"
#include "databaseTypes.h"

namespace miniMax
{

namespace database
{
    using namespace std;

	const int							SKV_FILE_HEADER_CODE		  = 0xF4F5;		// constant to identify the	header. These are the first two bytes of the file
	const int							PLYINFO_HEADER_CODE			  = 0xF3F2;		//     ''

	// This is a generic class for reading and writing the database files. It is used by the classes uncompFile and compFile.
	// The database is stored in memory and in an uncompressed file. After calculation the database is converted to a compressed file.
	// The uncompressed database is stored in two files: shortKnotValue.dat and plyInfo.dat
	// The compressed database is stored in one file: database.dat
	// The database files are located in the folder fileDirectory.
	// The database is organized in layers. Each layer contains a number of knots. Each knot contains a number of states.
	// Writing is supposed for one whole layer at a time. Reading is supposed for one whole layer at a time or for one state of a layer.
	// All reading/writing operations are directly accessing the database files.
	// The database files are opened and closed by the functions openDatabase() and closeDatabase(), prior to and after reading/writing.
	// Thread safety: The database files are NOT thread safe.
	class genericFile
	{
	public:
		virtual bool					openDatabase					(wstring const &fileDirectory) = 0;
		virtual void					closeDatabase					()																						{};
		virtual bool					removeFile						(wstring const &fileDirectory = L"")													{ return false; };
		virtual bool					isOpen							()																						{ return false; };
		virtual bool 					loadHeader						(databaseStatsStruct& dbStats, vector<layerStatsStruct>& layerStats)					{ return false; };
		virtual bool					saveHeader						(const databaseStatsStruct& dbStats, const vector<layerStatsStruct>& layerStats)		{ return false; };
		virtual bool					readSkv							(unsigned int layerNum, vector<twoBit>& skv)											{ return false; };
		virtual bool					readSkv							(unsigned int layerNum, twoBit& databaseByte, unsigned int stateNumber)					{ return false; };
		virtual bool					writeSkv						(unsigned int layerNum, const vector<twoBit>& skv)										{ return false; };
		virtual bool					readPlyInfo						(unsigned int layerNum, vector<plyInfoVarType>& plyInfo)								{ return false; };
		virtual bool					readPlyInfo						(unsigned int layerNum, plyInfoVarType& singlePlyInfo, unsigned int stateNumber)		{ return false; };
		virtual bool					writePlyInfo					(unsigned int layerNum, const vector<plyInfoVarType>& plyInfo)							{ return false; };
		virtual							~genericFile					()																						{ closeDatabase(); };
		wstring							getFileDirectory				()																						{ return fileDirectory; };

	protected:	
										genericFile						(gameInterface* game, logger& log) : game{game}, log{log} {};

		logger&							log;											// logger
		wstring							fileDirectory;									// path of the folder where the database files are located
		gameInterface	*				game							= nullptr;		// master class
	};

    // During calculation the database is stored in memory and in an uncompressed file. After calculation the database is converted to a compressed file.
	class uncompFile : public genericFile
	{
	public:
										uncompFile						(gameInterface* game, logger& log);
										~uncompFile						();

		bool							openDatabase					(wstring const &fileDirectory)														override;
		void							closeDatabase					()																					override;
		bool							removeFile						(wstring const &fileDirectory)														override;
		bool 							loadHeader						(databaseStatsStruct& dbStats, vector<layerStatsStruct>& layerStats)				override;
		bool							saveHeader						(const databaseStatsStruct& dbStats, const vector<layerStatsStruct>& layerStats)	override;
		bool							isOpen							()																					override;
		bool							readSkv							(unsigned int layerNum, vector<twoBit>& skv)										override;
		bool							readSkv							(unsigned int layerNum, twoBit& databaseByte, unsigned int stateNumber)				override;
		bool							writeSkv						(unsigned int layerNum, const vector<twoBit>& skv)									override;
		bool							readPlyInfo						(unsigned int layerNum, vector<plyInfoVarType>& plyInfo)							override;
		bool							readPlyInfo						(unsigned int layerNum, plyInfoVarType& singlePlyInfo, unsigned int stateNumber)	override;
		bool							writePlyInfo					(unsigned int layerNum, const vector<plyInfoVarType>& plyInfo)						override;

	private:
		struct skvFileHeaderStruct																	// header of the short knot value file
		{
			bool						completed						= false;					// true if all states have been calculated
			unsigned int				numLayers						= 0;						// number of layers
			unsigned int				headerCode						= SKV_FILE_HEADER_CODE;		// file identifier
			unsigned int				headerAndStatsSize				= 0;						// size in bytes of this struct plus the stats
		};

		struct plyInfoFileHeaderStruct																// header of the ply info file
		{
			bool						plyInfoCompleted				= false;					// true if ply innformation has been calculated for all game states
			unsigned int				numLayers						= 0;						// number of layers
			unsigned int				headerCode						= PLYINFO_HEADER_CODE;		// file identifier
			unsigned int				headerAndPlyInfosSize			= 0;						// size in bytes of this struct plus ...
		};

		struct skvFileLayerStruct																	// layer specific information for the short knot value file
		{	
			bool						layerIsCompletedAndInFile		= false;					// true, after user called writeSkv() storing all skv in the file
			long long					layerOffset						= 0;						// position of this struct in the short knot value file
			stateNumberVarType			knotsInLayer					= 0;						// number of knots of the corresponding layer
			stateNumberVarType			numWonStates					= 0;						// number of won states in this layer
			stateNumberVarType			numLostStates					= 0;						// number of lost states in this layer
			stateNumberVarType			numDrawnStates					= 0;						// number of drawn states in this layer
			stateNumberVarType			numInvalidStates				= 0;						// number of invalid states in this layer
			unsigned int				sizeInBytes						= 0;						// (knotsInLayer + 3) / 4
			vector<unsigned int>		succLayers;													// array containg the layer ids of the succeding layers
			vector<unsigned int>		partnerLayers;												// layers being calculated at the same time as this layer.

			unsigned int				getSizeInBytes					() const;
			bool 						saveToFile						(HANDLE hFile) const;
			bool 						loadFromFile					(HANDLE hFile);
			bool 						saveVectorToFile				(HANDLE hFile, const vector<unsigned int>& buffer) const;
			bool 						loadVectorFromFile				(HANDLE hFile, vector<unsigned int>& buffer);
		};

		struct plyInfoFileLayerStruct																// layer specific information for the ply info file
		{
			bool						plyInfoIsCompletedAndInFile		= false;					// true, after user called writePlyInfo() storing all ply info in the file
			long long					layerOffset						= 0;						// position of this struct in the ply info file
			unsigned int				sizeInBytes						= 0;						// size of this struct plus the array plyInfo[]
			stateNumberVarType			knotsInLayer					= 0;						// number of knots of the corresponding layer
		};

		HANDLE							hFileShortKnotValues			= INVALID_HANDLE_VALUE;		// handle of the file for the short knot value 
		HANDLE							hFilePlyInfo					= INVALID_HANDLE_VALUE;		// handle of the file for the ply info
		skvFileHeaderStruct				skvfHeader;													// short knot value file header
		plyInfoFileHeaderStruct			plyInfoHeader;												// header of the ply info file
		vector<skvFileLayerStruct>		myLayerStats;												// array of size [numLayers] containing general layer information and the skv
		vector<plyInfoFileLayerStruct>	plyInfos;													// array of size [numLayers] containing ply information 
											
		bool							createAndWriteEmptySkvHeader	();
		bool 							createAndWriteEmptyPlyHeader	();
		bool							openSkvFile						(databaseStatsStruct& dbStats, vector<layerStatsStruct>& layerStats);
		bool							openPlyInfoFile					(vector<layerStatsStruct>& layerStats);
		bool							saveSkvHeader					(const skvFileHeaderStruct& dbH, const vector<skvFileLayerStruct>& lStats);
		bool							savePlyHeader					(const plyInfoFileHeaderStruct& piH, const vector<plyInfoFileLayerStruct>& pInfo);
		bool							loadBytesFromFile				(HANDLE hFile, long long offset, unsigned int numBytes, void *pBytes);
		bool							saveBytesToFile					(HANDLE hFile, long long offset, unsigned int numBytes, const void *pBytes);
		void							unloadDatabase					();
	};

    // Compressed database file, to spare disk space during usage. The database is converted to this format after calculation.
	class compFile : public genericFile
	{
	public:
										compFile						(gameInterface* game, logger& log);
		bool							openDatabase					(wstring const &fileDirectory)														override;
		void							closeDatabase					()																					override;
		bool							removeFile						(wstring const &fileDirectory)														override;
		bool 							loadHeader						(databaseStatsStruct& dbStats, vector<layerStatsStruct>& layerStats)				override;
		bool							saveHeader						(const databaseStatsStruct& dbStats, const vector<layerStatsStruct>& layerStats)	override;
		bool							isOpen							()																					override;
		bool							readSkv							(unsigned int layerNum, vector<twoBit>& skv)										override;
		bool							readSkv							(unsigned int layerNum, twoBit& databaseByte, unsigned int stateNumber)				override;
		bool							writeSkv						(unsigned int layerNum, const vector<twoBit>& skv)									override;
		bool							readPlyInfo						(unsigned int layerNum, vector<plyInfoVarType>& plyInfo)							override;
		bool							readPlyInfo						(unsigned int layerNum, plyInfoVarType& singlePlyInfo, unsigned int stateNumber)	override;
		bool							writePlyInfo					(unsigned int layerNum, const vector<plyInfoVarType>& plyInfo)						override;

	private:	
		static constexpr unsigned int 	blockSizeInBytes				= 10000;		// size of one block in bytes. each section is stored in blocks of this fixed size. this enables random read access.

		compressor::winCompApi			comp;											// compression algorithmn
		compressor::file				file{comp};										// compressed database file
		wstring							fileName;										// name of the database file
		bool							fileOpened						= false;		// true if the database file is open
		databaseStatsStruct 			dbStatsCache;									// own copy of the database stats, used when reading/writing the database. updated when the header is loaded/written.
		vector<layerStatsStruct> 		layerStatsCache;								// own copy of the layer stats, used when reading/writing the database. updated when the header is loaded/written.

		bool							readSection						(const wstring& key, vector<unsigned int>& buffer);
		void							updateFileName					();
		void 							updateCache						(const databaseStatsStruct& dbStats, const vector<layerStatsStruct>& layerStats);
	};

} // namespace database

} // namespace miniMax
