/*********************************************************************
	miniMax.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "miniMax.h"
#include <numeric>

//-----------------------------------------------------------------------------
// Name: miniMax()
// Desc: miniMax class constructor
//-----------------------------------------------------------------------------
miniMax::miniMax::miniMax(gameInterface* game, unsigned int maxAlphaBetaSearchDepth)
	:
	 log{logger::logLevel::info, logger::logType::both, L"miniMax.log"},
	 game{game}, 
	 db{*game, log}, 
	 checker{log, threadManager, db, *game}, 
	 monitor{this, log}, 
	 abSolver{log, threadManager, db, *game}, 
	 rtSolver{log, threadManager, db, *game}
{
	// init default values
	curCalculatedLayer			= 0;
	abSolver.setSearchDepth(maxAlphaBetaSearchDepth);

	InitializeCriticalSection(&csOsPrint);
	srand((unsigned int)time(NULL));

	// Thousand separator
	#ifdef _MSC_VER 
		locale locale("de_CH.UTF-8"); 	
		cout.imbue(locale); 
	#endif
}

//-----------------------------------------------------------------------------
// Name: ~miniMax()
// Desc: miniMax class destructor
//-----------------------------------------------------------------------------
miniMax::miniMax::~miniMax()
{
	closeDatabase();
	DeleteCriticalSection(&csOsPrint);
}

//-----------------------------------------------------------------------------
// Name: setSearchDepth()
// Desc: 
//-----------------------------------------------------------------------------
void miniMax::miniMax::setSearchDepth(unsigned int maxAlphaBetaSearchDepth)
{
	abSolver.setSearchDepth(maxAlphaBetaSearchDepth);
}

//-----------------------------------------------------------------------------
// Name: getBestChoice()
// Desc: Returns the best choice if the database has been opened and calculates the best choice for that if database is not open.
//-----------------------------------------------------------------------------
bool miniMax::miniMax::getBestChoice(unsigned int& choice, stateInfo& infoAboutChoices)
{
	// set global vars
	return abSolver.getBestChoice(choice, infoAboutChoices);
}

//-----------------------------------------------------------------------------
// Name: calculateDatabase()
// Desc: Calculates the database, which must be already open.
//-----------------------------------------------------------------------------
bool miniMax::miniMax::calculateDatabase()
{
   	// locals
	bool abortCalculation = false;
	lastCalculatedLayer.clear();

	log.log(logger::logLevel::info, L"*************************");
	log.log(logger::logLevel::info, L"* Calculate Database    *");
	log.log(logger::logLevel::info, L"*************************");

	// call preparation function of parent class
	game->prepareCalculation();

	// when database not completed then do it
    if (db.isOpen() && !db.isComplete()) {

        // reserve memory
		lastCalculatedLayer.clear();
		threadManager.reset();

		// calc layer after layer, beginning with the last one
		for (curCalculatedLayer=0; curCalculatedLayer<db.getNumLayers(); curCalculatedLayer++) {

			// layer already calculated?
			if (db.isLayerCompleteAndInFile(curCalculatedLayer)) continue;

			// get layers to calculate
			layersToCalculate = db.getPartnerLayers(curCalculatedLayer);
			layersToCalculate.push_back(curCalculatedLayer);			
			
			// remove duplicates
			sort(layersToCalculate.begin(), layersToCalculate.end());
			layersToCalculate.erase(unique(layersToCalculate.begin(), layersToCalculate.end()), layersToCalculate.end());

			// don't calc if neither the layer nor the partner layer has any knots
			unsigned int totalNumberOfKnots = accumulate(layersToCalculate.begin(), layersToCalculate.end(), 0, [&](unsigned int sum, unsigned int curLayer) { return sum + db.getNumberOfKnots(curLayer); });
			if (totalNumberOfKnots == 0) continue;
			
			// calc
			abortCalculation  = (!calcLayer(curCalculatedLayer));

			// relase memory
			unloadDatabase();

			// don't save layer when aborted
			if (abortCalculation) break;

			// save header
			db.saveHeader();
		}

		// don't save layer and header when only preparing layers or when aborting
		if (!abortCalculation) {

			// calc layer statistics
			monitor.calcLayerStatistics("statistics.csv");

			// save header
			db.setAsComplete();
			db.saveHeader();
		}

        // free mem
		curAction = activity::none;
	} else {
		log.log(logger::logLevel::info, L"The database is already fully calculated.");
	}

	if (abortCalculation) {
		log.log(logger::logLevel::error, L"Layer calculation cancelled or failed!");
		return false;
	}

	// output
	log << L"*************************\n";
	log << L"* Calculation finished  *\n";
	log << L"*************************\n";
	return true;
}

//-----------------------------------------------------------------------------
// Name: calculateStatistics()
// Desc: Calculates statistics for a completed database.
//-----------------------------------------------------------------------------
bool miniMax::miniMax::calculateStatistics()
{
    log.log(logger::logLevel::info, L"*************************");
    log.log(logger::logLevel::info, L"* Calculate Statistics  *");
    log.log(logger::logLevel::info, L"*************************");

    // Check if database is open and complete
    if (db.isOpen() && db.isComplete()) {
        // Calculate layer statistics
        monitor.calcLayerStatistics("statistics.csv");

        log << L"*************************\n";
        log << L"* Statistics finished   *\n";
        log << L"*************************\n";
        return true;
    } else {
        log.log(logger::logLevel::error, L"Database must be open and complete to calculate statistics.");
        return false;
    }
}

//-----------------------------------------------------------------------------
// Name: calcLayer()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::miniMax::calcLayer(unsigned int layerNumber)
{
	// moves can be done reverse, leading to too depth searching trees
	if (game->shallRetroAnalysisBeUsed(layerNumber)) {
		if (!rtSolver.calcKnotValuesByRetroAnalysis(layersToCalculate)) return false;
	// use minimax-algorithm
	} else {
		if (!abSolver.calcKnotValuesByAlphaBeta(layersToCalculate)) return false;
	}

	// save layers
	for (auto layer : layersToCalculate) {
		db.saveLayerToFile(layer);
	}

	// test layers
	for (auto layer : layersToCalculate) {
		if (!checker.testLayer(layer)) {
			return log.log(logger::logLevel::error, L"ERROR: Layer calculation cancelled or failed!");
		}
	}

	// update output information
	EnterCriticalSection(&csOsPrint);
	lastCalculatedLayer.assign(layersToCalculate.begin(), layersToCalculate.end());
	LeaveCriticalSection(&csOsPrint);

	// everything was ok
	return true;
}

//-----------------------------------------------------------------------------
// Name: pauseDatabaseCalculation()
// Desc: 
//-----------------------------------------------------------------------------
void miniMax::miniMax::pauseDatabaseCalculation()
{
	threadManager.pauseExecution();
}

//-----------------------------------------------------------------------------
// Name: cancelDatabaseCalculation()
// Desc: 
//-----------------------------------------------------------------------------
void miniMax::miniMax::cancelDatabaseCalculation()
{
	// when returning from executeParallelLoop() all function shall quit immediatelly up to calculateDatabase()
	threadManager.cancelExecution();
}

//-----------------------------------------------------------------------------
// Name: wasDatabaseCalculationCancelled()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::miniMax::wasDatabaseCalculationCancelled()
{
	return threadManager.wasExecutionCancelled();
}

//-----------------------------------------------------------------------------
// Name: closeDatabase()
// Desc: 
//-----------------------------------------------------------------------------
void miniMax::miniMax::closeDatabase()
{
	db.closeDatabase();
}

//-----------------------------------------------------------------------------
// Name: unloadDatabase()
// Desc: 
//-----------------------------------------------------------------------------
void miniMax::miniMax::unloadDatabase()
{
	db.unload();
}

//-----------------------------------------------------------------------------
// Name: isCurrentStateInDatabase()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::miniMax::isCurrentStateInDatabase(unsigned int threadNo)
{
	unsigned int layerNum, stateNumber, symOp;

	if (!db.isOpen()) {
		return false;
	} else {
		game->getLayerAndStateNumber(threadNo, layerNum, stateNumber, symOp);
		return db.isLayerCompleteAndInFile(layerNum);
	}
}

//-----------------------------------------------------------------------------
// Name: openDatabase()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::miniMax::openDatabase(wstring const& directory, bool useCompFileIfBothExist)
{
	fileDirectory.assign(directory);
	if (fileDirectory.empty()) fileDirectory = L"./database/";

	if (db.isOpen()) {
		return log.log(logger::logLevel::trace, L"Database already open!");
	}
	return db.openDatabase(fileDirectory, useCompFileIfBothExist);
}

//-----------------------------------------------------------------------------
// Name: getNumThreads()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int miniMax::miniMax::getNumThreads()
{
	return threadManager.getNumThreads();
}

//-----------------------------------------------------------------------------
// Name: setNumThreads()
// Desc:
//-----------------------------------------------------------------------------
bool miniMax::miniMax::setNumThreads(unsigned int numThreads)
{
	return threadManager.setNumThreads(numThreads);
}

//-----------------------------------------------------------------------------
// Name: anyFreshlyCalculatedLayer()
// Desc: called by MAIN-thread in pMiniMax->csOsPrint critical-section
//-----------------------------------------------------------------------------
bool miniMax::miniMax::anyFreshlyCalculatedLayer()
{
	return (lastCalculatedLayer.size()>0);
}

//-----------------------------------------------------------------------------
// Name: getLastCalculatedLayer()
// Desc: called by MAIN-thread in pMiniMax->csOsPrint critical-section
//-----------------------------------------------------------------------------
unsigned int miniMax::miniMax::getLastCalculatedLayer()
{
	if (lastCalculatedLayer.size() == 0) return game->getNumberOfLayers();
	unsigned int tmp = lastCalculatedLayer.front();
	lastCalculatedLayer.pop_front();
	return tmp;
}

//-----------------------------------------------------------------------------
// Name: setOutputStream()
// Desc: Sets the output stream for the logger console output 
//-----------------------------------------------------------------------------
bool miniMax::miniMax::setOutputStream(wostream& theStream)
{
	return  log.setOutputStream(theStream);
}

//-----------------------------------------------------------------------------
// Name: setCurrentActivity()
// Desc: 
//-----------------------------------------------------------------------------
void miniMax::miniMax::setCurrentActivity(activity newAction)
{
	curAction = newAction;
}
