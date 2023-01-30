#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <regex>
#include <QFocusEvent>
#include <QDebug>
#include "pc_line_edit.h"

PcLineEdit::PcLineEdit(QWidget* parent):
	QLineEdit(parent)
{

}

void PcLineEdit::focusInEvent(QFocusEvent* event)
{
	emit onFocusChanged(true);
	QLineEdit::focusInEvent(event);
}

void PcLineEdit::focusOutEvent(QFocusEvent* event)
{
	emit onFocusChanged(false);
	QLineEdit::focusOutEvent(event);
}
