#include "treeitem.h"
#include <QStringList>

TreeItem::TreeItem(const QVector<QVariant>& data, TreeItem* parent) {
	_parent_item = parent;
	_item_data = data;
	_checked = true;
}

TreeItem::~TreeItem() {
	qDeleteAll(_child_items);
}

TreeItem* TreeItem::child(int row) {
	return _child_items.value(row);
}

TreeItem* TreeItem::parent() {
	return _parent_item;
}

int TreeItem::childCount() const {
	return _child_items.count();
}

int TreeItem::columnCount() const {
	return _item_data.count();
}

int TreeItem::row() const {
	if (_parent_item)
		return _parent_item->_child_items.indexOf(const_cast<TreeItem*>(this));
	return 0;
}

QVariant TreeItem::data(int column) const {
	return _item_data.value(column);
}

void TreeItem::appendChild(TreeItem* child) {
	_child_items.append(child);
}

bool TreeItem::insertChild(int position, int count, int columns) {
	if (position < 0 || position > _child_items.size())
		return false;
	for (int row = 0; row < count; ++row) {
		QVector<QVariant> data(columns);
		TreeItem* item = new TreeItem(data, this);
		_child_items.insert(position, item);
	}
	return true;
}

bool TreeItem::removeChild(int position, int count) {
	if (position < 0 || position + count > _child_items.size())
		return false;
	for (int row = 0; row < count; ++row)
		delete _child_items.takeAt(position);
	return true;
}

bool TreeItem::setData(int column, const QVariant& value) {
	if (column < 0 || column >= _item_data.size())
		return false;

	_item_data[column] = value;
	return true;
}