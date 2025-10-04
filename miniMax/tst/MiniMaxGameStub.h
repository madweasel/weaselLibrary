/*********************************************************************\
	MiniMaxGameStub.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/
#pragma once

#include "gtest/gtest.h"
#include <string>
#include <vector>
#include <iostream>

#include "miniMax/src/typeDef.h"
#include "weaselEssentials/src/threadManager.h"
#include "weaselEssentials/src/logger.h"
#include "miniMax/src/database/database.h"

using miniMax::plyInfoVarType;
using miniMax::twoBit;
using miniMax::stateAdressStruct;
using miniMax::gameInterface;
using miniMax::database::database;
using miniMax::retroAnalysis::predVars;

class testGraph {
public:
	// constants with shorter name
	const plyInfoVarType	PLY_INV 							= miniMax::PLYINFO_VALUE_INVALID;
	const plyInfoVarType	PLY_DRAW 							= miniMax::PLYINFO_VALUE_DRAWN;
	const twoBit 			SKV_INV 							= miniMax::SKV_VALUE_INVALID;
	const twoBit 			SKV_LOST 							= miniMax::SKV_VALUE_GAME_LOST;
	const twoBit 			SKV_DRAW 							= miniMax::SKV_VALUE_GAME_DRAWN;
	const twoBit 			SKV_WON 							= miniMax::SKV_VALUE_GAME_WON;

	// test state, with short name for easy assignment via constructor
	struct ts {
		unsigned int 		layerNumber							= 0;
		unsigned int 		stateNumber							= 0;
		bool 				playerToMoveHasChanged				= false;
	};

	// knot of the game graph
	struct testKnot {
		unsigned int 				layerNumber;
		unsigned int 				stateNumber;
		twoBit 						value;														// initial short knot value
		twoBit 						expValue;													// expected calculated short knot value
		plyInfoVarType				expPlyInfo;													// expected calculated ply info	
		vector<ts> 					predecessors;
		vector<ts> 					successors;
		vector<stateAdressStruct> 	symmetricStates;
	};

	// test specific functions
							testGraph						();
	bool					anyDuplicateKnot				();
	testKnot& 				getKnot							(unsigned int layer, unsigned int state);
	bool 					doesKnotExist					(unsigned int layer, unsigned int state);
	unsigned int			getNumExpectedStates			(unsigned int layer, twoBit expValue);

	// test data
	vector<testKnot> knots = {
		//       layer, state, 	initial value, 	calc. value,	plyInfo, 	predecessors , 							successors, 						symmetric states
		testKnot{0, 	0, 		SKV_INV, 		SKV_INV, 		PLY_INV,	{}, 									{},									{{0,0}}				},																		
		testKnot{0, 	1, 		SKV_WON, 		SKV_WON, 		0, 			{ts{0, 2, true}}, 						{},									{{1,0}}				},
		testKnot{0, 	2, 		SKV_DRAW, 		SKV_LOST, 		1, 			{}, 									{ts{0, 1, true}},					{{2,0}}				},
		// testKnot{0, 	0, 		SKV_INV, 		SKV_INV, 		0, 			{}, 									{},									{}					},
		// testKnot{0, 	1, 		SKV_INV, 		SKV_INV, 		0, 			{}, 									{},									{}					},
		// testKnot{0, 	2, 		SKV_INV, 		SKV_INV, 		0, 			{}, 									{},									{}					},
		// testKnot{0, 	0, 		SKV_INV, 		SKV_INV, 		0, 			{}, 									{},									{}					},
		// testKnot{0, 	1, 		SKV_INV, 		SKV_INV, 		0, 			{}, 									{},									{}					},
		// testKnot{0, 	2, 		SKV_INV, 		SKV_INV, 		0, 			{}, 									{},									{}					},
		// testKnot{0, 	0, 		SKV_INV, 		SKV_INV, 		0, 			{}, 									{},									{}					},
		// testKnot{0, 	1, 		SKV_INV, 		SKV_INV, 		0, 			{}, 									{},									{}					},
		// testKnot{0, 	2, 		SKV_INV, 		SKV_INV, 		0, 			{}, 									{},									{}					},
		// testKnot{0, 	0, 		SKV_INV, 		SKV_INV, 		0, 			{}, 									{},									{}					},
		// testKnot{0, 	1, 		SKV_INV, 		SKV_INV, 		0, 			{}, 									{},									{}					},
		// testKnot{0, 	2, 		SKV_INV, 		SKV_INV, 		0, 			{}, 									{},									{}					},
		// testKnot{0, 	0, 		SKV_INV, 		SKV_INV, 		0, 			{}, 									{},									{}					},
		// testKnot{0, 	1, 		SKV_INV, 		SKV_INV, 		0, 			{}, 									{},									{}					},
		// testKnot{0, 	2, 		SKV_INV, 		SKV_INV, 		0, 			{}, 									{},									{}					}
	};
};

class testGame : public gameInterface {
public:
	// partner layer                    					   0, 1, 2, 3, 4, 5, 6, 7, 8, 9
	vector<uint_1d> 		partnerLayers 					= {{0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}};
	testGraph 				graph;

	// each thread has its own state
	unsigned int 			numThreads 						= 3;							// number of threads
	vector<testGraph::ts> 	states;															// current state of each thread
	vector<testGraph::ts>  	movementBackups;												// backup of the last movement for undo()

	// test specific functions
							testGame						();
	void					setNumberOfThreads				(unsigned int numThreads);
	void 					checkWithDatabase				(database& db, const vector<unsigned int>& layersCalculated);

	// init
	virtual void			prepareCalculation				()																													override;

	// getter
    virtual bool         	shallRetroAnalysisBeUsed    	(unsigned int layerNum)																								override;
	virtual void			getPossibilities				(unsigned int threadNo, vector<unsigned int>& possibilityIds)														override;
	virtual unsigned int	getMaxNumPossibilities			()																													override;
	virtual unsigned int	getNumberOfLayers				()																													override;
	virtual unsigned int 	getMaxNumPlies					()																													override;
	virtual unsigned int	getNumberOfKnotsInLayer			(unsigned int layerNum)																								override;
    virtual void            getSuccLayers               	(unsigned int layerNum, vector<unsigned int>& succLayers)															override;
	virtual uint_1d			getPartnerLayers				(unsigned int layerNum)																								override;
	virtual void			getValueOfSituation				(unsigned int threadNo, float& floatValue, twoBit& shortValue)														override;
	virtual void			getLayerAndStateNumber			(unsigned int threadNo, unsigned int& layerNum, unsigned int& stateNumber, unsigned int& symOp)						override;
	virtual unsigned int	getLayerNumber					(unsigned int threadNo)																								override;
	virtual void			getSymStateNumWithDuplicates	(unsigned int threadNo, vector<miniMax::stateAdressStruct>& symStates)												override;
    virtual void            getPredecessors             	(unsigned int threadNo, vector<predVars>& predVars)																	override;
	virtual bool			isStateIntegrityOk				(unsigned int threadNo)																								override;
	virtual void			applySymOp						(unsigned int threadNo, unsigned char symmetryOperationNumber, bool doInverseOperation, bool playerToMoveChanged)	override;
	virtual bool			lostIfUnableToMove				(unsigned int threadNo)																								override;

	// setter
	virtual bool			setSituation					(unsigned int threadNo, unsigned int layerNum, unsigned int stateNumber)											override;
	virtual void			move							(unsigned int threadNo, unsigned int idPossibility, bool& playerToMoveChanged, void* &pBackup)						override;
	virtual void			undo							(unsigned int threadNo, unsigned int idPossibility, bool& playerToMoveChanged, void*  pBackup)						override;
	
	// output
	virtual void			printField						(unsigned int threadNo, twoBit value, unsigned int indentSpaces = 0)												override;
	virtual void			printMoveInformation			(unsigned int threadNo, unsigned int idPossibility)																	override;
	virtual wstring			getOutputInformation			(unsigned int layerNum)																								override;
};

class MiniMaxTestGameFixture : public ::testing::Test {
protected:
	logger 					log{logger::logLevel::debug, logger::logType::console, L""};
	testGame				game;
	database				db{game, log};
	threadManagerClass		tm;
	const wstring 			tmpFileDirectory;
	const unsigned int 		numThreads 			= 2;

							MiniMaxTestGameFixture(const string folderName);

	virtual void 			SetUp() override;
	virtual void 			TearDown() override;
};
