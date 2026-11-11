#pragma once

#include "LexicalAnalysis.h"
#include "IR.h"

class SyntaxAnalysis
{
public:
	SyntaxAnalysis(LexicalAnalysis& lex);

	/**
	 * Method which performs syntax analysis
	 * Returns true if syntax is correct, false otherwise
	 */
	bool Do();

	/**
	 * Returns the list of instructions
	 */
	Instructions& getInstructions();

	/**
	 * Returns the list of variables
	 */
	Variables& getVariables();

	/**
	 * Returns the name of the function
	 */
	std::string getFunctionName();

	/**
	 * Returns the labels map
	 */
	std::map<std::string, int>& getLabels();

private:
	/**
	 * Reference to lexical analysis object
	 */
	LexicalAnalysis& lexicalAnalysis;

	/**
	 * Iterator to current token
	 */
	TokenList::iterator currentToken;

	/**
	 * List of instructions
	 */
	Instructions instructions;

	/**
	 * List of variables
	 */
	Variables variables;

	/**
	 * Labels map (label name -> instruction position)
	 */
	std::map<std::string, int> labels;

	/**
	 * Function name
	 */
	std::string functionName;

	/**
	 * Position counter for instructions
	 */
	int instructionPosition;

	/**
	 * Reads next token from token list
	 */
	void readNextToken();

	/**
	 * Checks if current token is of expected type
	 */
	bool checkToken(TokenType expected);

	/**
	 * Tries to eat a token of given type, throws error if not found
	 */
	void eatToken(TokenType expected);

	/**
	 * Finds variable by name in variables list
	 */
	Variable* findVariable(std::string name);

	/**
	 * Parsing methods based on grammar
	 */
	void Q();
	void S();
	void L();
	void E();

	/**
	 * Prints syntax error
	 */
	void printSyntaxError(Token token);
};
