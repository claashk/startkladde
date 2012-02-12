#include "Csv.h"

#include <QAbstractTableModel>
#include <QStringList>

Csv::Csv (const QAbstractTableModel &model):
	model (model)
{
}

Csv::~Csv ()
{
}

/**
 * Replaces every quote by double quotes and encloses the string in double
 * quotes if it contains a comma or a quote.
 *
 * foo           => foo
 * foo "bar" baz => "foo ""bar"" baz"
 * foo, bar      => "foo, bar"
 * "foo"         => """foo"""
 * "foo, bar"    => """foo, bar"""
 *
 *
 */
QString Csv::escape (const QString &text) const
{
	QString result=text;

	result.replace ("\"", "\"\"");

	if (result.contains (",") or result.contains ("\""))
		result="\""+result+"\"";

	return result;
}

QString Csv::toString ()
{
	int rows=model.rowCount ();
	int columns=model.columnCount ();

	QStringList items;
	QStringList lines;

	// Header
	for (int column=0; column<columns; ++column)
	{
		QVariant value=model.headerData (column, Qt::Horizontal);
		items.append (escape (value.toString ()));
	}
	lines.append (items.join (","));
	items.clear ();

	// Body
	for (int row=0; row<rows; ++row)
	{
		for (int column=0; column<columns; ++column)
		{
			QVariant value=model.data (model.index (row, column));
			items.append (escape (value.toString ()));
		}
		lines.append (items.join (","));
		items.clear ();
	}

	return lines.join ("\n")+"\n";
}