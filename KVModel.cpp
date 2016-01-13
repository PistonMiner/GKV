#include <QtWidgets>

#include "KeyValues.h"
#include "KVModel.h"

KVModel::KVModel(QObject *parent /*= nullptr*/) : QAbstractItemModel(parent)
{
	rootItem = new KeyValues("Key", "Value");
}

int KVModel::columnCount(const QModelIndex & /* parent */) const
{
	return 2;
}

QVariant KVModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role == Qt::ToolTipRole)
	{
		return index.child(0, 1).data(Qt::DisplayRole);
	}

	if (role != Qt::DisplayRole && role != Qt::EditRole)
		return QVariant();

	KeyValues *item = getItem(index);

	return QString::fromStdString(item->GetData(!!index.column()));
}

Qt::ItemFlags KVModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;
	
	if (!getItem(index)->IsValue() && index.column() > 0)
		return QAbstractItemModel::flags(index);

	return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

KeyValues *KVModel::getItem(const QModelIndex &index) const
{
	if (index.isValid()) {
		KeyValues *item = static_cast<KeyValues*>(index.internalPointer());
		if (item)
			return item;
	}
	return rootItem;
}

QVariant KVModel::headerData(int section, Qt::Orientation orientation,
	int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return QString::fromStdString(rootItem->GetData(!!section));

	return QVariant();
}

QModelIndex KVModel::index(int row, int column, const QModelIndex &parent) const
{
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();

	KeyValues *parentItem = getItem(parent);

	KeyValues *childItem = parentItem->GetChildAt(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

bool KVModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
	return false;
}

bool KVModel::insertRows(int position, int rows, const QModelIndex &parent)
{
	KeyValues *parentItem = getItem(parent);
	bool success;

	beginInsertRows(parent, position, position + rows - 1);
	success = parentItem->InsertChildren(position, rows);
	endInsertRows();

	return success;
}

QModelIndex KVModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	KeyValues *childItem = getItem(index);
	KeyValues *parentItem = childItem->GetParent();

	if (parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->GetParent()->FindChild(parentItem), 0, parentItem);
}

bool KVModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
	return false;
}

bool KVModel::removeRows(int position, int rows, const QModelIndex &parent)
{
	KeyValues *parentItem = getItem(parent);
	bool success = true;

	beginRemoveRows(parent, position, position + rows - 1);
	success = parentItem->RemoveChildren(position, rows);
	endRemoveRows();

	return success;
}

int KVModel::rowCount(const QModelIndex &parent) const
{
	KeyValues *parentItem = getItem(parent);

	return parentItem->GetChildCount();
}

bool KVModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (role != Qt::EditRole)
		return false;

	KeyValues *item = getItem(index);
	bool result = item->SetData(!!index.column(), value.toString().toStdString());

	if (result)
		emit dataChanged(index, index);

	return result;
}

bool KVModel::setHeaderData(int section, Qt::Orientation orientation,
	const QVariant &value, int role)
{
	if (role != Qt::EditRole || orientation != Qt::Horizontal)
		return false;

	bool result = rootItem->SetData(!!section, value.toString().toStdString());

	if (result)
		emit headerDataChanged(orientation, section, section);

	return result;
}