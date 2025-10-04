/*********************************************************************
	miniMax_statistics.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "../miniMax.h"
// #include "statistics.h"
#include <windows.h>

//-----------------------------------------------------------------------------
// Name: testSetSituationAndGetPoss()
// Desc: 
//-----------------------------------------------------------------------------
miniMax::statistics::monitor::monitor(miniMax * pMiniMax, logger& log) : 
	log(log)
{
	mm		= pMiniMax;
	db		= &pMiniMax->db;
}

//-----------------------------------------------------------------------------
// Name: showMemoryStatus()
// Desc: 
//-----------------------------------------------------------------------------
void miniMax::statistics::monitor::showMemoryStatus()
{
	MEMORYSTATUSEX memStatus;
	memStatus.dwLength = sizeof (memStatus);
	GlobalMemoryStatusEx(&memStatus);

	std::cout << endl << "dwMemoryLoad           : " << memStatus.dwMemoryLoad;
	std::cout << endl << "ullAvailExtendedVirtual: " << memStatus.ullAvailExtendedVirtual;
	std::cout << endl << "ullAvailPageFile       : " << memStatus.ullAvailPageFile;
	std::cout << endl << "ullAvailPhys           : " << memStatus.ullAvailPhys;
	std::cout << endl << "ullAvailVirtual        : " << memStatus.ullAvailVirtual;
	std::cout << endl << "ullTotalPageFile       : " << memStatus.ullTotalPageFile;
	std::cout << endl << "ullTotalPhys           : " << memStatus.ullTotalPhys;
	std::cout << endl << "ullTotalVirtual        : " << memStatus.ullTotalVirtual;
}

//-----------------------------------------------------------------------------
// Name: calcLayerStatistics()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::statistics::monitor::calcLayerStatistics(const char *statisticsFileName)
{
	// locals
	stateAdressStruct	curState;
	twoBit		  		curStateValue;

	// database must be open
	if (!db->isOpen()) {
		return log.log(logger::logLevel::error, L"Database must be open to calculate statistics!");
	}

	// Open statistics file
	wofstream statFile(statisticsFileName, ios::out | ios::trunc);

	// opened file successfully?
	if (!statFile.is_open()) {
		return log.log(logger::logLevel::error, L"Failed to open statistics file");
	}

	// headline
	statFile << L"layer number,";
	statFile << L"white stones ";
	statFile << L"black stones,";
	statFile << L"won states,";
	statFile << L"lost states,";
	statFile << L"draw states,";
	statFile << L"invalid states,";
	statFile << L"total num states,";
	statFile << L"num succeding layers,";
	statFile << L"partner layer,";
	statFile << L"size in bytes,";
	statFile << L"succLayers[0] ";
	statFile << L"succLayers[1],";
	statFile << L"max num plies WON,";
	statFile << L"max ply state number WON,";
	statFile << L"max num plies LOST,";
	statFile << L"max ply state number LOST\n";

	mm->setCurrentActivity(activity::calcLayerStats);

	// calc and show statistics
	for (curState.layerNumber=0; curState.layerNumber<db->getNumLayers(); curState.layerNumber++) {

		// status output
		log << "Calculating statistics of layer: " << (int) curState.layerNumber << "\n";

		// only calc stats of completed layers
		if (!db->updateLayerStats(curState.layerNumber)) {
			return log.log(logger::logLevel::error, L"Failed to update layer stats of layer: " + to_wstring(curState.layerNumber) + L"\n");
		}

		// add line
		auto& succLayers 	= db->getSuccLayers(curState.layerNumber);
		auto& partnerLayers = db->getPartnerLayers(curState.layerNumber);
		statFile << curState.layerNumber
				 << L"," << mm->game->getOutputInformation(curState.layerNumber)
				 << L"," << db->getNumWonStates(curState.layerNumber)
				 << L"," << db->getNumLostStates(curState.layerNumber)
				 << L"," << db->getNumDrawnStates(curState.layerNumber)
				 << L"," << db->getNumInvalidStates(curState.layerNumber)
				 << L"," << db->getNumberOfKnots(curState.layerNumber)
				 << L"," << (unsigned int) succLayers.size()
				 << L",";
		for (auto& partnerLayer : partnerLayers) {
			statFile << partnerLayer << L" ";
		}
		statFile << L"," << (unsigned int) db->getLayerSizeInBytes(curState.layerNumber) << L",";
		for (auto& succLayer : succLayers) {
			statFile << succLayer << L" ";
		}
		// Get the maximum ply info value in the layer and show it as well as the state number
		{
			plyInfoVarType 		maxPlyInfoWon;
			stateNumberVarType	maxPlyStateWon;
			plyInfoVarType		maxPlyInfoLost;
			stateNumberVarType	maxPlyStateLost;
			if (!calcMaxPlyInfo(curState.layerNumber, maxPlyInfoWon, maxPlyStateWon, maxPlyInfoLost, maxPlyStateLost)) {
				return log.log(logger::logLevel::error, L"Failed to calculate max ply info for layer: " + to_wstring(curState.layerNumber) + L"\n");
			}
			statFile << L"," << maxPlyInfoWon << L"," << maxPlyStateWon << L"," << maxPlyInfoLost << L"," << maxPlyStateLost;
		}

		statFile << endl;

		// free memory
		db->unload();
	}

	// close file
	statFile.close();
	return true;
}

//-----------------------------------------------------------------------------
// Name: calcMaxPlyInfo()
// Desc:
//-----------------------------------------------------------------------------
bool miniMax::statistics::monitor::calcMaxPlyInfo(unsigned int layerNumber, plyInfoVarType &maxPlyInfoWon, stateNumberVarType &maxPlyStateWon, plyInfoVarType &maxPlyInfoLost, stateNumberVarType &maxPlyStateLost)
{
	// TODO: This function should be moved to miniMax::database::database::updateLayerStats() 
	//       when the database file is calculated the next time.

	// locals
	stateNumberVarType	numKnotsInLayer = db->getNumberOfKnots(layerNumber);
	stateNumberVarType	curStateNumber;
	twoBit		  		curStateValue;
	plyInfoVarType		curPlyInfo;

	// init
	maxPlyInfoWon 	= 0;
	maxPlyStateWon 	= numKnotsInLayer;
	maxPlyInfoLost 	= 0;
	maxPlyStateLost = numKnotsInLayer;

	// database layer must be fully loaded for better performance
	db->setLoadingOfFullLayerOnRead();

	// calc and show statistics
	for (curStateNumber=0; curStateNumber<numKnotsInLayer; curStateNumber++) {

		// get state value
		if (!db->readKnotValueFromDatabase(layerNumber, curStateNumber, curStateValue)) {
			return log.log(logger::logLevel::error, L"ERROR: Reading knot value from database failed!");
		}

		if (curStateValue == SKV_VALUE_GAME_WON || curStateValue == SKV_VALUE_GAME_LOST) {
			if (!db->readPlyInfoFromDatabase(layerNumber, curStateNumber, curPlyInfo)) {
				return log.log(logger::logLevel::error, L"ERROR: Reading ply info from database failed!");
			}
		} else {
			continue;
		}

		// check if it is the maximum
		if (curStateValue == SKV_VALUE_GAME_WON) {
			if (curPlyInfo > maxPlyInfoWon) {
				maxPlyInfoWon 	= curPlyInfo;
				maxPlyStateWon 	= curStateNumber;
			}
		} else if (curStateValue == SKV_VALUE_GAME_LOST) {
			if (curPlyInfo > maxPlyInfoLost) {
				maxPlyInfoLost 	= curPlyInfo;
				maxPlyStateLost = curStateNumber;
			}
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: getCurrentActionStr()
// Desc: called by MAIN-thread in pMiniMax->csOsPrint critical-section
//-----------------------------------------------------------------------------
const wchar_t* miniMax::statistics::monitor::getCurrentActionStr()
{
	switch (curAction)
	{
	case activity::initRetroAnalysis		:	return L"initiating retro-analysis";
	case activity::prepareCountArray		:	return L"preparing count arrays";
	case activity::performRetroAnalysis		:	return L"performing retro analysis";
	case activity::performAlphaBeta			:	return L"performing alpha-beta-algorithmn";
	case activity::testingLayer				:	return L"testing calculated layer";
	case activity::savingLayerToFile		:	return L"saving layer to file";
	case activity::calcLayerStats			:	return L"making layer statistics";
	case activity::none						:	return L"none";
	default:									return L"undefined";
	}
}

//-----------------------------------------------------------------------------
// Name: getCurrentCalculatedLayer()
// Desc: Fills the 'layers' vector with the currently calculated layer and its partner layers.
//       Called by MAIN-thread in pMiniMax->csOsPrint critical-section; not thread-safe and must be called under external synchronization.
//-----------------------------------------------------------------------------
void miniMax::statistics::monitor::getCurrentCalculatedLayer(vector<unsigned int> &layers)
{
	layers.clear();
	layers.push_back(mm->curCalculatedLayer);
	for (auto i : db->getPartnerLayers(mm->curCalculatedLayer)) {
		layers.push_back(i);
	}
}
