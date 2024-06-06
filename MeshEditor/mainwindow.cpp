#include "stdafx.h"
#include "mainwindow.h"
#include "PrioritySetManager.h"
#include "topologyoptwidget.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	setup_actions ();
	ui.tabWidget->setTabsClosable (true);
	ui.tabWidget->setMovable (true);
	ui.tabWidget->hide ();
}

MainWindow::~MainWindow()
{

}

void MainWindow::setup_actions ()
{
	connect (ui.tabWidget, SIGNAL (currentChanged(int)), SLOT (on_tab_changed (int)));
	connect (ui.tabWidget, SIGNAL (tabCloseRequested(int)), SLOT (on_tab_close_requested (int)));
	connect(ui.action_Hex_Topology_Opt, SIGNAL(triggered()), SLOT(on_hex_topology_opt()));
	connect(ui.action_Quad_Surfaces, SIGNAL(triggered()), SLOT(on_quad_surfaces()));
}

void MainWindow::on_tab_changed (int index)
{
	QList<QToolBar *> prev_toolbars = findChildren<QToolBar *>();
	foreach (auto toolbar, prev_toolbars){
		//removeToolBar (toolbar);
		toolbar->hide ();
	}
	//当index为-1时，表示没有任何tab窗口了
	if (index == -1){
		ui.tabWidget->hide ();
		return;
	}

	auto widget = ui.tabWidget->widget (index);
	auto object_name = widget->objectName ();
	std::vector<std::vector<QToolBar*> > new_toolbars;
	if (object_name == "TopologyOptWidget"){
		new_toolbars = ((TopologyOptWidget*)widget)->get_toolbars ();
	}
	
	foreach (auto one_line_toolbar, new_toolbars){
		foreach (auto toolbar, one_line_toolbar){
			if (!contains (widgets_in_tabwidget, widget))
				addToolBar (toolbar);
			toolbar->show ();
		}
		if (!contains (widgets_in_tabwidget, widget))
			addToolBarBreak (Qt::TopToolBarArea);
	}
	widgets_in_tabwidget.insert (widget);
}

void MainWindow::on_tab_close_requested (int index)
{
	auto widget = ui.tabWidget->widget (index);
	ui.tabWidget->removeTab (index);
	delete widget;
	widgets_in_tabwidget.erase (widget);
}


void MainWindow::on_hex_topology_opt()
{
	auto topology_opt_widget = new TopologyOptWidget (this);
	topology_opt_widget->setObjectName ("TopologyOptWidget");
	ui.tabWidget->addTab (topology_opt_widget, "TopologyOpt");
	ui.tabWidget->setCurrentWidget (topology_opt_widget);
	ui.tabWidget->show ();
}


void MainWindow::on_quad_surfaces()
{
	
}