#ifndef TableView_hpp
#define TableView_hpp

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#include "View.hpp"
#include "BasicTypes.hpp"

namespace ECE141 {
	class TableView : View {
	protected:

		std::stringstream dataStream;
		std::vector<uint64_t> columnWidths;
		uint64_t totalDataWidth = 0;

		struct TableConsts {
			static const char TABLE_CORNER = '+';
			static const char HORIZONTAL_LINE = '-';
			static const char COLUMN_END = '|';
			static const char EOL = '\n';
		};


	public:
		/// <summary>
		/// Generates Table.
		/// </summary>
		/// <param name="aHeaderList"></param>
		/// <param name="aWidthList"></param>
		TableView(std::vector<std::string> aHeaderList = std::vector<std::string>{ "Header" }, std::vector<uint64_t> aWidthList = std::vector<uint64_t>{ 20 }); // With headers/spacing information

		TableView(const TableView& aCopy);
		TableView& operator=(const TableView& aCopy);

		template<typename ... Args>
		bool insertRow(Args ... anArgList);

		bool insertRow(std::vector<Value>& aValueVector);

		bool insertFooter(std::string aFooterText); // End Statement
		bool show(std::ostream& aStream) override; // Takes in streams

		~TableView() {};
	private:

		template <typename T>
		bool isStreamable(T anObject);

		inline void insertLine(std::stringstream& aStream)
		{
			aStream << TableConsts::TABLE_CORNER
				<< std::string(totalDataWidth, TableConsts::HORIZONTAL_LINE)
				<< TableConsts::TABLE_CORNER
				<< TableConsts::EOL;
		}

		template <typename T>
		inline void insertCol(std::stringstream& aStream, T anElement, uint64_t aWidth);
	};


	template<typename ... Args>
	bool TableView::insertRow(Args ... anArgList) {
		uint64_t theArgCount = 0;
		uint64_t theColCount = columnWidths.size();

		dataStream << TableConsts::COLUMN_END;

		([&](auto& anArg)
			{
				if (!isStreamable(anArg) || theArgCount > theColCount) { return; }
				insertCol(dataStream, anArg, columnWidths[theArgCount]);
				++theArgCount;
				return;
			}(anArgList), ...);

		dataStream << TableConsts::EOL;
		insertLine(dataStream);

		dataStream.flush();

		return true;
	} // Add Row to Header

	template <typename T>
	bool TableView::isStreamable(T anObject) {
		return true;
	}

	template <typename T>
	inline void TableView::insertCol(std::stringstream& aStream, T anElement, uint64_t aWidth) {
		aStream << std::setw(aWidth) << std::left << anElement << TableConsts::COLUMN_END;
	}
}

#endif
