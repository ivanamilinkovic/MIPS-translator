#include "LexicalAnalysis.h"
#include "SyntaxAnalysis.h"
#include "LivenessAnalysis.h"
#include "InterferenceGraph.h"
#include "ResourceAllocation.h"

#include <iostream>
#include <exception>
#include <fstream>

using namespace std;

string getRegName(Variable* var)
{
	switch (var->getAssignment())
	{
	case t0: return "$t0";
	case t1: return "$t1";
	case t2: return "$t2";
	case t3: return "$t3";
	default: return "$??";
	}
}


void loadSpilled(ofstream& outFile, Variable* var)
{
	if (var->getType() == Variable::REG_VAR && var->isSpilled())
	{
		outFile << "\tlw\t" << getRegName(var) << ", __spill_" << var->getName() << endl;
	}
}

void storeSpilled(ofstream& outFile, Variable* var)
{
	if (var->getType() == Variable::REG_VAR && var->isSpilled())
	{
		outFile << "\tsw\t" << getRegName(var) << ", __spill_" << var->getName() << endl;
	}
}


void generateMipsCode(Instructions& instructions, Variables& variables, string functionName, string outputFileName)
{
	ofstream outFile(outputFileName);

	if (!outFile)
	{
		throw runtime_error("Cannot open output file!");
	}

	// .globl
	outFile << ".globl " << functionName << endl;
	outFile << endl;

	// .data sekcija
	outFile << ".data" << endl;
	Variables::iterator vit;
	for (vit = variables.begin(); vit != variables.end(); vit++)
	{
		if ((*vit)->getType() == Variable::MEM_VAR)
		{
			outFile << (*vit)->getName() << ":\t.word " << (*vit)->getValue() << endl;
		}
	}
	// spill lokacije za spilled promenljive
	for (vit = variables.begin(); vit != variables.end(); vit++)
	{
		if ((*vit)->getType() == Variable::REG_VAR && (*vit)->isSpilled())
		{
			outFile << "__spill_" << (*vit)->getName() << ":\t.word 0" << endl;
		}
	}
	outFile << endl;

	// .text sekcija
	outFile << ".text" << endl;
	outFile << functionName << ":" << endl;

	Instructions::iterator iit;
	for (iit = instructions.begin(); iit != instructions.end(); iit++)
	{
		Instruction* instr = *iit;

		if (instr->getLabel() != "")
		{
			outFile << instr->getLabel() << ":" << endl;
		}

		InstructionType type = instr->getType();
		Variables& dst = instr->getDst();
		Variables& src = instr->getSrc();

		// ucitaj spilled src promenljive pre instrukcije
		Variables::iterator sit;
		for (sit = src.begin(); sit != src.end(); sit++)
			loadSpilled(outFile, *sit);

		// za sw, dst je adresni registar koji se cita, pa i njega treba ucitati
		if (type == I_SW)
		{
			Variables::iterator dit;
			for (dit = dst.begin(); dit != dst.end(); dit++)
				loadSpilled(outFile, *dit);
		}

		switch (type)
		{
		case I_ADD:
		{
			Variables::iterator dit = dst.begin();
			Variables::iterator sit = src.begin();
			Variable* s1 = *sit; sit++;
			Variable* s2 = *sit;
			outFile << "\tadd\t" << getRegName(*dit) << ", " << getRegName(s1) << ", " << getRegName(s2) << endl;
			break;
		}
		case I_ADDI:
		{
			Variables::iterator dit = dst.begin();
			Variables::iterator sit = src.begin();
			outFile << "\taddi\t" << getRegName(*dit) << ", " << getRegName(*sit) << ", " << instr->getNumConst() << endl;
			break;
		}
		case I_SUB:
		{
			Variables::iterator dit = dst.begin();
			Variables::iterator sit = src.begin();
			Variable* s1 = *sit; sit++;
			Variable* s2 = *sit;
			outFile << "\tsub\t" << getRegName(*dit) << ", " << getRegName(s1) << ", " << getRegName(s2) << endl;
			break;
		}
		case I_LA:
		{
			Variables::iterator dit = dst.begin();
			Variables::iterator sit = src.begin();
			outFile << "\tla\t" << getRegName(*dit) << ", " << (*sit)->getName() << endl;
			break;
		}
		case I_LI:
		{
			Variables::iterator dit = dst.begin();
			outFile << "\tli\t" << getRegName(*dit) << ", " << instr->getNumConst() << endl;
			break;
		}
		case I_LW:
		{
			Variables::iterator dit = dst.begin();
			Variables::iterator sit = src.begin();
			outFile << "\tlw\t" << getRegName(*dit) << ", " << instr->getNumConst() << "(" << getRegName(*sit) << ")" << endl;
			break;
		}
		case I_SW:
		{
			Variables::iterator sit = src.begin();
			Variables::iterator dit = dst.begin();
			outFile << "\tsw\t" << getRegName(*sit) << ", " << instr->getNumConst() << "(" << getRegName(*dit) << ")" << endl;
			break;
		}
		case I_B:
		{
			outFile << "\tb\t" << instr->getTargetLabel() << endl;
			break;
		}
		case I_BLTZ:
		{
			Variables::iterator sit = src.begin();
			outFile << "\tbltz\t" << getRegName(*sit) << ", " << instr->getTargetLabel() << endl;
			break;
		}
		case I_NOP:
		{
			outFile << "\tnop" << endl;
			break;
		}
		case I_BEQ:
		{
			Variables::iterator sit = src.begin();
			Variable* s1 = *sit; sit++;
			Variable* s2 = *sit;
			outFile << "\tbeq\t" << getRegName(s1) << ", " << getRegName(s2) << ", " << instr->getTargetLabel() << endl;
			break;
		}
		case I_MUL:
		{
			Variables::iterator dit = dst.begin();
			Variables::iterator sit = src.begin();
			Variable* s1 = *sit; sit++;
			Variable* s2 = *sit;
			outFile << "\tmul\t" << getRegName(*dit) << ", " << getRegName(s1) << ", " << getRegName(s2) << endl;
			break;
		}
		case I_AND:
		{
			Variables::iterator dit = dst.begin();
			Variables::iterator sit = src.begin();
			Variable* s1 = *sit; sit++;
			Variable* s2 = *sit;
			outFile << "\tand\t" << getRegName(*dit) << ", " << getRegName(s1) << ", " << getRegName(s2) << endl;
			break;
		}
		default:
			break;
		}

		// sacuvaj spilled dst promenljive posle instrukcije (osim za sw gde je dst input)
		if (type != I_SW)
		{
			Variables::iterator dit;
			for (dit = dst.begin(); dit != dst.end(); dit++)
				storeSpilled(outFile, *dit);
		}
	}

	outFile.close();
}


void main()
{
	try
	{
		std::string fileName = ".\\..\\examples\\simple.mavn";
		bool retVal = false;

		LexicalAnalysis lex;

		if (!lex.readInputFile(fileName))
			throw runtime_error("\nException! Failed to open input file!\n");

		lex.initialize();

		retVal = lex.Do();

		if (retVal)
		{
			cout << "Lexical analysis finished successfully!" << endl;
			lex.printTokens();
		}
		else
		{
			lex.printLexError();
			throw runtime_error("\nException! Lexical analysis failed!\n");
		}

		// Syntax analysis
		SyntaxAnalysis syntax(lex);

		retVal = syntax.Do();

		if (retVal)
		{
			cout << endl << "Syntax analysis finished successfully!" << endl;
		}
		else
		{
			throw runtime_error("\nException! Syntax analysis failed!\n");
		}

		// Liveness analysis
		LivenessAnalysis liveness(syntax.getInstructions(), syntax.getLabels());
		liveness.Do();
		liveness.printLivenessInfo(__DUMPS__);

		cout << endl << "Liveness analysis finished successfully!" << endl;

		// Interference graph
		InterferenceGraph ig(syntax.getInstructions(), syntax.getVariables());
		ig.build();
		ig.printInterferenceMatrix();

		cout << endl << "Interference graph built successfully!" << endl;

		// Resource allocation
		ResourceAllocation ra(ig);
		retVal = ra.Do();

		if (retVal)
		{
			ra.printAllocation();
			cout << endl << "Resource allocation finished successfully!" << endl;
		}
		else
		{
			throw runtime_error("\nException! Resource allocation failed!\n");
		}

		// Code generation
		string outputFileName = ".\\..\\examples\\simple.s";
		generateMipsCode(syntax.getInstructions(), syntax.getVariables(), syntax.getFunctionName(), outputFileName);

		cout << endl << "Code generation finished! Output: " << outputFileName << endl;

	}
	catch (runtime_error e)
	{
		cout << e.what() << endl;
	}
}
