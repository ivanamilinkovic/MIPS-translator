#include "ResourceAllocation.h"

#include <iostream>

using namespace std;


ResourceAllocation::ResourceAllocation(InterferenceGraph& ig)
	: interferenceGraph(ig)
{
}


bool ResourceAllocation::Do()
{
	if (!simplify())
		return false;

	if (!select())
		return false;

	return true;
}


int ResourceAllocation::getDegree(int varIndex, vector<vector<int>>& matrix)
{
	int degree = 0;
	int size = matrix.size();
	for (int i = 0; i < size; i++)
	{
		if (matrix[varIndex][i] == __INTERFERENCE__)
			degree++;
	}
	return degree;
}


bool ResourceAllocation::simplify()
{
	Variables& regVars = interferenceGraph.getRegVariables();
	vector<vector<int>> tempMatrix = interferenceGraph.getMatrix();
	int size = regVars.size();

	vector<bool> removed(size, false);
	int removedCount = 0;

	while (removedCount < size)
	{
		bool found = false;
		int bestIdx = -1;
		int bestDegree = -1;

		// trazimo cvor sa stepenom < __REG_NUMBER__
		int idx = 0;
		Variables::iterator vit;
		for (vit = regVars.begin(); vit != regVars.end(); vit++)
		{
			if (!removed[idx])
			{
				int degree = getDegree(idx, tempMatrix);
				if (degree < __REG_NUMBER__)
				{
					// nasli smo cvor koji mozemo bezbedno ukloniti
					bestIdx = idx;
					found = true;
					break;
				}
				// pamtimo cvor sa najvecim stepenom za potencijalni spill
				if (degree > bestDegree)
				{
					bestDegree = degree;
					bestIdx = idx;
				}
			}
			idx++;
		}

		if (!found && bestIdx == -1)
		{
			return false;
		}

		// uklanjamo cvor (ili bezbedan ili potencijalni spill)
		idx = 0;
		for (vit = regVars.begin(); vit != regVars.end(); vit++)
		{
			if (idx == bestIdx)
			{
				simplifyStack.push(*vit);
				break;
			}
			idx++;
		}

		removed[bestIdx] = true;
		removedCount++;

		// brisemo grane
		for (int i = 0; i < size; i++)
		{
			tempMatrix[bestIdx][i] = __EMPTY__;
			tempMatrix[i][bestIdx] = __EMPTY__;
		}
	}

	return true;
}


bool ResourceAllocation::select()
{
	Variables& regVars = interferenceGraph.getRegVariables();
	vector<vector<int>>& matrix = interferenceGraph.getMatrix();
	int size = regVars.size();

	Regs availableRegs[] = { t0, t1, t2, t3 };

	while (!simplifyStack.empty())
	{
		Variable* var = simplifyStack.top();
		simplifyStack.pop();

		// nadji indeks ove promenljive
		int varIdx = 0;
		Variables::iterator vit;
		for (vit = regVars.begin(); vit != regVars.end(); vit++)
		{
			if (*vit == var)
				break;
			varIdx++;
		}

		// nadji koji registri su zauzeti od strane suseda
		bool usedRegs[__REG_NUMBER__] = { false };

		int neighborIdx = 0;
		for (vit = regVars.begin(); vit != regVars.end(); vit++)
		{
			if (neighborIdx != varIdx && matrix[varIdx][neighborIdx] == __INTERFERENCE__)
			{
				Regs neighborReg = (*vit)->getAssignment();
				if (neighborReg != no_assign)
				{
					usedRegs[neighborReg - 1] = true;
				}
			}
			neighborIdx++;
		}

		// dodeli prvi slobodan registar
		bool assigned = false;
		for (int i = 0; i < __REG_NUMBER__; i++)
		{
			if (!usedRegs[i])
			{
				var->setAssignment(availableRegs[i]);
				assigned = true;
				break;
			}
		}

		if (!assigned)
		{
			// spill - dodeli registar ali oznaci kao spilled
			var->setAssignment(availableRegs[0]);
			var->setSpilled(true);
			cout << "Warning: Variable " << var->getName() << " spilled to memory." << endl;
		}
	}

	return true;
}


void ResourceAllocation::printAllocation()
{
	cout << endl;
	cout << "=== Register Allocation ===" << endl;
	cout << endl;

	Variables& regVars = interferenceGraph.getRegVariables();
	Variables::iterator vit;
	for (vit = regVars.begin(); vit != regVars.end(); vit++)
	{
		string regName;
		switch ((*vit)->getAssignment())
		{
		case t0: regName = "$t0"; break;
		case t1: regName = "$t1"; break;
		case t2: regName = "$t2"; break;
		case t3: regName = "$t3"; break;
		default: regName = "none"; break;
		}
		cout << (*vit)->getName() << " -> " << regName << endl;
	}
}
