#ifndef FOCUS_FILTER_H_
#define FOCUS_FILTER_H_

#include <QObject>
#include <QEvent>

class FocusFilter : public QObject
{
	Q_OBJECT

public:
	bool eventFilter(QObject* obj, QEvent* event) override
	{
		if (event->type() == QEvent::FocusIn)
		{
			qDebug("Ate key press QEvent::FocusIn");
			emit onFocusChanged(obj, true);
		}
		else if (event->type() == QEvent::FocusOut)
		{
			qDebug("Ate key press QEvent::FocusOut");
			emit onFocusChanged(obj, false);
		}

		// standard event processing
		return QObject::eventFilter(obj, event);
	}

signals:
	void onFocusChanged(QObject* sender, bool focus);
};

#endif // FOCUS_FILTER_H_
