#include "StdAfx.h"
#include "groupcontrolwidget.h"
#include "PrioritySetManager.h"

GroupControlWidget::GroupControlWidget(HoopsView *_hoopsview, QWidget *parent)
	: hoopsview (_hoopsview), QWidget(parent)
{
	ui.setupUi(this);
	setup_actions ();
}

GroupControlWidget::~GroupControlWidget()
{

}

void GroupControlWidget::setup_actions ()
{
	connect (ui.pushButton_Refresh, SIGNAL (clicked ()), SLOT (on_fresh ()));
	connect (ui.pushButton_Delete, SIGNAL (clicked ()), SLOT (on_delete ()));
	connect (ui.pushButton_Setting, SIGNAL (clicked ()), SLOT (on_setting ()));
	connect (ui.pushButton_Clear_Highlights, SIGNAL (clicked ()), SLOT (on_clear_highlight ()));
	connect (ui.listWidget_Groups, SIGNAL (itemSelectionChanged()), SLOT (on_selection_changed ()));
	connect (ui.listWidget_Groups, SIGNAL (itemChanged(QListWidgetItem *)), SLOT (on_item_changed (QListWidgetItem*)));
}

void GroupControlWidget::on_fresh ()
{
	std::vector<VolumeMeshElementGroup*> groups, invisibile_groups, highlighted_groups;
	hoopsview->get_mesh_groups (groups, invisibile_groups, highlighted_groups);

	auto all_items = ui.listWidget_Groups->findItems(QString("*"), Qt::MatchWrap | Qt::MatchWildcard);
	foreach (auto item, all_items){
		auto group = (VolumeMeshElementGroup*)(item->data (Qt::UserRole).toUInt ());
		if (!contains (groups, group) && !contains (invisibile_groups, group) && !contains (highlighted_groups, group)){
			ui.listWidget_Groups->removeItemWidget (item);
			delete item;
		}
		if (contains (groups, group))
			groups.erase (std::find (groups.begin (), groups.end (), group));
		if (contains (invisibile_groups, group))
			invisibile_groups.erase (std::find (invisibile_groups.begin (), invisibile_groups.end (), group));
		if (contains (highlighted_groups, group))
			highlighted_groups.erase (std::find (highlighted_groups.begin (), highlighted_groups.end (), group));
	}

	auto fInsertItem = [&] (VolumeMeshElementGroup *group)->QListWidgetItem*{
		QListWidgetItem *new_item = new QListWidgetItem (group->type + " : " + group->name);
		new_item->setFlags (new_item->flags () | Qt::ItemIsUserCheckable);
		new_item->setData (Qt::UserRole, QVariant (unsigned int (group)));
		new_item->setData (Qt::UserRole + 1, QVariant (false));
		new_item->setCheckState (Qt::Unchecked);
		ui.listWidget_Groups->addItem (new_item);
		return new_item;
	};
	foreach (auto group, groups){
		auto item = fInsertItem (group);
		item->setCheckState (Qt::Checked);
	}
	
	foreach (auto group, highlighted_groups){
		auto item = fInsertItem (group);
		item->setSelected (true);
		item->setCheckState (Qt::Checked);
	}

	foreach (auto group, invisibile_groups){
		auto item = fInsertItem (group);
		item->setCheckState (Qt::Unchecked);
	}
}

void GroupControlWidget::on_delete ()
{
	auto selected_groups = ui.listWidget_Groups->selectedItems ();
	foreach (auto item, selected_groups){
		auto group = (VolumeMeshElementGroup*)(item->data (Qt::UserRole).toUInt ());
		hoopsview->derender_one_mesh_group (group);
		if (group->delete_when_unrender)
			delete group;
		ui.listWidget_Groups->removeItemWidget (item);
		delete item;
	}
}

void GroupControlWidget::on_setting ()
{

}

void GroupControlWidget::on_clear_highlight ()
{
	auto selected_groups = ui.listWidget_Groups->selectedItems ();
	foreach (auto item, selected_groups){
		auto group = (VolumeMeshElementGroup*)(item->data (Qt::UserRole).toUInt ());
		hoopsview->dehighlight_mesh_group (group);
		item->setData (Qt::UserRole + 1, QVariant (false));
		item->setSelected (false);
	}
}

void GroupControlWidget::on_selection_changed ()
{
	auto selected_groups = ui.listWidget_Groups->selectedItems ();
	QList<QListWidgetItem*> all_groups;
	for (int i = 0; i != ui.listWidget_Groups->count (); ++i){
		auto item = ui.listWidget_Groups->item (i);
		if (item->data (Qt::UserRole + 1).toBool ()){
			if (!selected_groups.contains (item)){
				auto group = (VolumeMeshElementGroup*)(item->data (Qt::UserRole).toUInt ());
				hoopsview->dehighlight_mesh_group (group);
				item->setData (Qt::UserRole + 1, QVariant (false));
			}
		}else{
			if (selected_groups.contains (item)){
				auto group = (VolumeMeshElementGroup*)(item->data (Qt::UserRole).toUInt ());
				hoopsview->highlight_mesh_group (group);
				item->setData (Qt::UserRole + 1, QVariant (true));
			}
		}
	}
}

void GroupControlWidget::on_item_changed (QListWidgetItem *item)
{
	if (!item) return;
	if (item->checkState () == Qt::Checked){
		auto group = (VolumeMeshElementGroup*)(item->data (Qt::UserRole).toUInt ());
		hoopsview->show_mesh_group (group, true);
	}else{
		auto group = (VolumeMeshElementGroup*)(item->data (Qt::UserRole).toUInt ());
		hoopsview->show_mesh_group (group, false);
	}
}