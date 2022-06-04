//
//  CmdProcessor.hpp
//  Database
//
//  Created by rick gessner on 3/17/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#ifndef CmdProcessor_hpp
#define CmdProcessor_hpp

#include <memory>

#include "Statement.hpp"
#include "Tokenizer.hpp"

namespace ECE141 {


	class CmdProcessor { //processor interface
	public:

		/// <summary>
		/// Abstract interface for all Processors (Statement Factories), also 
		/// enforces a flexible chain of responsibility pattern. 
		/// </summary>
		/// <param name="anOutput">Stream to pass all outputs to</param>
		CmdProcessor(std::ostream& anOutput);

		// deletes next ptr.
		virtual ~CmdProcessor();

		// Returns self if processor can process tokens,
		// else pass it next processor in chain of command.
		virtual CmdProcessor* recognizes(Tokenizer& aTokenizer) = 0;
		
		// Creates Statements for recognized commands.
		virtual Statement* makeStatement(Tokenizer& aTokenizer) = 0;

		virtual void run(Statement* aStatement) = 0;

		std::ostream& output;

	protected:

		CmdProcessor* nextProcessor;
	};
}

#endif /* CmdProcessor */
