#include "KeyValues.h"

#include <utility>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <functional>

#include <Windows.h>

KeyValues::KeyValues()
{
}

KeyValues::KeyValues(const std::string &name)
{
	m_Name = name;
}

KeyValues::KeyValues(KeyValues *parent)
{
	m_Parent = parent;
	m_Parent->m_Childs.push_back(this);
}

KeyValues::KeyValues(const std::string &name, const std::string &value)
{
	m_Name = name;
	m_Value = value;
	m_isValue = true;
}

KeyValues::~KeyValues()
{
	for (auto &child : m_Childs)
		delete child;
}

void KeyValues::LoadFromFile(const std::string &filepath)
{
	std::ifstream in(filepath);

	std::string contents;
	in.seekg(0, std::ios::end);
	contents.resize(in.tellg());
	in.seekg(0, std::ios::beg);
	in.read(&contents[0], contents.size());
	in.close();

	std::stringstream ss(contents);
	std::string line;
	size_t lineCount = 0;
	KeyValues *current = this;
	current->m_Parent = nullptr;

	while (std::getline(ss, line, '\n'))
	{
		// Skip whitespace
		line.erase(line.begin(), std::find_if_not(line.begin(), line.end(), [](char c){ return std::isspace(c); }));
		line.erase(std::find_if_not(line.rbegin(), line.rend(), [](char c){ return std::isspace(c); }).base(), line.end());

		// Skip comments
		if (!strncmp(line.c_str(), "/", strlen("/")))
			continue;

		if (!strncmp(line.c_str(), "#include", strlen("#include"))
			|| !strncmp(line.c_str(), "#base", strlen("#base")))
		{
			size_t nameBegin = line.find_first_of('"');
			current->LoadFromFile(line.substr(nameBegin, line.length() - nameBegin - 1));
		}

		// Either KeyValues or Value
		if (*line.c_str() == '"')
		{
			// New KeyValues entry
			if (std::count(line.begin(), line.end(), '"') == 2)
			{
				KeyValues *child = new KeyValues(current);
				child->m_Name = line.substr(1, line.length() - 2);
				current = child;
			}
			else if (std::count(line.begin(), line.end(), '"') == 4)
			{
				KeyValues *key = new KeyValues(current);
				key->m_Name = line.substr(1, line.find_first_of('"', 1) - 1);

				size_t secondBegin = line.find_first_of('"', key->m_Name.length() + 2) + 1;
				size_t secondLength = line.size() - secondBegin - 1;

				key->m_isValue = true;
				key->m_Value = line.substr(secondBegin, secondLength);
			}
		}
		else if (*line.c_str() == '{');
		else if (*line.c_str() == '}')
			current = current->GetParent();
	}
}

void KeyValues::SaveToFile(const std::string &filename)
{
	FILE *file = fopen(filename.c_str(), "wt");
	
	if (file)
	{
		int level = 0;
		std::function<void (KeyValues *)> writeKey = [&](KeyValues *key)
		{
			for (size_t i = 0; i < level; ++i)
				fprintf(file, "\t");

			if (key->m_isValue)
			{
				fprintf(file, "\"%s\"\t\"%s\"\n", key->m_Name.c_str(), key->m_Value.c_str());
			}
			else
			{
				fprintf(file, "\"%s\"\n", key->m_Name.c_str());

				for (size_t i = 0; i < level; ++i)
					fprintf(file, "\t");

				fprintf(file, "{\n");
				
				++level;
				for (auto &subkey : key->m_Childs)
					writeKey(subkey);
				--level;
				
				for (size_t i = 0; i < level; ++i)
					fprintf(file, "\t");

				fprintf(file, "}\n");
			}
		};

		for (auto &key : m_Childs)
		{
			writeKey(key);
		}

		fclose(file);
	}
}

bool KeyValues::InsertChildren(int position, int count)
{
	for (int i = 0; i < count; ++i)
	{
		KeyValues *newVal = new KeyValues();
		newVal->m_Name = "[no data]";
		newVal->m_isValue = false;
		newVal->m_Value = "[no data]";
		newVal->m_Parent = this;
	
		m_Childs.insert(m_Childs.begin() + position, 1, newVal);
	}

	return true;
}

bool KeyValues::RemoveChildren(int position, int count)
{
	// Array bounds checking
	if (position < 0 || position + count > m_Childs.size())
		return false;

	// First delete the elements to avoid memory leaks
	for (int i = 0; i < count; ++i)
		delete m_Childs[i + position];

	// Then remove them from the list
	m_Childs.erase(m_Childs.begin() + position, m_Childs.begin() + position + count);
	return true;
}