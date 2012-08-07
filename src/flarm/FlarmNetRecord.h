#ifndef FLARMNETRECORD_H_
#define FLARMNETRECORD_H_

#include <QApplication>
#include <QString>
#include <QList>
#include <QMetaType>

#include "src/model/objectList/ObjectModel.h"
#include "src/model/Entity.h"

class FlarmNetRecord: public Entity
{
	public:
		// *** Construction
		FlarmNetRecord ();
		FlarmNetRecord (dbId id);

		// *** Data
		QString flarmId;
		QString registration;
		QString owner;
		QString type;
		QString callsign;
		QString frequency;


		// *** Formatting
		virtual QString toString () const;
		virtual QString getDisplayName () const;


		// *** ObjectListWindow/ObjectEditorWindow helpers
		static QString objectTypeDescription () { return qApp->translate ("FlarmNetRecord", "FlarmNet record"); }
		static QString objectTypeDescriptionDefinite () { return qApp->translate ("FlarmNetRecord", "the FlarmNet record"); }
		static QString objectTypeDescriptionPlural () { return qApp->translate ("FlarmNetRecord", "FlarmNet records"); }


		// SQL interface
		static QString dbTableName ();
		static QString selectColumnList ();
		static FlarmNetRecord createFromResult (const Result &result);
		static QString insertColumnList ();
		static QString insertPlaceholderList ();
		virtual void bindValues (Query &q) const;
		static QList<FlarmNetRecord> createListFromResult (Result &query);

	private:
		void initialize ();

};

Q_DECLARE_METATYPE (FlarmNetRecord);

#endif