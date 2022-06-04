#include <fstream>

#include "ScriptProcessor.hpp"
#include "Helpers.hpp"


namespace ECE141 {

	CmdProcessor* ECE141::ScriptProcessor::recognizes(Tokenizer& aTokenizer)
	{
		TokenSequencer theSeq(aTokenizer);
		if (ScriptStatement::recognize(theSeq)) {
			//Reset tokenizer.
			theSeq.reset();
			return this;
		}
		theSeq.reset();
		return nullptr;
	}

	Statement* ScriptProcessor::makeStatement(Tokenizer& aTokenizer)
	{
		TokenSequencer theSeq(aTokenizer);
		Statement* theStatement = new ScriptStatement();

		theStatement->parse(theSeq);

		if (theStatement) {
			return theStatement;
		}
		return nullptr;
	}

	void ScriptProcessor::run(Statement* aStmt)
	{
		// Only 1 statement gets routed here
		ScriptStatement* theStatement = dynamic_cast<ScriptStatement*>(aStmt);
		std::string theScriptName = Config::getScriptFilepath(theStatement->getScriptName());

		if (file_exists(theScriptName)) {
			std::ifstream theStream(theScriptName);
			
			if (theStream) {
				doScriptTest(theStream, output);
			}
		}
	}

	bool ScriptProcessor::doScriptTest(std::istream& anInput, std::ostream& anOutput) {
		ECE141::Application   theApp(anOutput);
		ECE141::StatusResult  theResult{};
		std::string theCommand;

		while (theResult && anInput) {
			std::getline(anInput, theCommand);
			std::stringstream theStream(theCommand);
			anOutput << theCommand << "\n";
			theResult = theApp.handleInput(theStream);
			if (theResult == ECE141::userTerminated) {
				theResult.error = Errors::noError;
				break;
			}
			else if (!theResult) {
				anOutput << "Error (" << theResult.error << ") " << "\n";
			}
		}
		return theResult;
	}


	bool ScriptStatement::recognize(TokenSequencer& aSequencer)
	{
		return aSequencer.hasData("run");
	}

	void ScriptStatement::parse(TokenSequencer& aSequencer)
	{
		if (!aSequencer.isIdentifier().getData(scriptFilename)) {
			throw Errors::syntaxError;
		}
	}

}


