#pragma once

#include "IR.h"
#include "Constants.h"

#include <vector>

class InterferenceGraph
{
public:
	InterferenceGraph(Instructions& instr, Variables& vars);

	/**
	 * Builds the interference graph from liveness analysis results
	 */
	void build();

	/**
	 * Returns the interference matrix
	 */
	std::vector<std::vector<int>>& getMatrix();

	/**
	 * Returns the list of register variables
	 */
	Variables& getRegVariables();

	/**
	 * Prints interference matrix
	 */
	void printInterferenceMatrix();

private:
	Instructions& instructions;
	Variables& variables;

	/**
	 * Only register variables participate in interference
	 */
	Variables regVariables;

	/**
	 * Interference matrix (size = number of reg variables)
	 */
	std::vector<std::vector<int>> matrix;
};
