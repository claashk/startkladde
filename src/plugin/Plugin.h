#ifndef PLUGIN_H_
#define PLUGIN_H_

#include <qobject.h>

//#include "src/config/Settings.h"

class QWidget;
class PluginSettingsPane;
class QSettings;

#define SK_PLUGIN \
	public: \
		virtual QString getId          () const; \
		virtual QString getName        () const; \
		virtual QString getDescription () const; \
		static QString _getId          (); \
		static QString _getName        (); \
		static QString _getDescription ();

#define SK_PLUGIN_DEFINITION(klass, id, name, description) \
	QString klass::getId           () const { return id;          } \
	QString klass::getName         () const { return name;        } \
	QString klass::getDescription  () const { return description; } \
	QString klass::_getId          ()       { return id;          } \
	QString klass::_getName        ()       { return name;        } \
	QString klass::_getDescription ()       { return description; }


/**
 * A common base class for all plugins
 *
 * A plugin:
 *   - has an ID, a name and a description
 *   - can be started and terminated
 *   - may have settings which can be written to and read from a QSettings and
 *     edited by the user using a PluginSettingsPane
 */
class Plugin: public QObject
{
		Q_OBJECT

	public:
		Plugin ();
		virtual ~Plugin ();

		// FIXME use a guid for Id?
		virtual QString getId          () const=0;
		virtual QString getName        () const=0;
		virtual QString getDescription () const=0;

	public slots:
		/**
		 * Starts the plugin. Time or memory consuming operations should not be
		 * performed before this method is called. This method may be called
		 * when the plugin has already been started.
		 */
		virtual void start ()=0;

		/**
		 * Terminates the plugin. Time or memory consuming operations should
		 * not be performed after this method is called. This method may be
		 * called when the plugin has not been started.
		 */
		virtual void terminate ()=0;

		/**
		 * Terminates the plugin and starts it again. If the plugin has not
		 * been started, it will be started.
		 */
		virtual void restart ();

		/**
		 * Reads the settings from the given QSettings
		 *
		 * The QSettings must have been placed at the correct group.
		 */
		virtual void readSettings (const QSettings &settings)=0;

		/**
		 * Writes the settings to the given QSettings
		 *
		 * The QSettings must have been placed at the correct group.
		 */
		virtual void writeSettings (QSettings &settings)=0;

		/**
		 * Creates a PluginSettingsPane for editing the settings of this plugin
		 * instance
		 *
		 * The caller takes ownership of the created PluginSettingsPane.
		 *
		 * @param parent the parent widget for the plugin pane
		 * @return a newly created PluginSettingsPane instance
		 */
		virtual PluginSettingsPane *createSettingsPane (QWidget *parent=NULL)=0;

		virtual void minuteChanged () {}

		virtual QString configText () const=0;

		virtual bool filenameIsAbsolute (const QString &filename) const;
		virtual QString resolveFilename (const QString &filename, const QStringList &pluginPaths) const;
};

#endif
