#ifndef PERSON_H_
#define PERSON_H_

#include <QApplication>
#include <QString>
#include <QMetaType>
#include <QDate>

#include "src/model/Entity.h"
#include "src/model/objectList/ObjectModel.h"

class QSqlQuery;

class Person: public Entity
{
	public:
		// *** Types
		class DefaultObjectModel: public ObjectModel<Person>
		{
			public:
				virtual int columnCount () const;
				virtual QVariant displayHeaderData (int column) const;
				virtual QVariant displayData (const Person &object, int column) const;
		};


		// *** Construction
		Person ();
		Person (dbId id);


		// *** Data
		QString lastName;
		QString firstName;
		QString club;
		QString clubId;
		bool checkMedical;
		QDate medicalValidity;


		// *** Comparison
		virtual bool operator== (const Person &o) const { return id==o.id; }
		virtual bool operator< (const Person &o) const;


		// *** Formatting
		virtual QString toString () const;
		virtual QString fullName () const;
		virtual QString formalName () const;
		virtual QString formalNameWithClub () const;
		virtual QString getDisplayName () const;


		// *** ObjectListWindow/ObjectEditorWindow helpers
		// TODO TR this translates poorly, change?
		static QString objectTypeDescription () { return qApp->translate ("Person", "person"); }
		static QString objectTypeDescriptionDefinite () { return qApp->translate ("Person", "the person"); }
		static QString objectTypeDescriptionPlural () { return qApp->translate ("Person", "people"); }


		// SQL interface
		static QString dbTableName ();
		static QString selectColumnList ();
		static Person createFromResult (const Result &result);
		static QString insertColumnList ();
		static QString insertPlaceholderList ();
		virtual void bindValues (Query &q) const;
		static QList<Person> createListFromResult (Result &query);

	private:
		void initialize ();
};

Q_DECLARE_METATYPE (Person);

#endif
