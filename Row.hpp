//
//  Row.hpp
//  PA3
//
//  Created by rick gessner on 4/2/22.
//

#ifndef Row_hpp
#define Row_hpp

#include <stdio.h>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <memory>
#include "Storage.hpp"
#include "BasicTypes.hpp"
#include "Attribute.hpp"

//feel free to use this, or create your own version...

namespace ECE141 {
	
	class Row : public Storable {
	public:

		Row(uint64_t anEntityID = 0);
		Row(const Row& aRow);

		// Row(const Attribute &anAttribute); //maybe?

		~Row();

		Row& operator=(const Row& aRow);
		bool operator==(Row& aCopy) const;

		//STUDENT: What other methods do you require?

		Row& set(const std::string& aKey,
			const Value& aValue);

		KeyValues& getData() { return data; }

		uint64_t            entityId; //hash value of entity?
		//uint32_t            blockNumber;
		void encode(Block &aBlock) override;
		void decode(Block &aBlock) override;
	protected:
		KeyValues          data;
	};

	//-------------------------------------------

	using RowCollection = std::vector<std::shared_ptr<Row> >;
}
#endif /* Row_hpp */
