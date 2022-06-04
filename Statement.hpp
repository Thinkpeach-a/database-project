//
//  Statement.hpp
//  Database
//
//  Created by rick gessner on 3/20/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#ifndef Statement_hpp
#define Statement_hpp

#include "keywords.hpp"
#include "TokenSequencer.hpp"
#include <iostream>
#include <vector>

namespace ECE141 {

	class Tokenizer;
	
	/// <summary>
	/// All statement types
	/// </summary>
	enum class StatementType {
		get_version, show_help, quit, show_dbs, create_db, drop_db, use_db, dump_db,
		create_tb, drop_tb, describe_tb, show_tbs, unknown_type,
        insert_row, select, delete_rws, update_rws, script, show_idx, show_idxs
	};

	class Statement {
	public:
		/// <summary>
		/// Default statement interface.
		/// </summary>
		/// <param name="aType"></param>
		Statement(StatementType aType = StatementType::unknown_type) : type{ aType } {}

		/// <summary>
		/// Uses TokenSequencer to generate statement.
		/// </summary>
		/// <param name="aTokenizer"></param>
		/// <returns></returns>
		virtual void parse(TokenSequencer& aTokenizer) { throw Errors::unknownCommand; }

		StatementType getType() { return type; };

	protected:
		StatementType type;
	};


}

#endif /* Statement_hpp */
