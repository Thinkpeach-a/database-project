#include "TableView.hpp"
#include "BasicTypes.hpp"

#include <exception>
#include <sstream>

namespace ECE141 {

	TableView::TableView(std::vector<std::string> aHeaderList, std::vector<uint64_t> aWidthList)  {
		columnWidths = aWidthList;

		uint64_t theListSize = aHeaderList.size();
		if (columnWidths.size() != theListSize)
		{
			throw std::length_error("Table Header & Column Widths Mismatch");
		};

		std::stringstream theHeaderStream;


		for (uint64_t i = 0; i < theListSize; ++i)
		{
			insertCol(theHeaderStream, aHeaderList[i], columnWidths[i]);
			totalDataWidth += columnWidths[i];
		}

		//make adjustments for spaces between columns
		totalDataWidth += theListSize - 1; // one extra spacer for each element - the end of the table

		insertLine(dataStream);

		dataStream << TableConsts::COLUMN_END
			<< theHeaderStream.str()
			<< TableConsts::EOL;

		insertLine(dataStream);

	}

	bool TableView::insertRow(std::vector<Value>& aValueVector) {
		dataStream << TableConsts::COLUMN_END;

		uint64_t theArgCount = 0;

		for (auto& theValue : aValueVector) {
			insertCol(dataStream,BasicTypes::toString(theValue), columnWidths[theArgCount]);
			++theArgCount;
		}
		dataStream << TableConsts::EOL;
		insertLine(dataStream);

		dataStream.flush();

		return true;
	}

	bool TableView::insertFooter(std::string aFooterText) {
		dataStream << aFooterText;
		return true;
	}
	bool TableView::show(std::ostream& aStream)
	{
		aStream << dataStream.str();
		return true;
	}
	void TableView::insertLine(std::stringstream &aStream)
	{
		aStream << TableConsts::TABLE_CORNER
			<< std::string(totalDataWidth, TableConsts::HORIZONTAL_LINE)
			<< TableConsts::TABLE_CORNER
			<< TableConsts::EOL;
	}
}
