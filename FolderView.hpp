//
//  FolderView.hpp
//  Assignment2
//
//  Created by rick gessner on 2/15/21.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef FolderView_h
#define FolderView_h

#include "FolderReader.hpp"
#include "TableView.hpp"
#include <iomanip>
#include <vector>

namespace ECE141 {

	// USE: provide view class that lists db files in storage path...
	class FolderView{
	public: 
		/// <summary>
		/// Constructs a view to display all files of a specific type as a stream.
		/// </summary>
		/// <param name="aPath"></param>
		/// <param name="anExtension"></param>
		FolderView(std::string aPath, std::string anExtension = ".db")
			: reader(aPath), extension(anExtension){}

		/// <summary>
		/// Generates view of file list.
		/// </summary>
		/// <param name="anOutput"></param>
		/// <returns></returns>
		virtual bool show(std::ostream& anOutput) {
			// construct table
			reader.each(extension, [&](const std::string aFilename) {
				uint64_t theFilenameLen = aFilename.length();
				if (theFilenameLen > maxColWidth) { // Update table width
					maxColWidth = theFilenameLen;
				}
				files.push_back(fs::path(aFilename).filename().string()); // Store filename as well
				
				return true;
				});

			std::vector<std::string> theHeader = { "Files" };
			std::vector<uint64_t> theWidth = { maxColWidth };

			TableView theTable(theHeader, theWidth);

			uint64_t theFileCount = files.size();
			for (uint64_t i = 0; i < theFileCount; ++i) {
				theTable.insertRow(files[i]);
			}
			
			theTable.show(anOutput);


			return true;
		}

		/// <summary>
		/// Gets width of longest file name for table widths.
		/// </summary>
		/// <param name="aFilename"></param>
		/// <returns></returns>
		virtual bool getMaxFilenameWidth(const std::string& aFilename) {
			uint64_t theFilenameLen = aFilename.length();
			if (theFilenameLen > maxColWidth) {
				maxColWidth = theFilenameLen;
			}
			return false;
		}
		// Set a min column size
		uint64_t maxColWidth = 15;

		TableView table;

		std::vector<std::string> files;

		FolderReader reader;
		std::string extension;

	};

}

#endif /* FolderView_h */
