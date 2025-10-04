/***************************************************************************************************************************
	miniMax.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
***************************************************************************************************************************/
#include "typeDef.h"

namespace miniMax
{
//-----------------------------------------------------------------------------
// Name: operator==
// Desc: Compares the state number and layer number
//-----------------------------------------------------------------------------
bool stateAdressStruct::operator==(const stateAdressStruct& rhs) const
{
	return (stateNumber == rhs.stateNumber && layerNumber == rhs.layerNumber);
}

//-----------------------------------------------------------------------------
// Name: operator<
// Desc: Compares the state number and layer number
//-----------------------------------------------------------------------------
bool stateAdressStruct::operator<(const stateAdressStruct& rhs) const
{
	if (layerNumber < rhs.layerNumber) return true;
	if (layerNumber > rhs.layerNumber) return false;
	return (stateNumber < rhs.stateNumber);
}

//-----------------------------------------------------------------------------
// Name: operator>
// Desc: Compares the state number and layer number
//-----------------------------------------------------------------------------
bool stateAdressStruct::operator>(const stateAdressStruct& rhs) const
{
	if (layerNumber > rhs.layerNumber) return true;
	if (layerNumber < rhs.layerNumber) return false;
	return (stateNumber > rhs.stateNumber);
}

//-----------------------------------------------------------------------------
// Name: updateBestAmountOfPlies()
// Desc:
//-----------------------------------------------------------------------------
void stateInfo::updateBestAmountOfPlies()
{
	// when the game is won, then try to find the minimum amount of plies to win
	if (shortValue == SKV_VALUE_GAME_WON) {
		bestAmountOfPlies = PLYINFO_VALUE_INVALID;
		for (auto& curChoice : choices) { 
			if (curChoice.shortValue == SKV_VALUE_GAME_WON) {
				if (bestAmountOfPlies > curChoice.plyInfo + 1) {
					bestAmountOfPlies = curChoice.plyInfo + 1;
				}
			}
		}
	// when the game is lost, then try to find the maximum amount of plies to lose
	} else if (shortValue == SKV_VALUE_GAME_LOST) {
		bestAmountOfPlies = 0;
		for (auto& curChoice : choices) { 
			if (curChoice.shortValue == SKV_VALUE_GAME_LOST) {
				if (bestAmountOfPlies < curChoice.plyInfo + 1) {
					bestAmountOfPlies = curChoice.plyInfo + 1;
				}
			}
		}
	// when the game is drawn, then try to maximize the choice leading to a state with the most options to win
	} else if (shortValue == SKV_VALUE_GAME_DRAWN) {
		bestAmountOfPlies = 0;
		for (auto& curChoice : choices) { 
			if (curChoice.shortValue == SKV_VALUE_GAME_DRAWN) {
				if (bestAmountOfPlies <= curChoice.freqValuesSubMoves[SKV_VALUE_GAME_WON]) {
					bestAmountOfPlies = curChoice.freqValuesSubMoves [SKV_VALUE_GAME_WON];
				}
			}
		}
	// invalid state
	} else {
		bestAmountOfPlies = PLYINFO_VALUE_INVALID;
	}
}

//-----------------------------------------------------------------------------
// Name: falseOrStop()
// Desc: Returns false or stops the program. 
//		 If stopOnCriticalError is set to true, the program will stop.
//		 This is practical for debugging.
//-----------------------------------------------------------------------------
bool returnValues::falseOrStop()
{
	if (returnValues::stopOnCriticalError) {
		WaitForSingleObject(GetCurrentProcess(), INFINITE);
	}
	return false;
}

#pragma region progressCounter
//-----------------------------------------------------------------------------
// Name: progressCounter()
// Desc: Constructor 
//-----------------------------------------------------------------------------
progressCounter::progressCounter(int64_t &roughTotalNumStatesProcessed) : roughTotalNumStatesProcessed(roughTotalNumStatesProcessed)
{
}

//-----------------------------------------------------------------------------
// Name: stateProcessed()
// Desc: Increments the number of processed states by one and print the progress to the log
//-----------------------------------------------------------------------------
void progressCounter::stateProcessed(logger& log, stateNumberVarType numKnotsInLayer, const wstring& text)
{
	statesProcessedByThisThread++;

	if (numKnotsInLayer == 0) return; // avoid division by zero

	// print status
	if (statesProcessedByThisThread % OUTPUT_EVERY_N_STATES == 0) { 
		roughTotalNumStatesProcessed += OUTPUT_EVERY_N_STATES;
		int percentage = (int) ((roughTotalNumStatesProcessed * 100) / numKnotsInLayer);
		if (percentage > 100) percentage = 100;
		wstringstream wss;
		wss << text << roughTotalNumStatesProcessed << " of " << numKnotsInLayer << " states being " << percentage << " %";
		log.log(logger::logLevel::info, wss.str().c_str());
	}
}

//-----------------------------------------------------------------------------
// Name: getStatesProcessedByThisThread()
// Desc: 
//-----------------------------------------------------------------------------
int64_t progressCounter::getStatesProcessedByThisThread() const
{
    return statesProcessedByThisThread;
}
#pragma endregion

} // namespace miniMax
