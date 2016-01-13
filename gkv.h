#ifndef GKV_H
#define GKV_H

#include <QtWidgets/QMainWindow>
#include "ui_gkv.h"
#include "ui_attribs.h"

#include "KVModel.h"
#include "KeyValues.h"

class GKV : public QMainWindow
{
	Q_OBJECT

public:
	GKV(QWidget *parent = 0);
	~GKV();

public slots:
	void onCustomContextMenu(const QPoint &point);

	void onQuit();
	void onMountValues();
	void onNew();
	void onAddAttrib();

private:
	Ui::GKVClass ui;

	KVModel *m_Model;
	std::vector<KeyValues *> m_AttributeTable;
};

Q_DECLARE_METATYPE(KeyValues *);

class AttribDialogIntern : public QDialog
{
	Q_OBJECT

	Ui::AttribDialog ui;
	std::vector<KeyValues *> m_selectedAttribs;

public slots:
	virtual void accept()
	{
		for (int i = 0; i < ui.attribList->rowCount(); ++i)
		{
			auto item = ui.attribList->item(i, 0);
			if (item->checkState())
			{
				m_selectedAttribs.push_back(item->data(Qt::UserRole).value<KeyValues *>());
			}
		}
		QDialog::accept();
	};


public:
	AttribDialogIntern(const std::vector<KeyValues *> &attribs, QWidget *parent = 0)
	{
		ui.setupUi(this);
		ui.attribList->setRowCount(attribs.size());

		int i = 0;
		for (auto &key : attribs)
		{
			if (key->FindChildByName("attribute_class"))
			{
				QTableWidgetItem *classEntry = new QTableWidgetItem(QString::fromStdString(key->FindChildByName("attribute_class")->GetData(true)));
				ui.attribList->setItem(i, 1, classEntry);
			}
			
			QTableWidgetItem *attr = new QTableWidgetItem(QString::fromStdString(key->FindChildByName("name")->GetData(true)));
			attr->setFlags(attr->flags() | Qt::ItemIsUserCheckable);
			attr->setCheckState(Qt::Unchecked);
			attr->setData(Qt::UserRole, QVariant::fromValue(key));

			ui.attribList->setItem(i++, 0, attr);
		}
	}

	~AttribDialogIntern() {}

	std::vector<KeyValues *> &getCheckedData() { return m_selectedAttribs; }

};

#endif // GKV_H
