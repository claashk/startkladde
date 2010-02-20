#include "ShellPlugin.h"

#include <fstream>

//#include <sys/types.h>
//#include <unistd.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <fcntl.h>
//#include <signal.h>

#include <iostream>

#include <QLabel>
#include <QTimer>
#include <QTextCodec>
#include <QToolTip>
#include <QProcess>

#include "src/text.h"
#include "src/config/Options.h"


// Construction/destruction
void ShellPlugin::init ()
{
	restart_interval=-1;
	rich_text=false;
	caption_display=NULL;
	value_display=NULL;
	warn_on_death=false;
	subprocess=NULL;

	// The plugin output encoding is UTF8, but it is interpreted as latin1 and
	// converted to UTF-8 by QProcess. It may not be obvious that this
	// conversion (codec->toUnicode) will work, but it does.
	codec=QTextCodec::codecForName ("UTF-8");
}

ShellPlugin::ShellPlugin ()
{
	init ();
	caption="Time:";
	command="date +%H:%M";
	restart_interval=-1;
}

ShellPlugin::ShellPlugin (const ShellPlugin &o):
	QObject ()
{
	(*this)=o;
}

ShellPlugin::ShellPlugin (const QString desc)
{
	init ();

	QStringList split=desc.split(',');
	trim (split);

	QStringList::iterator end=split.end ();
	QStringList::iterator it=split.begin ();

	if (it!=end) caption=*it;
	if (++it!=end) command=*it;
	if (++it!=end) restart_interval=(*it).toInt ();
	while (++it!=end)
	{
		if ((*it)=="warn_on_death") warn_on_death=true;
		else if ((*it)=="rich_text") rich_text=true;
	}
}
//

ShellPlugin::ShellPlugin (const QString &_caption, const QString &_command, int _interval)
{
	init ();
	caption=_caption;
	command=_command;
	restart_interval=_interval;
}

ShellPlugin &ShellPlugin::operator= (const ShellPlugin &o)
{
	caption=o.caption;
	command=o.command;
	rich_text=o.rich_text;
	restart_interval=o.restart_interval;
	warn_on_death=o.warn_on_death;
	subprocess=NULL;
	caption_display=o.caption_display;
	value_display=o.value_display;
	return *this;
}

ShellPlugin::~ShellPlugin ()
{
	if (subprocess) subprocess->terminate ();
	sched_yield ();
}
//


void ShellPlugin::start ()
{
	if (subprocess)
	{
		subprocess->terminate ();
		sched_yield ();
		delete subprocess;
	}

	if (command.isEmpty ())
	{
		if (value_display)
		{
			value_display->setTextFormat (Qt::PlainText);
			value_display->setText ("Keine Plugin-Datei angegeben.");
			QToolTip::remove (value_display);
		}
		return;
	}

	// Separate the command in a file name and paramters at the first ' '
	QString command_file, command_parameters;
	int first_space=command.indexOf (' ');
	if (first_space<0)
	{
		command_file=command;
		command_parameters.clear ();
	}
	else
	{
		// foo bar
		// 0123456
		command_file=command.left (first_space);
		command_parameters=command.mid (first_space+1);
	}

	// Find the plugin file from the plugin path list.
	QString command_file_dir, command_file_basename;
	if (opts.find_plugin_file (command_file, &command_file_dir, &command_file_basename).isEmpty ())
	{
		// The plugin file was not found.
		emit pluginNotFound ();

		if (value_display)
		{
			value_display->setTextFormat (Qt::PlainText);
			value_display->setText ("Plugin-Datei \""+command_file+"\" nicht gefunden.");
			QToolTip::remove (value_display);
		}
	}
	else
	{
		// If the plugin path was explicitly given (either relative or
		// absolute), we execute the process from the current directory. If
		// not, it was found in the plugin_path and we execute it from there.
		QString working_dir;
		QString binary_file;
		if (!command_file.contains ('/'))
		{
			// Found in path
			working_dir=command_file_dir;
			binary_file="./"+command_file_basename;
		}
		else
		{
			working_dir=".";
			binary_file=command_file;
		}

		QString complete_command=binary_file+" "+command_parameters;

		if (value_display)
		{
			QToolTip::add (value_display, complete_command+" ["+working_dir+"]");
		}

		QStringList args;
		args.append ("/bin/sh");
		args.append ("-c");
		args.append (complete_command);

		subprocess=new QProcess (this);
		subprocess->setWorkingDirectory (working_dir);

//		QObject::connect (subprocess, SIGNAL (destroyed (QObject *)), subprocess, SLOT (kill ()));
		QObject::connect (subprocess, SIGNAL (readyReadStandardOutput ()), this, SLOT (output_available ()));
		QObject::connect (subprocess, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (subprocess_died ()));



		if (value_display)
			value_display->setText ("");

		// Qt4
		subprocess->start ("/bin/sh -c \""+complete_command+"\"", QIODevice::ReadOnly);
		subprocess->closeWriteChannel ();

	}
}

void ShellPlugin::output_available ()
{
	if (!subprocess) return;

	QString line;
	while (line=codec->toUnicode (subprocess->readLine ().trimmed ().constData ()), !line.isEmpty ())
	{
		emit lineRead (line);

		if (value_display)
		{
			// Let the plugins wrap, or else the window might get wider than the screen
//			if (rich_text) line="<nobr>"+line+"</nobr>";
			value_display->setText (line);
		}
	}
}

void ShellPlugin::subprocess_died ()
{
	if (warn_on_death) std::cout << "The process for '" << caption << "' died." << std::endl;
	if (restart_interval>=0) QTimer::singleShot (restart_interval*1000, this, SLOT (start ()));
}

void ShellPlugin::terminate ()
{
	if (subprocess) subprocess->terminate ();
}

void ShellPlugin::restart ()
{
	terminate ();
	start ();
}

