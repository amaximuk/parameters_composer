#include "focus_filter.h"

bool FocusFilter::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::FocusIn)
	{
		emit onFocusChanged(obj, true);
	}
	else if (event->type() == QEvent::FocusOut)
	{
		emit onFocusChanged(obj, false);
	}

	// standard event processing
	return QObject::eventFilter(obj, event);
}
