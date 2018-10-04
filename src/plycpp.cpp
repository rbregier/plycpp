// MIT License
// 
// Copyright(c) 2018 Romain Brégier
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <plycpp.h>

#include <fstream>
#include <sstream>
#include <map>
#include <iostream>
#include <cassert>
#include <algorithm>

namespace plycpp
{
	bool isBigEndianArchitecture()
	{
		union {
			uint32_t i;
			char c[4];
		} myunion = { 0x01020304 };

		return myunion.c[0] == 1;
	}


	const std::map<DataType, int> dataTypeByteSize{
		{ CHAR, 1 },
		{ UCHAR, 1 },
		{ SHORT, 2 },
		{ USHORT, 2 },
		{ INT, 4 },
		{ UINT, 4 },
		{ FLOAT, 4 },
		{ DOUBLE, 8 }
	};


	const std::map<std::string, DataType> strToDataType{
		{ "char", CHAR },
		{ "uchar", UCHAR },
		{ "unsigned char", UCHAR },
		{ "short", SHORT },
		{ "ushort", USHORT },
		{ "unsigned short", USHORT },
		{ "int", INT },
		{ "int32", INT },
		{ "uint", UINT },
		{ "unsigned int", UINT },
		{ "uint32", UINT },
		{ "float", FLOAT },
		{ "float32", FLOAT },
		{ "double", DOUBLE },
		{ "float64", DOUBLE }
	};

	const std::map<DataType, std::string> dataTypeToStr{
		{ CHAR, "char" },
		{ UCHAR, "uchar" },
		{ SHORT, "short" },
		{ USHORT, "ushort" },
		{ INT, "int" },
		{ UINT, "uint" },
		{ FLOAT, "float" },
		{ DOUBLE, "double" },
	};

	DataType parseDataType(const std::string& name)
	{
		const auto& it = strToDataType.find(name);
		if (it != strToDataType.end())
		{
			return it->second;
		}
		else
			throw ParsingException(std::string("Unkown data type:" + name));
	}

	std::string dataTypeToString(const DataType type)
	{
		return (dataTypeToStr.find(type))->second;
	}

	PropertyArray::PropertyArray(const DataType type, const size_t size, const bool isList)
	{
		this->type = type;
		this->stepSize = (dataTypeByteSize.find(type))->second;
		this->data.resize(size * this->stepSize);
		this->isList_ = isList;
	}


	void splitString(const std::string& input, std::vector<std::string>& result)
	{
		result.clear();
		std::stringstream ss(input);
		while (true)
		{
			std::string elem;
			ss >> elem;
			if (ss.fail())
				break;
			result.push_back(elem);
		}

	}

	size_t strtol_except(const std::string& in)
	{
		std::stringstream ss(in);
		size_t val;
		ss >> val;
		if (ss.fail())
			throw ParsingException("Invalid unsigned integer");
		return val;
	}


	inline void readASCIIValue(std::ifstream& fin, unsigned char* const  ptData, const DataType type)
	{
		int temp;
		switch (type)
		{
		case CHAR:
			fin >> temp;
			*reinterpret_cast<char*>(ptData) = static_cast<char>(temp);
			break;
		case UCHAR:
			fin >> temp;
			*reinterpret_cast<unsigned char*>(ptData) = static_cast<unsigned char>(temp);
			break;
		case SHORT:
			fin >> *reinterpret_cast<int16_t*>(ptData);
			break;
		case USHORT:
			fin >> *reinterpret_cast<uint16_t*>(ptData);
			break;
		case INT:
			fin >> *reinterpret_cast<int32_t*>(ptData);
			break;
		case UINT:
			fin >> *reinterpret_cast<uint32_t*>(ptData);
			break;
		case FLOAT:
			fin >> *reinterpret_cast<float*>(ptData);
			break;
		case DOUBLE:
			fin >> *reinterpret_cast<double*>(ptData);
			break;
		}
	}

	inline void writeASCIIValue(std::ofstream& fout, unsigned char* const  ptData, const DataType type)
	{
		switch (type)
		{
		case CHAR:
			fout << int(*reinterpret_cast<char*>(ptData));
			break;
		case UCHAR:
			fout << int(*reinterpret_cast<unsigned char*>(ptData));
			break;
		case SHORT:
			fout << *reinterpret_cast<int16_t*>(ptData);
			break;
		case USHORT:
			fout << *reinterpret_cast<uint16_t*>(ptData);
			break;
		case INT:
			fout << *reinterpret_cast<int32_t*>(ptData);
			break;
		case UINT:
			fout << *reinterpret_cast<uint32_t*>(ptData);
			break;
		case FLOAT:
			fout << *reinterpret_cast<float*>(ptData);
			break;
		case DOUBLE:
			fout << *reinterpret_cast<double*>(ptData);
			break;
		}
	}

	template <FileFormat format>
	void readDataContent(std::ifstream& fin, PLYData& data)
	{
		/// Store a pointer to the current place where to write next data for each property of each element
		std::map<std::shared_ptr<PropertyArray>, unsigned char*> writingPlace;
		for (auto& elementTuple : data)
		{
			auto& element = elementTuple.data;
			for (auto& propertyTuple : element->properties)
			{
				auto& prop = propertyTuple.data;
				writingPlace[prop] = prop->data.data();
			}
		}

		//// Iterate over elements array
		for (auto& elementArrayTuple : data)
		{
			auto& elementArray = elementArrayTuple.data;
			const size_t elementsCount = elementArray->size();
			// Iterate over elements
			for (size_t i = 0; i < elementsCount; ++i)
			{
				// Iterate over properties of the element
				for (auto& propertyTuple : elementArray->properties)
				{
					auto& prop = propertyTuple.data;

					if (!prop->isList())
					{
						// Read data
						auto& ptData = writingPlace[prop];
						// Safety check
						assert(ptData >= prop->data.data());
						assert(ptData + prop->stepSize <= prop->data.data() + prop->data.size());

						if (format == ASCII)
						{
							readASCIIValue(fin, ptData, prop->type);

						}
						else
						{
							fin.read(reinterpret_cast<char*>(ptData), prop->stepSize);
						}
						// Increment
						ptData += prop->stepSize;
					}
					else
					{
						// Read count
						unsigned char count;

						if (format == ASCII)
						{
							int temp;
							fin >> temp;
							count = static_cast<unsigned char>(temp);
						}
						else
						{
							fin.read(reinterpret_cast<char*>(&count), sizeof(unsigned char));
						}
						if (fin.fail() || count != 3)
						{
							throw ParsingException("Only lists of 3 values are supported");
						}

						// Read data
						auto& ptData = writingPlace[prop];
						const size_t chunkSize = 3 * prop->stepSize;

						// Safety check
						assert(ptData >= prop->data.data());
						assert(ptData + chunkSize <= prop->data.data() + prop->data.size());

						if (format == ASCII)
						{
							readASCIIValue(fin, ptData, prop->type);
							ptData += prop->stepSize;
							readASCIIValue(fin, ptData, prop->type);
							ptData += prop->stepSize;
							readASCIIValue(fin, ptData, prop->type);
							ptData += prop->stepSize;
						}
						else
						{
							fin.read(reinterpret_cast<char*>(ptData), chunkSize);
							ptData += chunkSize;
						}
					}
				}
			}
		}
	}

	void myGetline(std::ifstream& fin, std::string& line)
	{
		std::getline(fin, line);
		// Files created with Windows have a carriage return
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
	}

	void load(const std::string& filename, PLYData& data)
	{
		// Read header and reserve memory
		data.clear();
		std::string format;
		std::string version;

		std::ifstream fin(filename, std::ios::binary);
		fin.sync_with_stdio(false);

		if (!fin.is_open())
			throw ParsingException(std::string("Unable to open ") + filename);

		std::string line;
		myGetline(fin, line);

		std::shared_ptr<ElementArray> currentElement = nullptr;

		if (line != "ply")
		{
			throw ParsingException("Missing magic number ""ply""");
		}

		while (line != "end_header")
		{
			myGetline(fin, line);
			if (fin.fail())
				throw ParsingException("Header parsing exception");

			std::vector<std::string> lineContent;
			splitString(line, lineContent);

			if (lineContent.size() == 3 && lineContent[0] == "format")
			{
				format = lineContent[1];
				version = lineContent[2];
			}
			if (lineContent.size() == 3 && lineContent[0] == "element")
			{
				// New element
				const std::string& name = lineContent[1];
				const size_t count = strtol_except(lineContent[2]);

				currentElement.reset(new ElementArray(count));

				data.push_back(name, currentElement);
			}
			else if (lineContent.size() == 3 && lineContent[0] == "property")
			{
				if (!currentElement)
					throw ParsingException("Header issue!");

				// New property
				const DataType dataType = parseDataType(lineContent[1]);
				const std::string& name = lineContent[2];

				std::shared_ptr<PropertyArray> newProperty(new PropertyArray(dataType, currentElement->size()));
				currentElement->properties.push_back(name, newProperty);
			}
			else if (lineContent.size() == 5 && lineContent[0] == "property" && lineContent[1] == "list")
			{
				if (!currentElement)
					throw ParsingException("Header issue!");

				const DataType indexCountType = parseDataType(lineContent[2]);
				const DataType dataType = parseDataType(lineContent[3]);
				const std::string& name = lineContent[4];

				if (indexCountType != UCHAR)
					throw ParsingException("Only uchar is supported as counting type for lists");

				std::shared_ptr<PropertyArray> newProperty(new PropertyArray(dataType, 3 * currentElement->size(), true));
				currentElement->properties.push_back(name, newProperty);
			}

		}

		if (fin.fail())
		{
			throw ParsingException("Issue while parsing header");
		}


		/////////////////////////////////////
		// Read data
		if (format == "ascii")
		{
			readDataContent<FileFormat::ASCII>(fin, data);

			if (fin.fail())
			{
				throw ParsingException("Issue while parsing ascii data");
			}
		}
		else
		{
			const bool isBigEndianArchitecture_ = isBigEndianArchitecture();
			if (format != "binary_little_endian" && format != "binary_big_endian")
				throw ParsingException("Unknown binary format");


			if ((isBigEndianArchitecture_ && format != "binary_big_endian")
				|| (!isBigEndianArchitecture_ && format != "binary_little_endian"))
				throw ParsingException("Endianness conversion is not supported yet");
			
			readDataContent<FileFormat::BINARY>(fin, data);

			if (fin.fail())
			{
				throw ParsingException("Issue while parsing binary data");
			}

			// Ensure we reached the end of file by trying to read a last char
			char useless;
			fin.read(&useless, 1);
			if (!fin.eof())
			{
				throw ParsingException("End of file not reached at the end of parsing.");
			}
		}
	}


	template<FileFormat format> 
	void writeDataContent(std::ofstream& fout, const PLYData& data)
	{
		/// Store a pointer to the current place from which to read next data for each property of each element
		std::map<std::shared_ptr<PropertyArray>, unsigned char*> readingPlace;
		for (auto& elementTuple : data)
		{
			auto& element = elementTuple.data;
			for (auto& propertyTuple : element->properties)
			{
				auto& prop = propertyTuple.data;
				readingPlace[prop] = prop->data.data();
			}
		}

		//// Iterate over elements array
		for (auto& elementArrayTuple : data)
		{
			auto& elementArray = elementArrayTuple.data;
			const size_t elementsCount = elementArray->size();
			// Iterate over elements
			for (size_t i = 0; i < elementsCount; ++i)
			{
				// Iterate over properties of the element
				for (auto& propertyTuple : elementArray->properties)
				{
					auto& prop = propertyTuple.data;
					// Write data
					auto& ptData = readingPlace[prop];
					if (!prop->isList())
					{
						// Safety check
						assert(ptData >= prop->data.data());
						assert(ptData + prop->stepSize <= prop->data.data() + prop->data.size());
						if (format == FileFormat::BINARY)
							fout.write(reinterpret_cast<const char*>(ptData), prop->stepSize);
						else
						{
							writeASCIIValue(fout, ptData, prop->type);
							fout << " ";
						}
						ptData += prop->stepSize;
					}
					else
					{
						if (format == FileFormat::BINARY)
						{
							const unsigned char count = 3;
							// Write the number of elements
							fout.write(reinterpret_cast<const char*>(&count), sizeof(unsigned char));
							// Write data
							const size_t chunckSize = 3 * prop->stepSize;
							// Safety check
							assert(ptData >= prop->data.data());
							assert(ptData + chunckSize <= prop->data.data() + prop->data.size());
							fout.write(reinterpret_cast<const char*>(ptData), chunckSize);
							ptData += chunckSize;
						}
						else
						{
							fout << "3 ";
							writeASCIIValue(fout, ptData, prop->type);
							fout << " ";
							ptData += prop->stepSize;
							writeASCIIValue(fout, ptData, prop->type);
							fout << " ";
							ptData += prop->stepSize;
							writeASCIIValue(fout, ptData, prop->type);
							fout << " ";
							ptData += prop->stepSize;
						}

					}


				}
				if (format == FileFormat::ASCII)
				{
					fout << "\n";
				}
			}
		}
	}

	void save(const std::string& filename, const PLYData& data, const FileFormat format)
	{
		std::ofstream fout(filename, std::ios::binary);

		// Write header
		fout << "ply\n";
		switch (format)
		{
		case FileFormat::ASCII:
			fout << "format ascii 1.0\n";
			break;
		case FileFormat::BINARY:
			if (isBigEndianArchitecture())
				fout << "format binary_big_endian 1.0\n";
			else
				fout << "format binary_little_endian 1.0\n";
			break;
		default:
			throw ParsingException("Unknown file format. Should not happen.");
			break;
		}

		// Iterate over elements array
		for (const auto& elementArrayTuple : data)
		{
			const auto& elementArrayName = elementArrayTuple.key;
			auto& elementArray = elementArrayTuple.data;
			const size_t elementsCount = elementArray->size();

			fout << "element " << elementArrayName << " " << elementsCount << std::endl;
			// Iterate over properties
			for (const auto& propertyTuple : elementArray->properties)
			{
				auto& propName = propertyTuple.key;
				auto& prop = propertyTuple.data;

				if (!prop)
					throw ParsingException("Null property " + elementArrayName + " -- " + propName);

				// String name of the property type
				const auto& itTypeName = dataTypeToStr.find(prop->type);
				if (itTypeName == dataTypeToStr.end())
					throw ParsingException("Should not happen");

				if (!prop->isList())
				{
					if (prop->data.size() != elementsCount * prop->stepSize)
					{
						throw ParsingException("Inconsistent size for " + elementArrayName + " -- " + propName);
					}

					fout << "property " << itTypeName->second << " " << propName << std::endl;
				}
				else
				{
					if (prop->data.size() != 3 * elementsCount * prop->stepSize)
					{
						throw ParsingException("Inconsistent size for list " + elementArrayName + " -- " + propName);
					}

					fout << "property list uchar " << itTypeName->second << " " << propName << std::endl;
				}

			}
		}
		fout << "end_header" << std::endl;

		// Write data
		switch (format)
		{
		case FileFormat::BINARY:
			writeDataContent<FileFormat::BINARY>(fout, data);
			break;
		case FileFormat::ASCII:
			writeDataContent<FileFormat::ASCII>(fout, data);
			break;
		default:
			throw ParsingException("Unknown file format. Should not happen.");
			break;
		}
		

		if (fout.fail())
		{
			throw ParsingException("Problem while writing binary data");
		}
	}
}