#include "SyntaxAnalysis.h"

#include <iostream>
#include <iomanip>
#include <map>

using namespace std;


SyntaxAnalysis::SyntaxAnalysis(LexicalAnalysis& lex)
	: lexicalAnalysis(lex), instructionPosition(0)
{
	currentToken = lexicalAnalysis.getTokenList().begin();
}


bool SyntaxAnalysis::Do()
{
	try
	{
		Q();

		// postavi labele na odgovarajuce instrukcije
		map<string, int>::iterator lit;
		for (lit = labels.begin(); lit != labels.end(); lit++)
		{
			Instructions::iterator iit;
			for (iit = instructions.begin(); iit != instructions.end(); iit++)
			{
				if ((*iit)->getPosition() == lit->second)
				{
					(*iit)->setLabel(lit->first);
				}
			}
		}

		return true;
	}
	catch (runtime_error& e)
	{
		cout << e.what() << endl;
		return false;
	}
}


Instructions& SyntaxAnalysis::getInstructions()
{
	return instructions;
}


Variables& SyntaxAnalysis::getVariables()
{
	return variables;
}


string SyntaxAnalysis::getFunctionName()
{
	return functionName;
}


map<string, int>& SyntaxAnalysis::getLabels()
{
	return labels;
}


void SyntaxAnalysis::readNextToken()
{
	if (currentToken != lexicalAnalysis.getTokenList().end())
	{
		currentToken++;
	}
}


bool SyntaxAnalysis::checkToken(TokenType expected)
{
	return currentToken->getType() == expected;
}


void SyntaxAnalysis::eatToken(TokenType expected)
{
	if (currentToken->getType() != expected)
	{
		printSyntaxError(*currentToken);
		throw runtime_error("Syntax error!");
	}
	readNextToken();
}


Variable* SyntaxAnalysis::findVariable(string name)
{
	Variables::iterator it;
	for (it = variables.begin(); it != variables.end(); it++)
	{
		if ((*it)->getName() == name)
			return *it;
	}
	return NULL;
}


void SyntaxAnalysis::printSyntaxError(Token token)
{
	cout << "Syntax error! Token: ";
	token.printTokenInfo();
}


// Q -> S ; L
void SyntaxAnalysis::Q()
{
	S();
	eatToken(T_SEMI_COL);
	L();
}


// L -> eof | Q
void SyntaxAnalysis::L()
{
	if (checkToken(T_END_OF_FILE))
	{
		// kraj programa
		return;
	}
	else
	{
		Q();
	}
}


// S -> _mem mid num | _reg rid | _func id | id: E | E
void SyntaxAnalysis::S()
{
	if (checkToken(T_MEM))
	{
		// S -> _mem mid num
		readNextToken();

		string memName = currentToken->getValue();
		eatToken(T_M_ID);

		int memValue = atoi(currentToken->getValue().c_str());
		eatToken(T_NUM);

		Variable* var = new Variable(memName, variables.size());
		var->setType(Variable::MEM_VAR);
		var->setValue(memValue);
		variables.push_back(var);
	}
	else if (checkToken(T_REG))
	{
		// S -> _reg rid
		readNextToken();

		string regName = currentToken->getValue();
		eatToken(T_R_ID);

		Variable* var = new Variable(regName, variables.size());
		var->setType(Variable::REG_VAR);
		variables.push_back(var);
	}
	else if (checkToken(T_FUNC))
	{
		// S -> _func id
		readNextToken();

		functionName = currentToken->getValue();
		eatToken(T_ID);
	}
	else if (checkToken(T_ID))
	{
		// S -> id: E  (labela)
		string labelName = currentToken->getValue();
		readNextToken();

		eatToken(T_COL);

		labels[labelName] = instructionPosition;

		E();
	}
	else
	{
		// S -> E
		E();
	}
}


// E -> add rid, rid, rid
// E -> addi rid, rid, num
// E -> sub rid, rid, rid
// E -> la rid, mid
// E -> lw rid, num(rid)
// E -> li rid, num
// E -> sw rid, num(rid)
// E -> b id
// E -> bltz rid, id
// E -> nop
// E -> beq rid, rid, id
// E -> mul rid, rid, rid
// E -> and rid, rid, rid
void SyntaxAnalysis::E()
{
	Variables dst;
	Variables src;
	InstructionType type = I_NO_TYPE;
	string targetLabel = "";
	int numConst = 0;

	if (checkToken(T_ADD))
	{
		// add rid, rid, rid
		type = I_ADD;
		readNextToken();

		string dstName = currentToken->getValue();
		eatToken(T_R_ID);
		eatToken(T_COMMA);

		string src1Name = currentToken->getValue();
		eatToken(T_R_ID);
		eatToken(T_COMMA);

		string src2Name = currentToken->getValue();
		eatToken(T_R_ID);

		Variable* dstVar = findVariable(dstName);
		Variable* src1Var = findVariable(src1Name);
		Variable* src2Var = findVariable(src2Name);

		dst.push_back(dstVar);
		src.push_back(src1Var);
		src.push_back(src2Var);
	}
	else if (checkToken(T_ADDI))
	{
		// addi rid, rid, num
		type = I_ADDI;
		readNextToken();

		string dstName = currentToken->getValue();
		eatToken(T_R_ID);
		eatToken(T_COMMA);

		string srcName = currentToken->getValue();
		eatToken(T_R_ID);
		eatToken(T_COMMA);

		numConst = atoi(currentToken->getValue().c_str());
		eatToken(T_NUM);

		Variable* dstVar = findVariable(dstName);
		Variable* srcVar = findVariable(srcName);

		dst.push_back(dstVar);
		src.push_back(srcVar);
	}
	else if (checkToken(T_SUB))
	{
		// sub rid, rid, rid
		type = I_SUB;
		readNextToken();

		string dstName = currentToken->getValue();
		eatToken(T_R_ID);
		eatToken(T_COMMA);

		string src1Name = currentToken->getValue();
		eatToken(T_R_ID);
		eatToken(T_COMMA);

		string src2Name = currentToken->getValue();
		eatToken(T_R_ID);

		Variable* dstVar = findVariable(dstName);
		Variable* src1Var = findVariable(src1Name);
		Variable* src2Var = findVariable(src2Name);

		dst.push_back(dstVar);
		src.push_back(src1Var);
		src.push_back(src2Var);
	}
	else if (checkToken(T_LA))
	{
		// la rid, mid
		type = I_LA;
		readNextToken();

		string dstName = currentToken->getValue();
		eatToken(T_R_ID);
		eatToken(T_COMMA);

		string srcName = currentToken->getValue();
		eatToken(T_M_ID);

		Variable* dstVar = findVariable(dstName);
		Variable* srcVar = findVariable(srcName);

		dst.push_back(dstVar);
		src.push_back(srcVar);
	}
	else if (checkToken(T_LI))
	{
		// li rid, num
		type = I_LI;
		readNextToken();

		string dstName = currentToken->getValue();
		eatToken(T_R_ID);
		eatToken(T_COMMA);

		numConst = atoi(currentToken->getValue().c_str());
		eatToken(T_NUM);

		Variable* dstVar = findVariable(dstName);
		dst.push_back(dstVar);
	}
	else if (checkToken(T_LW))
	{
		// lw rid, num(rid)
		type = I_LW;
		readNextToken();

		string dstName = currentToken->getValue();
		eatToken(T_R_ID);
		eatToken(T_COMMA);

		numConst = atoi(currentToken->getValue().c_str());
		eatToken(T_NUM);
		eatToken(T_L_PARENT);

		string srcName = currentToken->getValue();
		eatToken(T_R_ID);
		eatToken(T_R_PARENT);

		Variable* dstVar = findVariable(dstName);
		Variable* srcVar = findVariable(srcName);

		dst.push_back(dstVar);
		src.push_back(srcVar);
	}
	else if (checkToken(T_SW))
	{
		// sw rid, num(rid)
		type = I_SW;
		readNextToken();

		string srcName = currentToken->getValue();
		eatToken(T_R_ID);
		eatToken(T_COMMA);

		numConst = atoi(currentToken->getValue().c_str());
		eatToken(T_NUM);
		eatToken(T_L_PARENT);

		string dstName = currentToken->getValue();
		eatToken(T_R_ID);
		eatToken(T_R_PARENT);

		Variable* srcVar = findVariable(srcName);
		Variable* dstVar = findVariable(dstName);

		src.push_back(srcVar);
		dst.push_back(dstVar);
	}
	else if (checkToken(T_B))
	{
		// b id
		type = I_B;
		readNextToken();

		targetLabel = currentToken->getValue();
		eatToken(T_ID);
	}
	else if (checkToken(T_BLTZ))
	{
		// bltz rid, id
		type = I_BLTZ;
		readNextToken();

		string srcName = currentToken->getValue();
		eatToken(T_R_ID);
		eatToken(T_COMMA);

		targetLabel = currentToken->getValue();
		eatToken(T_ID);

		Variable* srcVar = findVariable(srcName);
		src.push_back(srcVar);
	}
	else if (checkToken(T_NOP))
	{
		// nop
		type = I_NOP;
		readNextToken();
	}
	else if (checkToken(T_BEQ))
	{
		// beq rid, rid, id
		type = I_BEQ;
		readNextToken();

		string src1Name = currentToken->getValue();
		eatToken(T_R_ID);
		eatToken(T_COMMA);

		string src2Name = currentToken->getValue();
		eatToken(T_R_ID);
		eatToken(T_COMMA);

		targetLabel = currentToken->getValue();
		eatToken(T_ID);

		Variable* src1Var = findVariable(src1Name);
		Variable* src2Var = findVariable(src2Name);

		src.push_back(src1Var);
		src.push_back(src2Var);
	}
	else if (checkToken(T_MUL))
	{
		// mul rid, rid, rid
		type = I_MUL;
		readNextToken();

		string dstName = currentToken->getValue();
		eatToken(T_R_ID);
		eatToken(T_COMMA);

		string src1Name = currentToken->getValue();
		eatToken(T_R_ID);
		eatToken(T_COMMA);

		string src2Name = currentToken->getValue();
		eatToken(T_R_ID);

		Variable* dstVar = findVariable(dstName);
		Variable* src1Var = findVariable(src1Name);
		Variable* src2Var = findVariable(src2Name);

		dst.push_back(dstVar);
		src.push_back(src1Var);
		src.push_back(src2Var);
	}
	else if (checkToken(T_AND))
	{
		// and rid, rid, rid
		type = I_AND;
		readNextToken();

		string dstName = currentToken->getValue();
		eatToken(T_R_ID);
		eatToken(T_COMMA);

		string src1Name = currentToken->getValue();
		eatToken(T_R_ID);
		eatToken(T_COMMA);

		string src2Name = currentToken->getValue();
		eatToken(T_R_ID);

		Variable* dstVar = findVariable(dstName);
		Variable* src1Var = findVariable(src1Name);
		Variable* src2Var = findVariable(src2Name);

		dst.push_back(dstVar);
		src.push_back(src1Var);
		src.push_back(src2Var);
	}
	else
	{
		printSyntaxError(*currentToken);
		throw runtime_error("Syntax error! Unexpected token in instruction.");
	}

	Instruction* instr = new Instruction(instructionPosition, type, dst, src);
	instr->setNumber(instructionPosition);
	instr->setTargetLabel(targetLabel);
	instr->setNumConst(numConst);
	instructions.push_back(instr);
	instructionPosition++;
}
