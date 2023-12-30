#pragma once

#include <QObject>
#include <QEvent>

class FocusFilter : public QObject
{
	Q_OBJECT

public:
	bool eventFilter(QObject* obj, QEvent* event) override;

signals:
	void onFocusChanged(QObject* sender, bool focus);
};
