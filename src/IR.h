#pragma once

#include "Types.h"


/**
 * This class represents one variable from program code.
 */
class Variable
{
public:
	enum VariableType
	{
		MEM_VAR,
		REG_VAR,
		NO_TYPE
	};

	Variable() : m_type(NO_TYPE), m_name(""), m_position(-1), m_assignment(no_assign), m_value(0), m_spilled(false) {}
	Variable(std::string name, int pos) : m_type(NO_TYPE), m_name(name), m_position(pos), m_assignment(no_assign), m_value(0), m_spilled(false) {}

	std::string getName() { return m_name; }
	void setName(std::string name) { m_name = name; }

	int getPosition() { return m_position; }
	void setPosition(int pos) { m_position = pos; }

	VariableType getType() { return m_type; }
	void setType(VariableType type) { m_type = type; }

	Regs getAssignment() { return m_assignment; }
	void setAssignment(Regs reg) { m_assignment = reg; }

	int getValue() { return m_value; }
	void setValue(int val) { m_value = val; }

	bool isSpilled() { return m_spilled; }
	void setSpilled(bool spilled) { m_spilled = spilled; }

private:
	VariableType m_type;
	std::string m_name;
	int m_position;
	Regs m_assignment;
	int m_value;
	bool m_spilled;
};


/**
 * This type represents list of variables from program code.
 */
typedef std::list<Variable*> Variables;

/**
 * This class represents one instruction in program code.
 */
class Instruction
{
public:
	Instruction () : m_position(0), m_type(I_NO_TYPE), m_label(""), m_number(0), m_targetLabel(""), m_numConst(0) {}
	Instruction (int pos, InstructionType type, Variables& dst, Variables& src) :
		m_position(pos), m_type(type), m_dst(dst), m_src(src), m_label(""), m_number(0), m_targetLabel(""), m_numConst(0) {}

	int getPosition() { return m_position; }
	void setPosition(int pos) { m_position = pos; }

	InstructionType getType() { return m_type; }
	void setType(InstructionType type) { m_type = type; }

	Variables& getDst() { return m_dst; }
	Variables& getSrc() { return m_src; }

	Variables& getUse() { return m_use; }
	Variables& getDef() { return m_def; }
	Variables& getIn() { return m_in; }
	Variables& getOut() { return m_out; }

	std::list<Instruction*>& getSucc() { return m_succ; }
	std::list<Instruction*>& getPred() { return m_pred; }

	std::string getLabel() { return m_label; }
	void setLabel(std::string label) { m_label = label; }

	int getNumber() { return m_number; }
	void setNumber(int num) { m_number = num; }

	std::string getTargetLabel() { return m_targetLabel; }
	void setTargetLabel(std::string label) { m_targetLabel = label; }

	int getNumConst() { return m_numConst; }
	void setNumConst(int num) { m_numConst = num; }

private:
	int m_position;
	InstructionType m_type;

	Variables m_dst;
	Variables m_src;

	Variables m_use;
	Variables m_def;
	Variables m_in;
	Variables m_out;
	std::list<Instruction*> m_succ;
	std::list<Instruction*> m_pred;

	std::string m_label;
	int m_number;
	std::string m_targetLabel;
	int m_numConst;
};


/**
 * This type represents list of instructions from program code.
 */
typedef std::list<Instruction*> Instructions;
