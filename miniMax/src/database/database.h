/*********************************************************************\
	database.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#pragma once

#include "databaseFile.h"
#include "databaseStats.h"
#include "weaselEssentials/src/logger.h"

namespace miniMax
{

namespace database
{
	class database
	{
	public:
		using 						succLayerList 					= std::vector<unsigned int>;
		using 						partnerLayerList 				= std::vector<unsigned int>;

									database						(gameInterface& gi, logger& log);
									~database						();

		// open and close functions
		bool						openDatabase					(wstring const &fileDirectory, bool useCompFileIfBothExist = true);
		bool						closeDatabase					();
		bool						saveHeader						();
		bool						removeDatabaseFiles				();
		void						unload							();

		// statistics
		void						showLayerStats					(unsigned int layerNumber);
		bool 						updateLayerStats				(unsigned int layerNumber);

		// setter
		bool						setAsComplete					();
		bool 						setLoadingOfFullLayerOnRead		();
		
		// getter
		bool						isOpen							()							{ return file ? file->isOpen() : false; };
		bool						isComplete						();
		bool						isLayerCompleteAndInFile		(unsigned int layerNumber);
		unsigned int				getNumberOfKnots				(unsigned int layerNumber);
		unsigned int				getNumLayers					()							{ return dbStats.numLayers; }
		const partnerLayerList&		getPartnerLayers				(unsigned int layerNumber)	{ if (layerNumber >= layerStats.size()) return partnerLayerDummy; 	return layerStats[layerNumber].partnerLayers; };
		const succLayerList&		getSuccLayers					(unsigned int layerNumber)  { if (layerNumber >= layerStats.size()) return succLayerDummy; 		return layerStats[layerNumber].succLayers; };
		long long					getMemoryUsed					()							{ return arrayInfos.getMemoryUsed(); };
		stateNumberVarType			getNumWonStates					(unsigned int layerNum);
		stateNumberVarType			getNumLostStates				(unsigned int layerNum);
		stateNumberVarType			getNumDrawnStates				(unsigned int layerNum);
		stateNumberVarType			getNumInvalidStates				(unsigned int layerNum);
		long long					getLayerSizeInBytes				(unsigned int layerNum);
		wstring  					getFileDirectory				() 							{ if (file) return file->getFileDirectory(); return L""; };
	
		// read and write operations
		bool						readKnotValueFromDatabase		(unsigned int  layerNumber, unsigned int  stateNumber, twoBit &knotValue);
		bool						readPlyInfoFromDatabase			(unsigned int  layerNumber, unsigned int  stateNumber, plyInfoVarType &value);
		bool						writeKnotValueInDatabase		(unsigned int  layerNumber, unsigned int  stateNumber, twoBit  knotValue);
		bool						writePlyInfoInDatabase			(unsigned int  layerNumber, unsigned int  stateNumber, plyInfoVarType value);
		bool 						loadLayerFromFile				(unsigned int  layerNumber);
		bool						saveLayerToFile					(unsigned int  layerNumber);
	
		// functions for gui output
		arrayInfoContainer			arrayInfos;										// information about the arrays in memory

	private:

		// functions
		bool 						resizePlyInfo					(layerStatsStruct& myLss, unsigned int layerNumber);
		bool 						resizeSkv						(layerStatsStruct& myLss, unsigned int layerNumber);

		// general 
		logger&						log;											// logger
		gameInterface *				game							= nullptr;		// master class
		genericFile *				file							= nullptr;		// file handler
		std::mutex 					csDatabaseMutex;								// mutex for I/O operations
		databaseStatsStruct			dbStats;										// general information about the database
		vector<layerStatsStruct>	layerStats;										// layer specific information 
		succLayerList				succLayerDummy;									// dummy for empty return value
		partnerLayerList			partnerLayerDummy;								// dummy for empty return value	
		bool 						loadFullLayerOnRead				= false;		// load full layer on read ?

		// performance measurement
		speedometer::printFuncType	printIops						= [&](wstring& name, float operationsPerSec) {  };
		speedometer					speedoReadSkv {L"Read  knot value ", MEASURE_TIME_FREQUENCY, printIops};			// measure database io operations per second of read operations 
		speedometer					speedoWriteSkv{L"Write knot value ", MEASURE_TIME_FREQUENCY, printIops};			// measure database io operations per second of write operations
		speedometer					speedoReadPly {L"Read  ply info   ", MEASURE_TIME_FREQUENCY, printIops};			// measure database io operations per second of read operations
		speedometer					speedoWritePly{L"Write ply info   ", MEASURE_TIME_FREQUENCY, printIops};			// measure database io operations per second of write operations
	};

} // namespace database

} // namespace miniMax
