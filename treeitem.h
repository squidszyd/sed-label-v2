#ifndef TREEITEM_H
#define TREEITEM_H

#include <QVector>
#include <QVariant>
#include <QList>
#include "structures.h"

class TreeItem {
public:
	explicit TreeItem(const QVector<QVariant>& data, TreeItem* parent = 0);
	~TreeItem();

	TreeItem* child(int row);
	TreeItem* parent();
	
	int childCount() const;
	int columnCount() const;
	int row() const;

	bool insertChild(int position, int count, int columns);
	bool removeChild(int position, int count);
	bool setData(int column, const QVariant& value);
	bool isChecked() const { return _checked; }
	void setChecked(bool set) { _checked = set; }

	void appendChild(TreeItem* child);

	QVariant data(int column) const;

private:
	QList<TreeItem*> _child_items;
	QVector<QVariant> _item_data;
	TreeItem* _parent_item;
	bool _checked;
};

#endif