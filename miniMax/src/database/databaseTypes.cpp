/*********************************************************************
	databaseTypes.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "databaseTypes.h"

namespace miniMax
{

namespace database
{

bool databaseStatsStruct::operator==(const databaseStatsStruct& other) const
{
    return completed == other.completed && numLayers == other.numLayers;
}

bool layerStatsStruct::operator==(const layerStatsStruct& other) const
{
    bool isEqual = completedAndInFile == other.completedAndInFile
        && partnerLayers == other.partnerLayers
        && knotsInLayer == other.knotsInLayer 
        && numWonStates == other.numWonStates 
        && numLostStates == other.numLostStates 
        && numDrawnStates == other.numDrawnStates 
        && numInvalidStates == other.numInvalidStates 
        && succLayers == other.succLayers 
        && skv == other.skv 
        && plyInfo == other.plyInfo;
    return isEqual;
}

long long layerStatsStruct::getLayerSizeInBytesForSkv() const
{
    // Rounds up to the nearest multiple of 4 to ensure enough storage for packing 2-bit values per knot.
    return ((knotsInLayer + 3) / 4) * sizeof(twoBit);
}

long long layerStatsStruct::getLayerSizeInBytesForPlyInfo() const
{
    return knotsInLayer * sizeof(plyInfoVarType);
}

layerStatsStruct::layerStatsStruct() = default;

layerStatsStruct::~layerStatsStruct() = default;

} // namespace database

} // namespace miniMax
