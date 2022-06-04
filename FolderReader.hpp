//
//  FolderReader.hpp
//  Database5
//
//  Created by rick gessner on 4/4/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef FolderReader_h
#define FolderReader_h

#include <string>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace ECE141 {

	using FileVisitor = std::function<bool(const std::string&)>;

		class FolderReader {
		public:
			/// <summary>
			/// Constructs object to read folder contents.
			/// </summary>
			/// <param name="aPath"></param>
			FolderReader(std::string aPath) : path(aPath) {}
			virtual ~FolderReader() {}

			virtual bool exists(const std::string& aFilename) {
				std::ifstream theStream(aFilename);
				return !theStream ? false : true;
			}

			/// <summary>
			/// Iterates though files in list to display. Can be modified to behave
			/// differently based on return type of aVisitor.
			/// </summary>
			/// <param name="anExt">fileExtention</param>
			/// <param name="aVisitor">Function that takes in a filename, returns bool</param>
			virtual void each(const std::string& anExt,
				const FileVisitor& aVisitor) const {
				for (const auto& file : fs::directory_iterator(path)) {
					if (file.path().extension() == anExt) {
						aVisitor(file.path().string());
					}
				}
			};

			std::string path;
	};

}

#endif /* FolderReader_h */
