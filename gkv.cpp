#include "gkv.h"
#include <QFileDialog>

#include "KeyValues.h"
#include "KVModel.h"

GKV::GKV(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	
	m_Model = new KVModel();
	//m_Model->MountValues(data);

	ui.treeView->setModel(m_Model);
	//ui.treeView->resizeColumnToContents(0);
	//ui.treeView->resizeColumnToContents(1);

	ui.treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.treeView, SIGNAL(customContextMenuRequested(const QPoint &)),
		this, SLOT(onCustomContextMenu(const QPoint &)));
	connect(ui.actionQuit, SIGNAL(triggered()), this, SLOT(onQuit()));
	connect(ui.actionMountData, SIGNAL(triggered()), this, SLOT(onMountValues()));
	connect(ui.actionNew, SIGNAL(triggered()), this, SLOT(onNew()));
	connect(ui.actionAddAttrib, SIGNAL(triggered()), this, SLOT(onAddAttrib()));
}

GKV::~GKV()
{

}

void GKV::onQuit()
{
	qApp->exit();
}

void GKV::onMountValues()
{
	std::string path = QFileDialog::getOpenFileName(this, "Mount data from file").toStdString();
	if (path != "")
	{
		KeyValues *data = new KeyValues(path);
		data->LoadFromFile(path);
		m_Model->MountValues(data);
		
		if (KeyValues *cur = data->FindChildByName("items_game"))
			if (cur = cur->FindChildByName("attributes"))
			{
				KeyValues *sub = cur->GetChildAt(0);
				for (int i = 0; i < cur->GetChildCount(); sub = cur->GetChildAt(++i))
				{
					m_AttributeTable.push_back(sub);
				}
			}
	}
}

void GKV::onNew()
{
	m_Model->MountValues(new KeyValues("[no data]"));
}

void GKV::onAddAttrib()
{
	AttribDialogIntern diag(m_AttributeTable, this);
	diag.exec();

	if (diag.result() == QDialog::DialogCode::Accepted)
	{
		KeyValues *item = m_Model->getItem(ui.treeView->currentIndex());
		if (item->IsValue())
			return;

		for (auto &attr : diag.getCheckedData())
		{
			m_Model->ManualWriteBegin(ui.treeView->currentIndex());
			if (!item->FindChildByName("attributes"))
			{
				item->InsertChildren(item->GetChildCount(), 1);
				item->GetChildAt(item->GetChildCount() - 1)->SetData(false, "attributes");
			}

			if (KeyValues *attribKey = item->FindChildByName("attributes"))
			{
				attribKey->InsertChildren(attribKey->GetChildCount(), 1);
				KeyValues *subKey = attribKey->GetChildAt(attribKey->GetChildCount() - 1);
				subKey->SetData(false, attr->FindChildByName("name")->GetData(true));
				subKey->InsertChildren(0, 2);
				subKey->GetChildAt(0)->SetMode(true);
				subKey->GetChildAt(0)->SetData(false, "attribute_class");
				
				if (auto classKey = attr->FindChildByName("attribute_class"))
					subKey->GetChildAt(0)->SetData(true, classKey->GetData(true));
				
				subKey->GetChildAt(1)->SetMode(true);
				subKey->GetChildAt(1)->SetData(false, "value");

				if (auto typeKey = attr->FindChildByName("description_format"))
				{
					typeKey->GetData(true);
					subKey->GetChildAt(1)->SetData(true, typeKey->GetData(true) == "value_is_percentage" ||
														 typeKey->GetData(true) == "value_is_inverted_percentage" ? "1.0" : "0");
				}
				else
					subKey->GetChildAt(1)->SetData(true, "0");
			}
			m_Model->ManualWriteEnd();
		}
	}
}

void GKV::onCustomContextMenu(const QPoint &point)
{
#define ADD_KEY "Add key here"
#define DELETE_KEY "Delete key"

#define ADD_VALUE "Add value here"
#define DELETE_VALUE "Delete value"

#define ADD_SUB_KEY "Add subkey"
#define ADD_SUB_VALUE "Add subvalue"

#define SAVE_KEY "Save key to file"

#define COPY_KEY "Copy key"
#define PASTE_KEY "Paste key as subkey"

	static KeyValues *clipboard = nullptr;

	QMenu valueMenu;
	valueMenu.addAction(ADD_VALUE);
	valueMenu.addAction(ADD_KEY);
	valueMenu.addSeparator();
	valueMenu.addAction(DELETE_VALUE);
	valueMenu.addSeparator();

	QMenu keyMenu;
	keyMenu.addAction(ADD_VALUE);
	keyMenu.addAction(ADD_KEY);
	keyMenu.addSeparator();
	keyMenu.addAction(ADD_SUB_VALUE);
	keyMenu.addAction(ADD_SUB_KEY);
	keyMenu.addSeparator();
	keyMenu.addAction(DELETE_KEY);
	keyMenu.addSeparator();
	keyMenu.addAction(SAVE_KEY);
	keyMenu.addSeparator();
	keyMenu.addAction(COPY_KEY);
	if (clipboard)
		keyMenu.addAction(PASTE_KEY);

	QModelIndex index = ui.treeView->indexAt(point);
	if (index.isValid())
	{
		KeyValues *item = m_Model->getItem(index);
		QAction *selectedAction;

		if (item->IsValue())
			selectedAction = valueMenu.exec(ui.treeView->mapToGlobal(point));
		else
			selectedAction = keyMenu.exec(ui.treeView->mapToGlobal(point));

		if (selectedAction)
		{
			int id = item->GetParent()->FindChild(item);

			if (selectedAction->text() == DELETE_KEY)
			{
				m_Model->removeRows(id, 1, index.parent());
			}
			else if (selectedAction->text() == DELETE_VALUE)
			{
				m_Model->removeRows(id, 1, index.parent());
			}
			else if (selectedAction->text() == ADD_VALUE)
			{
				m_Model->insertRows(id + 1, 1, index.parent());
				item->GetParent()->GetChildAt(id + 1)->SetMode(true);
			}
			else if (selectedAction->text() == ADD_KEY)
			{
				m_Model->insertRows(id + 1, 1, index.parent());
			}
			else if (selectedAction->text() == ADD_SUB_VALUE)
			{
				m_Model->insertRows(0, 1, index);
				item->GetChildAt(0)->SetMode(true);
			}
			else if (selectedAction->text() == ADD_SUB_KEY)
			{
				m_Model->insertRows(0, 1, index);
			}
			else if (selectedAction->text() == SAVE_KEY)
			{
				std::string path = QFileDialog::getSaveFileName(this, SAVE_KEY, QString::fromStdString(item->GetData(false))).toStdString();
				if (path != "")
					item->SaveToFile(path);
			}
			else if (selectedAction->text() == COPY_KEY)
			{
				clipboard = item;
			}
			else if (selectedAction->text() == PASTE_KEY)
			{
				m_Model->ManualWriteBegin(index);
				item->InsertChildren(item->GetChildCount(), clipboard->Copy());
				m_Model->ManualWriteEnd();
			}

		}		
	}
}