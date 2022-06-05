#include <algorithm>

#include "TableLoader.hpp"

namespace ECE141 {
    
    TableLoader& ECE141::TableLoader::loadTables(SQLQuery& aQuery)
    {
        // If join, load both tables
            // Apply where
        if (aQuery.hasJoin) {
            conditionalLoad(aQuery.join.leftTable, aQuery.whereFilter, leftRows);
            conditionalLoad(aQuery.join.rightTable, aQuery.whereFilter, rightRows);

            applyJoin(aQuery.join.leftValue, aQuery.join.rightValue);
        } 
        else {
            conditionalLoad(aQuery.EntityName, aQuery.whereFilter, outputRows);
        }

        sortRows(aQuery.orderNames[0], outputRows);
        // Apply Limit
        applyLimit(outputRows, aQuery.limitStartIndex, aQuery.limitCount);


        return *this;
    }
    void TableLoader::showOutputs(std::ostream& anOutput)
    {
        // Build Headers

        // Load Rows
    }
    void TableLoader::sortRows(std::string& aName, std::vector<std::shared_ptr<Row>>& aRowsList)
    {
        //TODO: Do Recursion
        std::sort(aRowsList.begin(), aRowsList.end(),
            [&](std::shared_ptr<Row> aFirstRow, std::shared_ptr<Row> aSecondRow) {
                return BasicTypes::toString(aFirstRow->getData()[aName]) <
                    BasicTypes::toString(aSecondRow->getData()[aName]);
            });
    }
    void TableLoader::conditionalLoad(std::string & anEnitityName, Filters & aFilter, std::vector<std::shared_ptr<Row>>&aRowsList)
    {
        size_t theRowStart = storage->getNextPointer(database->getRowIndex(anEnitityName));
        Row theTempRow;

        storage->iterate([&](Block& aBlock, uint64_t anIndex) {
            theTempRow.decode(aBlock);

            //TODO: Wait for update filter interface with scoping
            //if (aFilter.matches(theTempRow.getData())) {
            //    aRowsList.push_back(std::make_unique<Row>(theTempRow));
            //}

            aRowsList.push_back(std::make_unique<Row>(theTempRow));
            return aBlock.header.nextPtr;
            }, theRowStart); 
    }
    void TableLoader::applyJoin(std::string& aLeftCol, std::string& aRightCol)
    {
        // Build table Header
        std::vector<std::string> theTableHeader;
        std::string theEntityName;
        //for (auto& mapIter : leftRows) { // left table, then right
        //    
        //} 

        if (leftRows.size() == 0) { return; }
        // Sort both tables according to join column
        sortRows(aLeftCol, leftRows);
        sortRows(aRightCol, rightRows);

        // Iterate through left table, if right table matches, iterate right, if it stops matching, iterate left
        auto theRightTableIter = rightRows.begin();
        for (auto& theLeftRow : leftRows) {
            if (theRightTableIter != rightRows.end()) {
                //TODO: Fix something
                if (theLeftRow->getData()[aLeftCol] == (*theRightTableIter)->getData()[aRightCol]) {
                    buildJoinedRow(theLeftRow, *theRightTableIter);
                }
                ++theRightTableIter;
            }
            else {}
        }
        
    }
    void TableLoader::applyLimit(std::vector<std::shared_ptr<Row>>& aRowsList, size_t aStartIndex, size_t aLimitCount)
    {
        uint64_t theDisplayCount = aRowsList.size() < aLimitCount ? aRowsList.size() : aLimitCount;

        if (aStartIndex > theDisplayCount) {
            return;
        }


    }
}

