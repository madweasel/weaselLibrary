/***************************************************************************************************************************
	dbCompTrans.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
***************************************************************************************************************************/
#pragma once

#include "databaseTypes.h"
#include "databaseFile.h"

namespace miniMax
{

class dbCompTrans : public gameInterface
{
public:	
											dbCompTrans								();
											~dbCompTrans							();
	bool									compressDataBase						(wstring const& fromFilepath, wstring const& toFilepath);
	
	template <typename F, typename T> bool 	checkIfEqual(wstring const& fromFilepath, wstring const& toFilepath);

private:				

	static long long						calcCheckSum							(void * pByte, size_t numBytes);
	static bool								readFromFileAndCalcCheckSum				(database::genericFile & file, unsigned int layerNum, vector<database::layerStatsStruct>& layerStats, long long & checkSumSkv, long long & checkSumPlyInfo);
	static bool								printError								(wstring const & message);
	template <typename F, typename T> bool	transferDataBase						(wstring const& fromFilepath, wstring const& toFilepath, database::genericFile & fileFrom, database::genericFile & fileTo);
	template <typename F, typename T> bool 	transferDataBase						(wstring const& fromFilepath, wstring const& toFilepath);
};

} // namespace miniMax