#ifndef SKLABEL_H_
#define SKLABEL_H_

#include <QLabel>
#include <QColor>

#include "src/accessor.h"

/**
 * A QLabel with some additional features
 *
 * An SkLabel can be made invisible without disturbing the layout by setting
 * the foreground color equal to the background color. This is called
 * "concealing" the label.
 *
 * An SkLabel can be set to an error state which affects the background color.
 *
 * An SkLabel has a setText slot which accepts a parameter for choosing
 * richt-text vs. plaintext.
 */
class SkLabel: public QLabel
{
	Q_OBJECT

	public:
		// Construction
		SkLabel (QWidget *parent=0, Qt::WindowFlags f=0);
		SkLabel (const QString &text, QWidget *parent=0, Qt::WindowFlags f=0);

		// Property access
		void setDefaultBackgroundColor (const QColor &color);
		attr_accessor (QColor, ErrorColor, errorColor);
		void resetDefaultBackgroundColor ();
		QColor getDefaultBackgroundColor ();

		void setPaletteForegroundColor (const QColor &color);
		void setPaletteBackgroundColor (const QColor &color);

		using QLabel::setText;

	public slots:
		void setConcealed (bool concealed);
		void setError (bool error);
		void setNumber (int number);
		void setNumber (float number);
		void setNumber (double number);

		void setText (const QString &text, Qt::TextFormat format);

	signals:
		void doubleClicked (QMouseEvent *event);

	protected:
		void updateColors ();
		virtual void mouseDoubleClickEvent (QMouseEvent *event) { emit doubleClicked (event); }

	private:
		bool concealed;
		bool error;

		QColor defaultBackgroundColor;
		bool useDefaultBackgroundColor;
		QColor errorColor;
};

#endif

