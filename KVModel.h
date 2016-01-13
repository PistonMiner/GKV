#pragma once

#include "KeyValues.h"
#include <QAbstractItemModel>

class KVModel : public QAbstractItemModel
{
	Q_OBJECT
private:
	KeyValues *rootItem;

public:
	KVModel(QObject *parent = nullptr);
	~KVModel() { delete rootItem; }

	void MountValues(KeyValues *val)
	{
		this->beginResetModel();
		rootItem->m_Childs.push_back(val);
		val->m_Parent = rootItem;
		this->endResetModel();
	}
	KeyValues *getItem(const QModelIndex &index) const;

	void ManualWriteBegin(const QModelIndex &index)
	{
		this->beginInsertRows(index, 0, getItem(index)->GetChildCount());
	}

	void ManualWriteEnd() { this->endInsertRows(); }

	// --- QAbstractItemModel overrides ---
	QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

	QModelIndex index(int row, int column,
		const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;

	int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

	Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
	bool setData(const QModelIndex &index, const QVariant &value,
		int role = Qt::EditRole) Q_DECL_OVERRIDE;
	bool setHeaderData(int section, Qt::Orientation orientation,
		const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

	bool insertColumns(int position, int columns,
		const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
	bool removeColumns(int position, int columns,
		const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
	bool insertRows(int position, int rows,
		const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
	bool removeRows(int position, int rows,
		const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
	// ------------------------------------
};