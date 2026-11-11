#include "LivenessAnalysis.h"

#include <iostream>
#include <iomanip>

using namespace std;


LivenessAnalysis::LivenessAnalysis(Instructions& instr, map<string, int>& lab)
	: instructions(instr), labels(lab)
{
}


void LivenessAnalysis::Do()
{
	buildControlFlowGraph();
	setDefUse();
	computeLiveness();
}


Instruction* LivenessAnalysis::findInstructionByPosition(int pos)
{
	Instructions::iterator it;
	for (it = instructions.begin(); it != instructions.end(); it++)
	{
		if ((*it)->getPosition() == pos)
			return *it;
	}
	return NULL;
}


bool LivenessAnalysis::variableExists(Variables& vars, Variable* var)
{
	Variables::iterator it;
	for (it = vars.begin(); it != vars.end(); it++)
	{
		if (*it == var)
			return true;
	}
	return false;
}


void LivenessAnalysis::buildControlFlowGraph()
{
	Instructions::iterator it;
	for (it = instructions.begin(); it != instructions.end(); it++)
	{
		Instruction* instr = *it;
		InstructionType type = instr->getType();

		// sledeca instrukcija u nizu
		Instructions::iterator nextIt = it;
		nextIt++;
		Instruction* nextInstr = NULL;
		if (nextIt != instructions.end())
			nextInstr = *nextIt;

		if (type == I_B)
		{
			// bezuslovni skok - samo ciljna instrukcija je sledbenik
			string target = instr->getTargetLabel();
			if (labels.find(target) != labels.end())
			{
				int targetPos = labels[target];
				Instruction* targetInstr = findInstructionByPosition(targetPos);
				if (targetInstr != NULL)
				{
					instr->getSucc().push_back(targetInstr);
					targetInstr->getPred().push_back(instr);
				}
			}
		}
		else if (type == I_BLTZ || type == I_BEQ)
		{
			// uslovni skok - sledbenik je i sledeca instrukcija i ciljna
			if (nextInstr != NULL)
			{
				instr->getSucc().push_back(nextInstr);
				nextInstr->getPred().push_back(instr);
			}

			string target = instr->getTargetLabel();
			if (labels.find(target) != labels.end())
			{
				int targetPos = labels[target];
				Instruction* targetInstr = findInstructionByPosition(targetPos);
				if (targetInstr != NULL)
				{
					instr->getSucc().push_back(targetInstr);
					targetInstr->getPred().push_back(instr);
				}
			}
		}
		else
		{
			// obicna instrukcija - sledbenik je sledeca u nizu
			if (nextInstr != NULL)
			{
				instr->getSucc().push_back(nextInstr);
				nextInstr->getPred().push_back(instr);
			}
		}
	}
}


void LivenessAnalysis::setDefUse()
{
	Instructions::iterator it;
	for (it = instructions.begin(); it != instructions.end(); it++)
	{
		Instruction* instr = *it;
		InstructionType type = instr->getType();

		Variables& dst = instr->getDst();
		Variables& src = instr->getSrc();

		switch (type)
		{
		case I_ADD:
		case I_SUB:
		case I_MUL:
		case I_AND:
			// def = dst(rid), use = src(rid, rid)
		{
			Variables::iterator dit;
			for (dit = dst.begin(); dit != dst.end(); dit++)
			{
				if ((*dit)->getType() == Variable::REG_VAR)
					instr->getDef().push_back(*dit);
			}
			Variables::iterator sit;
			for (sit = src.begin(); sit != src.end(); sit++)
			{
				if ((*sit)->getType() == Variable::REG_VAR)
					instr->getUse().push_back(*sit);
			}
			break;
		}

		case I_ADDI:
			// def = dst(rid), use = src(rid)
		{
			Variables::iterator dit;
			for (dit = dst.begin(); dit != dst.end(); dit++)
			{
				if ((*dit)->getType() == Variable::REG_VAR)
					instr->getDef().push_back(*dit);
			}
			Variables::iterator sit;
			for (sit = src.begin(); sit != src.end(); sit++)
			{
				if ((*sit)->getType() == Variable::REG_VAR)
					instr->getUse().push_back(*sit);
			}
			break;
		}

		case I_LA:
			// def = dst(rid), use = {} (src je mem var)
		{
			Variables::iterator dit;
			for (dit = dst.begin(); dit != dst.end(); dit++)
			{
				if ((*dit)->getType() == Variable::REG_VAR)
					instr->getDef().push_back(*dit);
			}
			break;
		}

		case I_LI:
			// def = dst(rid), use = {}
		{
			Variables::iterator dit;
			for (dit = dst.begin(); dit != dst.end(); dit++)
			{
				if ((*dit)->getType() == Variable::REG_VAR)
					instr->getDef().push_back(*dit);
			}
			break;
		}

		case I_LW:
			// def = dst(rid koji prima vrednost), use = src(rid iz zagrade)
		{
			Variables::iterator dit;
			for (dit = dst.begin(); dit != dst.end(); dit++)
			{
				if ((*dit)->getType() == Variable::REG_VAR)
					instr->getDef().push_back(*dit);
			}
			Variables::iterator sit;
			for (sit = src.begin(); sit != src.end(); sit++)
			{
				if ((*sit)->getType() == Variable::REG_VAR)
					instr->getUse().push_back(*sit);
			}
			break;
		}

		case I_SW:
			// oba registra se citaju, nista se ne upisuje
			// src = registar cija se vrednost cuva, dst = adresni registar
		{
			Variables::iterator sit;
			for (sit = src.begin(); sit != src.end(); sit++)
			{
				if ((*sit)->getType() == Variable::REG_VAR)
					instr->getUse().push_back(*sit);
			}
			Variables::iterator dit;
			for (dit = dst.begin(); dit != dst.end(); dit++)
			{
				if ((*dit)->getType() == Variable::REG_VAR)
					instr->getUse().push_back(*dit);
			}
			break;
		}

		case I_BLTZ:
			// use = src(rid), def = {}
		{
			Variables::iterator sit;
			for (sit = src.begin(); sit != src.end(); sit++)
			{
				if ((*sit)->getType() == Variable::REG_VAR)
					instr->getUse().push_back(*sit);
			}
			break;
		}

		case I_BEQ:
			// use = src(rid, rid), def = {}
		{
			Variables::iterator sit;
			for (sit = src.begin(); sit != src.end(); sit++)
			{
				if ((*sit)->getType() == Variable::REG_VAR)
					instr->getUse().push_back(*sit);
			}
			break;
		}

		case I_B:
		case I_NOP:
			// nema use ni def
			break;

		default:
			break;
		}
	}
}


void LivenessAnalysis::computeLiveness()
{
	bool changed = true;

	while (changed)
	{
		changed = false;

		// prolazimo unazad kroz instrukcije
		Instructions::reverse_iterator rit;
		for (rit = instructions.rbegin(); rit != instructions.rend(); rit++)
		{
			Instruction* instr = *rit;

			// sacuvamo stare skupove in i out
			Variables oldIn = instr->getIn();
			Variables oldOut = instr->getOut();

			// out[i] = unija in[s] za sve sledbenike s
			instr->getOut().clear();
			list<Instruction*>::iterator succIt;
			for (succIt = instr->getSucc().begin(); succIt != instr->getSucc().end(); succIt++)
			{
				Variables& succIn = (*succIt)->getIn();
				Variables::iterator vit;
				for (vit = succIn.begin(); vit != succIn.end(); vit++)
				{
					if (!variableExists(instr->getOut(), *vit))
						instr->getOut().push_back(*vit);
				}
			}

			// in[i] = use[i] unija (out[i] - def[i])
			instr->getIn().clear();

			// prvo dodamo use
			Variables::iterator uit;
			for (uit = instr->getUse().begin(); uit != instr->getUse().end(); uit++)
			{
				if (!variableExists(instr->getIn(), *uit))
					instr->getIn().push_back(*uit);
			}

			// zatim dodamo out - def
			Variables::iterator oit;
			for (oit = instr->getOut().begin(); oit != instr->getOut().end(); oit++)
			{
				if (!variableExists(instr->getDef(), *oit))
				{
					if (!variableExists(instr->getIn(), *oit))
						instr->getIn().push_back(*oit);
				}
			}

			// proveravamo da li se nesto promenilo
			if (instr->getIn().size() != oldIn.size() || instr->getOut().size() != oldOut.size())
			{
				changed = true;
			}
		}
	}
}


void LivenessAnalysis::printLivenessInfo(int dumps)
{
	if (dumps == __NO_DUMPS__)
		return;

	cout << endl;
	cout << "=== Liveness Analysis ===" << endl;
	cout << endl;

	Instructions::iterator it;
	for (it = instructions.begin(); it != instructions.end(); it++)
	{
		Instruction* instr = *it;

		cout << "Instruction " << instr->getPosition() << ":" << endl;

		cout << "  def: ";
		Variables::iterator vit;
		for (vit = instr->getDef().begin(); vit != instr->getDef().end(); vit++)
			cout << (*vit)->getName() << " ";
		cout << endl;

		cout << "  use: ";
		for (vit = instr->getUse().begin(); vit != instr->getUse().end(); vit++)
			cout << (*vit)->getName() << " ";
		cout << endl;

		cout << "  in:  ";
		for (vit = instr->getIn().begin(); vit != instr->getIn().end(); vit++)
			cout << (*vit)->getName() << " ";
		cout << endl;

		cout << "  out: ";
		for (vit = instr->getOut().begin(); vit != instr->getOut().end(); vit++)
			cout << (*vit)->getName() << " ";
		cout << endl;

		cout << "  succ: ";
		list<Instruction*>::iterator sit;
		for (sit = instr->getSucc().begin(); sit != instr->getSucc().end(); sit++)
			cout << (*sit)->getPosition() << " ";
		cout << endl;

		cout << endl;
	}
}
