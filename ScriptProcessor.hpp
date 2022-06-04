#ifndef ScriptProcessor_hpp
#define ScriptProcessor_hpp

#include "CmdProcessor.hpp"
#include "Tokenizer.hpp"
#include "Application.hpp"
#include "TokenSequencer.hpp"
#include "Statement.hpp"

namespace ECE141 {
	class ScriptProcessor : public CmdProcessor {
	public:
		ScriptProcessor(std::ostream aStream, Application* anApp) : app{ anApp }, CmdProcessor{ aStream } {
			nextProcessor = nullptr; }

		CmdProcessor* recognizes(Tokenizer& aTokenizer) override;

		Statement* makeStatement(Tokenizer& aTokenizer) override;

		void       run(Statement* aStmt) override;

	protected:

		bool doScriptTest(std::istream& anInput, std::ostream& anOutput);

		Application* app;
	};

	class ScriptStatement : public Statement {
	public:
		ScriptStatement() : Statement{StatementType::script} {}

		static bool recognize(TokenSequencer& aSequencer);

		virtual void parse(TokenSequencer& aSequencer);

		std::string& getScriptName() { return scriptFilename; };

	protected:
		std::string scriptFilename;

		
	};
}

#endif // !ScriptProcessor_hpp
