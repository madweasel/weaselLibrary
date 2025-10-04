/*********************************************************************\
	databaseTypes.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#pragma once

#include <vector>
#include <shared_mutex>

#include "../typeDef.h"

namespace miniMax
{

namespace database
{
    using namespace std;

	// basic information about the database
	struct databaseStatsStruct
	{
		bool						completed						= false;					// true if all states have been calculated
		unsigned int				numLayers						= 0;						// number of layers

        bool 						operator==						(const databaseStatsStruct &other) const;
    };

    // layer specific information
	#pragma pack(push, 1)																		// align the following struct to byte boundary. only this guarantess stable byte order in release and debug mode
	struct layerStatsStruct
	{
		bool						completedAndInFile				= false;					// true if all states have been calculated and are stored in the file
		unsigned char 				dummy[3];													// aligns 'completedAndInFile' bool to 4 bytes for file compatibility
		unsigned int				partnerLayer					= 0;						// layer being calculated at the same time as this layer. This is a legacy variable, to be able to read the old database files.
		stateNumberVarType			knotsInLayer					= 0;						// number of knots of the corresponding layer
		stateNumberVarType			numWonStates					= 0;						// number of won states in this layer
		stateNumberVarType			numLostStates					= 0;						// number of lost states in this layer
		stateNumberVarType			numDrawnStates					= 0;						// number of drawn states in this layer
		stateNumberVarType			numInvalidStates				= 0;						// number of invalid states in this layer

		vector<unsigned int>		partnerLayers;												// layer id relevant when switching current and opponent player
		vector<unsigned int>		succLayers;													// array containing the layer ids of the succeding layers
		vector<twoBit>				skv;														// array of size [(knotsInLayer + 3) / 4] containing the short knot values
		vector<plyInfoVarType>		plyInfo;													// array of size [knotsInLayer] containing the ply info for each knot in this layer
		bool 						isPlyInfoResized 				= false;					// true if the ply info array has been resized. this is needed for the ply info array to be resized in the database file
		bool						isSkvResized					= false;					// true if the skv array has been resized. this is needed for the skv array to be resized in the database file

		// only these bytes are saved to file.
		static const size_t 		numBytesLayerStatsHeader 		= sizeof(completedAndInFile	) + sizeof(dummy)	// bool is 1 byte, but alignment is 4 bytes. this depends on the compiler and the compiler settings
																	+ sizeof(partnerLayer		)		
																	+ sizeof(knotsInLayer		)		
																	+ sizeof(numWonStates		)		
																	+ sizeof(numLostStates		)		
																	+ sizeof(numDrawnStates		)		
																	+ sizeof(numInvalidStates	);	

		layerStatsStruct();
		~layerStatsStruct();

        bool 						operator==						(const layerStatsStruct &other) const;
        long long 					getLayerSizeInBytesForSkv		() const;
        long long 					getLayerSizeInBytesForPlyInfo	() const;
    };
#pragma pack(pop)
  
} // namespace database

} // namespace miniMax
