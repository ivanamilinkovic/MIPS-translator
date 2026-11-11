#pragma once

#include "IR.h"
#include "InterferenceGraph.h"
#include "Constants.h"

#include <vector>
#include <stack>

class ResourceAllocation
{
public:
	ResourceAllocation(InterferenceGraph& ig);

	/**
	 * Performs register allocation using graph coloring
	 * Returns true if successful, false if spill needed
	 */
	bool Do();

	/**
	 * Prints allocation results
	 */
	void printAllocation();

private:
	InterferenceGraph& interferenceGraph;

	/**
	 * Stack for simplify phase
	 */
	std::stack<Variable*> simplifyStack;

	/**
	 * Simplify phase - remove nodes with degree < __REG_NUMBER__
	 */
	bool simplify();

	/**
	 * Select phase - assign colors (registers) to variables
	 */
	bool select();

	/**
	 * Returns the degree of a variable in the interference graph
	 */
	int getDegree(int varIndex, std::vector<std::vector<int>>& matrix);
};
