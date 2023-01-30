#ifndef PC_LINE_EDIT_H_
#define PC_LINE_EDIT_H_

#include <string>
#include <vector>
#include <QLineEdit>

class PcLineEdit : public QLineEdit
{
	Q_OBJECT

public:
	PcLineEdit(QWidget* parent = nullptr);

protected:
	void focusInEvent(QFocusEvent* event) override;
	void focusOutEvent(QFocusEvent* event) override;

signals:
	void onFocusChanged(bool focus);
};

#endif // PC_LINE_EDIT_H_
