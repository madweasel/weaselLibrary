/*********************************************************************\
	backTracking.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef BACKTRACKING_H
#define BACKTRACKING_H

#include <list>
#include <vector>

using namespace std;

using vectui				= vector<unsigned int>;

class backTracking
{
public:
	struct solutionPath
	{
		unsigned int		numSteps				= 0;
		vectui				path;
	};

protected: 

	// search depth
	unsigned int			curDepth;				// current depth of search
	vectui					curPath;				// possibilityIDs representing the current path
	list<solutionPath>*		solutionList;			// list with solutions
	unsigned int			maxSearchDepth;			// maximum steps
	unsigned int			numSolToFind;			// amount of solutions which still shall be found
	bool					abortCalculation;		// true when search routine shall be stopped
	
	// Functions
	void					searchSolution			(list<solutionPath> *solList, unsigned int searchDepth, unsigned int numSolutionsToFind);
	bool					tryNextStep				();

	virtual bool			targetReached			()																			{ return false; };
	virtual void			setBeginningSituation	() 																			{};
	virtual unsigned int *	deletePossibilities		(void *pPossibilities) 														{ return nullptr;  };
	virtual unsigned int *	getPossibilities		(unsigned int *numPossibilities,  void **pPossibilities)					{ return 0; };
	virtual void			undo					(unsigned int idPossibility, void  *pBackup,  void  *pPossibilities) 		{};
	virtual void			move					(unsigned int idPossibility, void **pBackup,  void  *pPossibilities) 		{};
};

#endif
