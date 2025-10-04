/*********************************************************************\
	successorCountArray.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/
#pragma once

#include "weaselEssentials/src/logger.h"
#include "weaselEssentials/src/threadManager.h"
#include "miniMax/src/typeDef.h"
#include "miniMax/src/database/database.h"
#include "miniMax/src/retroAnalysis/stateQueue.h"

namespace miniMax
{

// Using 2 Bytes for counting the number of predecessors allows a maximum of 65535 predecessors.
typedef unsigned short		countArrayVarType;							// 2 Byte for counting predecessors
const countArrayVarType		COUNT_ARRAY_MAX_VALUE		= 65535;		// maximum value for the count array

namespace retroAnalysis
{
	// class for the successor count array
 	// - decreasing and increasing the counter is thread safe
	// - the number of succeding states is specific to one layer and is stored in a count array called 'succCountArray'
	class successorCountArray
	{
		public:
														successorCountArray				(logger& log, database::database& db, unsigned int layerNumber);
														~successorCountArray			();
			countArrayVarType							increaseCounter					(stateNumberVarType stateNumber);	
			countArrayVarType							decreaseCounter					(stateNumberVarType stateNumber);
			unsigned int 								getLayerNumber					() const { return layerNumber; }

			logger& 									log;													// logger, used for output
			database::database& 						db;														// database, for storing the calculated values
			const unsigned int 							layerNumber;											// layer number
			vector<countArrayVarType>					succCountArray;											// count array for the number of drawn/unknown successors for each state
			mutex										succCountArrayMutex;									// mutex for the count array
	};

	// class for storing the succCountArrays in files
	// - the successor count arrays are stored in a file, one for each layer
	// - all layers to calculate must be loaded/stored at once
	// - the file is opened in the constructor and closed in the destructor
	// - the file is opened in binary mode and the data is written in binary format
	class successorCountFileStorage
	{
		public:
			struct layerInfoStruct
			{
				unsigned int							layerNumber;											// layer number
				unsigned int							numKnotsInLayer;										// number of knots in the current layer
				successorCountArray* 					succCountArray					= nullptr;				// successor count array for the current layer

				// user needs to provide only the layer number and the number of knots in the current layer
														layerInfoStruct					() {};
														layerInfoStruct					(unsigned int layerNumber, unsigned int numKnotsInCurLayer, successorCountArray& succCountArray) : layerNumber(layerNumber), numKnotsInLayer(numKnotsInCurLayer), succCountArray(&succCountArray) {}
			
				// reserved for internal use	
				HANDLE									hFileCountArray					= NULL;					// file handle for loading and saving the arrays in 'countArrays'
				wstring									sCountArrayFilePath;									// file path for the count array file
			};

														successorCountFileStorage		(logger& log, const wstring& fileDirectory, vector<layerInfoStruct>& layersToCalculate);
														~successorCountFileStorage		();
			bool										write							();
			bool										read							();

		private:
			logger& 									log;													// logger, used for output
			vector<layerInfoStruct>						layersToCalculate;										// layer number
	};

	// class for calculating the number of succeding states for each state
	// - thereby the database must already contain the knot values, for the invalid states. all others must be won, drawn or lost.
	// - the game interface must provide the functions 'setSituation()' and 'getPredecessors()'
	// - initialization is done in parellel
	class successorCountManager
	{
		friend class addNumSuccedorsVars;

		public:
														successorCountManager			(logger& log, threadManagerClass& tm, database::database& db, gameInterface& game, vector<stateQueue>& statesToProcess);
														~successorCountManager			();
			bool										init							(vector<unsigned int>& layersToCalculate);
			bool 										isReady							();
			countArrayVarType 							getAndDecreaseCounter			(unsigned int layerNumber, stateNumberVarType stateNumber);

		protected:
			bool 										initLayer						(successorCountArray& sca);
			bool										calcNumSuccedors				(unsigned int layerNumber);
			bool										addNumSuccedors					(unsigned int layerNumber);

			logger& 									log;													// logger, used for output
			threadManagerClass& 						tm;														// thread manager, for parallel processing
			database::database& 						db;														// database, for storing the calculated values
			gameInterface& 								game;													// game interface, for getting the game specific information
			long long 									totalNumStatesProcessed			= 0;					// number of states processed by all threads
			long long									roughTotalNumStatesProcessed	= 0;					// number of states processed by all threads (rough estimate)
			vector<bool>								layerProcessed;											// flag indicating if the layer has already been initialized 
			vector<successorCountArray*>				succCountArrays;										// One successor count array for each layer in 'layersToCalculate'. (For the nine men's morris game two layers have to considered at once.)
			vector<stateQueue>& 						statesToProcess;										// queue of states to be processed, one for each thread
			bool 										loadedScaFromFile				= false;				// true if the count arrays are loaded from file, but the statesToProcess still needs to be filled

			// static thread functions
			static DWORD								addNumSuccedorsThreadProc		(void* pParameter, int64_t index);
	};

	// thread specific variables for the function 'addNumSuccedorsThreadProc()'
	class addNumSuccedorsVars : public threadManagerClass::threadVarsArrayItem
	{
	public:
		friend class successorCountManager;
	
		successorCountManager& 							scm;													// pointer to all successor count arrays
		unsigned int									layerNumber						= 0;					// current layer number being calculated
		progressCounter									statesProcessed;										// number of states processed by the thread
		vector<predVars>								predVars{MAX_NUM_PREDECESSORS};							// array containing the predecessor states
		
														addNumSuccedorsVars				(addNumSuccedorsVars const& master);
														addNumSuccedorsVars				(successorCountManager& scm, unsigned int layerNumber, long long& roughTotalNumStatesProcessed);
		bool 											storePredecessorState			(const stateAdressStruct& predState);
		void											reduce							(); 
	
	private:
		const size_t 									predStatesChunkSize 			= 1000000;				// chunk size for the bufferPredStates
		static std::mutex								succCountArrayMutex;									// mutex for the successor count arrays
		vector<stateAdressStruct>						bufferPredStates;										// buffer for storing the predecessor states
		vector<int>										mapLayerNumberToScaId;									// map for storing the layer number to the id of the successor count array
		char											padding[64];											// Padding to avoid cache coherence issues

		bool 											flush							();

	};

} // namespace retroAnalysis

} // namespace miniMax
