#include "InterferenceGraph.h"

#include <iostream>
#include <iomanip>

using namespace std;


InterferenceGraph::InterferenceGraph(Instructions& instr, Variables& vars)
	: instructions(instr), variables(vars)
{
}


void InterferenceGraph::build()
{
	// izdvajamo samo registarske promenljive
	Variables::iterator vit;
	for (vit = variables.begin(); vit != variables.end(); vit++)
	{
		if ((*vit)->getType() == Variable::REG_VAR)
			regVariables.push_back(*vit);
	}

	int size = regVariables.size();

	// inicijalizujemo matricu na 0
	matrix.resize(size);
	for (int i = 0; i < size; i++)
	{
		matrix[i].resize(size, __EMPTY__);
	}

	// za svaku instrukciju, promenljive koje su istovremeno zive se smetaju
	Instructions::iterator iit;
	for (iit = instructions.begin(); iit != instructions.end(); iit++)
	{
		Instruction* instr = *iit;
		Variables& out = instr->getOut();

		// sve promenljive u out skupu se medjusobno smetaju
		Variables::iterator it1;
		for (it1 = out.begin(); it1 != out.end(); it1++)
		{
			Variables::iterator it2 = it1;
			it2++;
			for (; it2 != out.end(); it2++)
			{
				// nadji indekse u regVariables listi
				int idx1 = -1, idx2 = -1;
				int idx = 0;
				Variables::iterator rit;
				for (rit = regVariables.begin(); rit != regVariables.end(); rit++)
				{
					if (*rit == *it1) idx1 = idx;
					if (*rit == *it2) idx2 = idx;
					idx++;
				}

				if (idx1 >= 0 && idx2 >= 0 && idx1 != idx2)
				{
					matrix[idx1][idx2] = __INTERFERENCE__;
					matrix[idx2][idx1] = __INTERFERENCE__;
				}
			}
		}

		// def promenljive se smetaju sa svim iz out skupa
		Variables& def = instr->getDef();
		Variables::iterator dit;
		for (dit = def.begin(); dit != def.end(); dit++)
		{
			Variables::iterator oit;
			for (oit = out.begin(); oit != out.end(); oit++)
			{
				if (*dit == *oit)
					continue;

				int idx1 = -1, idx2 = -1;
				int idx = 0;
				Variables::iterator rit;
				for (rit = regVariables.begin(); rit != regVariables.end(); rit++)
				{
					if (*rit == *dit) idx1 = idx;
					if (*rit == *oit) idx2 = idx;
					idx++;
				}

				if (idx1 >= 0 && idx2 >= 0 && idx1 != idx2)
				{
					matrix[idx1][idx2] = __INTERFERENCE__;
					matrix[idx2][idx1] = __INTERFERENCE__;
				}
			}
		}
	}
}


vector<vector<int>>& InterferenceGraph::getMatrix()
{
	return matrix;
}


Variables& InterferenceGraph::getRegVariables()
{
	return regVariables;
}


void InterferenceGraph::printInterferenceMatrix()
{
	int size = regVariables.size();

	cout << endl;
	cout << "=== Interference Graph ===" << endl;
	cout << endl;

	// header
	cout << setw(6) << " ";
	Variables::iterator vit;
	for (vit = regVariables.begin(); vit != regVariables.end(); vit++)
		cout << setw(4) << (*vit)->getName();
	cout << endl;

	int row = 0;
	for (vit = regVariables.begin(); vit != regVariables.end(); vit++)
	{
		cout << setw(6) << (*vit)->getName();
		for (int col = 0; col < size; col++)
		{
			cout << setw(4) << matrix[row][col];
		}
		cout << endl;
		row++;
	}
}
