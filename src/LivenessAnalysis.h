#pragma once

#include "IR.h"

#include <map>

class LivenessAnalysis
{
public:
	LivenessAnalysis(Instructions& instr, std::map<std::string, int>& lab);

	/**
	 * Performs liveness analysis
	 */
	void Do();

	/**
	 * Prints liveness analysis results
	 */
	void printLivenessInfo(int dumps);

private:
	/**
	 * Reference to instructions list
	 */
	Instructions& instructions;

	/**
	 * Reference to labels map
	 */
	std::map<std::string, int>& labels;

	/**
	 * Builds control flow graph (succ/pred)
	 */
	void buildControlFlowGraph();

	/**
	 * Sets def and use sets for each instruction
	 */
	void setDefUse();

	/**
	 * Performs iterative liveness computation
	 */
	void computeLiveness();

	/**
	 * Helper: finds instruction by position
	 */
	Instruction* findInstructionByPosition(int pos);

	/**
	 * Helper: checks if variable is in list
	 */
	bool variableExists(Variables& vars, Variable* var);
};
