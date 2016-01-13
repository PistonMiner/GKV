#pragma once

#include <deque>

class KeyValues
{
	friend class KVModel;
protected:
	std::string m_Name;

	// if this is true, then it's a value and not a subkey and can not have children.
	bool m_isValue = false;
	std::string m_Value;
	std::deque<KeyValues *> m_Childs;

	KeyValues *m_Parent;

public:
	KeyValues();
	KeyValues(KeyValues *parent);
	KeyValues(const std::string &name);
	KeyValues(const std::string &name, const std::string &value);
	~KeyValues();

	void LoadFromFile(const std::string &filename);
	void SaveToFile(const std::string &filename);

	KeyValues *GetParent() { return m_Parent; }
	int GetChildCount() const { return m_Childs.size(); }
	KeyValues *GetChildAt(int row) { return row < m_Childs.size() ? m_Childs[row] : nullptr; };

	bool InsertChildren(int position, int count);
	bool RemoveChildren(int position, int count);

	bool IsValue() { return m_isValue; }

	std::string GetData(bool isVal) const
	{
		if (!m_isValue && isVal)
			return "";
		return isVal ? m_Value : m_Name;
	}
	bool SetData(bool isVal, const std::string &data)
	{
		if (!m_isValue && isVal)
			return false;
		else if (isVal)
			m_Value = data;
		else
			m_Name = data;

		return true;
	}

	int FindChild(KeyValues *child)
	{
		int i = 0;
		for (auto &element : m_Childs)
		{
			if (element == child)
				return i;
			++i;
		}
		return -1;
	}

	KeyValues *FindChildByName(const std::string &name)
	{
		KeyValues *sub = this->GetChildAt(0);
		for (int i = 0; i < this->GetChildCount(); sub = this->GetChildAt(++i))
		{
			if (sub->GetData(false) == name)
				return sub;
		}

		return (KeyValues *) nullptr;
	}

	void Remove()
	{
		m_Parent->RemoveChildren(m_Parent->FindChild(this), 1);
	}

	void SetMode(bool mode) { m_isValue = mode; }

	bool KeyValues::InsertChildren(int position, KeyValues *key)
	{
		key->m_Parent = this;
		m_Childs.insert(m_Childs.begin() + position, 1, key);

		return true;
	}

	KeyValues *Copy()
	{
		KeyValues *copy = new KeyValues(m_Name, m_Value);
		copy->m_isValue = this->m_isValue;

		for (auto& c : this->m_Childs)
		{
			copy->InsertChildren(copy->GetChildCount(), c->Copy());
		}

		return copy;
	}
};