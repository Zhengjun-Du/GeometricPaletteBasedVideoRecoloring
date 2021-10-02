#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

template<typename T> std::ostream&  operator<< (std::ostream& os, const std::vector<T>& vec) {
	os << "[";
	for (int i = 0; i < (int)vec.size(); i++)
	{
		if (i != 0)
			os << ",";
		os << vec[i];
	}
	os << "]";
	return os;
}

template<typename T> T fromString(const std::string& str) {
	T t;
	std::stringstream ss(str);
	ss >> t;
	return t;
}

namespace my_util {

	template<typename T> T clamp(const T&val, const T&val_min, const T&val_max)
	{
		T tmp = std::min(val, val_max);
		tmp = std::max(tmp, val_min);
		return tmp;
	}

	/*
		e.g.: 
		directory: "." => "./", 
		directory: "./" => "./"
	*/
	inline std::string addSlashToEnd(const std::string& directory) {
		if (directory.length() == 0)
			return directory;
		int last_pos = directory.length() - 1;
		if (directory[last_pos] == '\\' || directory[last_pos] == '/')
			return directory;
		else if (directory.find('\\') != -1)
			return directory + "\\";
		else
			return directory + "/";
	}


	/*
		e.g: fullpath: "D:\\test\\aaa.png" => "aaa.png",
	*/
	inline std::string getFilenameWithExt(const std::string& fullpath) {
		int pos2 = fullpath.find_last_of('\\');
		int pos3 = fullpath.find_last_of('/');
		int pos4 = std::max(pos2, pos3);
		if (pos4 == -1) {
			return fullpath;
		}
		else {
			return fullpath.substr(pos4 + 1);
		}
	}

	/*
		e.g: fullpath: "D:\\test\\aaa.png" => "aaa",
	*/
	inline std::string getFilename(const std::string& fullpath) {
		std::string filename;

		std::string fnWithExt = getFilenameWithExt(fullpath);

		//remove extension
		int pos = fnWithExt.find_last_of('.');
		if (pos == -1) {
			filename = fnWithExt;
		}
		else {
			filename = fnWithExt.substr(0, pos);
		}

		return filename;
	}
}
