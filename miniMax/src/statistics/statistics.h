/*********************************************************************\
	strLib.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "../miniMax.h"
#include "../typeDef.h"
#include "weaselEssentials/src/logger.h"

namespace miniMax
{

namespace statistics
{
	class monitor
	{
	friend class miniMax;

	public:
		void					showMemoryStatus				();
		bool					calcLayerStatistics				(const char *statisticsFileName);
		bool					calcMaxPlyInfo					(unsigned int layerNumber, plyInfoVarType &maxPlyInfoWon, stateNumberVarType &maxPlyStateWon, plyInfoVarType &maxPlyInfoLost, stateNumberVarType &maxPlyStateLost);
		void		 			getCurrentCalculatedLayer		(vector<unsigned int> &layers);
		const wchar_t* 			getCurrentActionStr				();

								monitor							(miniMax* m, logger& log);
	private:
		miniMax	*				mm								= nullptr;
		database::database *	db								= nullptr;
		gameInterface *			game							= nullptr;
		logger&					log;
	};

} // namespace statistics

} // namespace miniMax