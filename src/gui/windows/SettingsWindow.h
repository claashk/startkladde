#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QtGui/QDialog>

#include "ui_SettingsWindow.h"

class InfoPlugin;
class ShellPluginInfo;

class SettingsWindow: public QDialog
{
    Q_OBJECT

	public:
		SettingsWindow (QWidget *parent=0);
		~SettingsWindow ();

	protected:
		void readSettings ();
		void writeSettings ();

		void readItem (QTreeWidgetItem *item, const InfoPlugin *plugin);
		void makeItemEditable (QListWidgetItem *item);

		bool allowEdit ();
		void warnEdit ();

	private slots:
		void on_mysqlDefaultPortCheckBox_toggled () { updateWidgets (); }

		void on_addPluginPathButton_clicked ();
		void on_removePluginPathButton_clicked ();
		void on_pluginPathUpButton_clicked ();
		void on_pluginPathDownButton_clicked ();

		void on_addInfoPluginButton_clicked ();
		void on_removeInfoPluginButton_clicked ();
		void on_infoPluginUpButton_clicked ();
		void on_infoPluginDownButton_clicked ();
		void on_infoPluginSettingsButton_clicked ();

		void on_infoPluginList_itemDoubleClicked (QTreeWidgetItem *item, int column);

		void updateWidgets ();

		void on_buttonBox_accepted ();

	private:
		Ui::SettingsWindowClass ui;
		bool warned;
		QList<InfoPlugin *> infoPlugins;

	public:
		bool databaseSettingsChanged;
};

#endif
