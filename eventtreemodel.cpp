#include "EventTreeModel.h"
#include "treeitem.h"
#include <QStringList>

EventTreeModel::EventTreeModel(const QMap<QString, QVector<Event>>& data, QObject* parent) {
	QVector<QVariant> root_data;
	root_data << "NAME" << "START" << "END" << "LEN";
	_root_item = new TreeItem(root_data);
	setupModelData(data, _root_item);
}

EventTreeModel::~EventTreeModel() {
	delete _root_item;
}

int EventTreeModel::columnCount(const QModelIndex& parent) const {
	if (parent.isValid())
		return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
	else
		return _root_item->columnCount();
}

int EventTreeModel::rowCount(const QModelIndex& parent) const {
	TreeItem* parent_item;
	if (parent.column() > 0)
		return 0;
	if (!parent.isValid())
		parent_item = _root_item;
	else
		parent_item = static_cast<TreeItem*>(parent.internalPointer());
	return parent_item->childCount();
}

QVariant EventTreeModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid())
		return QVariant();
	TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
	if (role == Qt::CheckStateRole && index.column() == 0)
		return static_cast<int>(item->isChecked() ? Qt::Checked : Qt::Unchecked);
	if (role == Qt::DisplayRole)
		return item->data(index.column());
	else
		return QVariant();
}

QVariant EventTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return _root_item->data(section);
	else return QVariant();
}

QModelIndex EventTreeModel::index(int row, int column, const QModelIndex& parent) const {
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	TreeItem* parent_item;
	if (!parent.isValid())
		parent_item = _root_item;
	else
		parent_item = static_cast<TreeItem*>(parent.internalPointer());

	TreeItem* child_item = parent_item->child(row);
	if (child_item)
		return createIndex(row, column, child_item);
	else
		return QModelIndex();
}

QModelIndex EventTreeModel::parent(const QModelIndex& index) const {
	if (!index.isValid())
		return QModelIndex();

	TreeItem* child_item = static_cast<TreeItem*>(index.internalPointer());
	TreeItem* parent_item = child_item->parent();

	if (parent_item == _root_item)
		return QModelIndex();
	return createIndex(parent_item->row(), 0, parent_item);
}

Qt::ItemFlags EventTreeModel::flags(const QModelIndex& index) const {
	if (!index.isValid())
		return 0;
	Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	if (index.column() == 0)
		flags |= Qt::ItemIsUserCheckable;
	return flags;
}

void EventTreeModel::setupModelData(const QMap<QString, QVector<Event>>& data, TreeItem* parent) {
	for (auto it = data.begin(); it != data.end(); ++it) {
		auto& event_name = it.key();
		auto& event_list = it.value();
		QVector<QVariant> event_data;
		event_data << event_name << "" << "" << "";
		TreeItem* event_list_parent = new TreeItem(event_data, parent);
		for (auto& evt : event_list) {
			QVector<QVariant> column_data;
			column_data << "" << QString::number(evt._start_fid) << QString::number(evt._end_fid)
				<< QString::number(evt._end_fid - evt._start_fid + 1);
			event_list_parent->appendChild(new TreeItem(column_data, event_list_parent));
		}
		parent->appendChild(event_list_parent);
	}
}

TreeItem* EventTreeModel::getItem(const QModelIndex& index) const {
	if (index.isValid()) {
		TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
		if (item)
			return item;
	}
	return _root_item;
}

bool EventTreeModel::insertRows(int position, int rows, const QModelIndex& parent) {
	TreeItem* parent_item = getItem(parent);
	bool success;

	beginInsertRows(parent, position, position + rows - 1);
	success = parent_item->insertChild(position, rows, _root_item->columnCount());
	endInsertRows();

	return success;
}

bool EventTreeModel::removeRows(int position, int rows, const QModelIndex& parent) {
	TreeItem* parent_item = getItem(parent);
	bool success = true;

	beginRemoveRows(parent, position, position + rows - 1);
	success = parent_item->removeChild(position, rows);
	endRemoveRows();

	return success;
}

bool EventTreeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
	bool result = false;
	QModelIndex last_index = index;
	if (role == Qt::DisplayRole) {
		TreeItem* item = getItem(index);
		result = item->setData(index.column(), value);
	}
	else if (role == Qt::CheckStateRole && index.column() == 0) {
		TreeItem* item = getItem(index);
		item->setChecked(!(item->isChecked()));
		for (int i = 0; i < item->childCount(); ++i) {
			auto* child = item->child(i);
			child->setChecked(item->isChecked());
		}
		if (index.parent() != QModelIndex() && item->isChecked())
			getItem(index.parent())->setChecked(item->isChecked());
		last_index = index.child(index.row(), 0);
		result = true;
	}
	if (result)
		emit dataChanged(index, last_index);
	return result;
}

bool EventTreeModel::setData(const QModelIndex& parent, int row, const Event& evt) {
	bool result = true;
	for (int c = 0; c < this->columnCount() && result; ++c) {
		QModelIndex& index = this->index(row, c, parent);
		switch (c) {
		case 0:
			break;
		case 1:
			result = this->setData(index, QVariant(QString::number(evt._start_fid)));
			break;
		case 2:
			result = this->setData(index, QVariant(QString::number(evt._end_fid)));
			break;
		case 3:
			result = this->setData(index, QVariant(QString::number(evt._end_fid - evt._start_fid + 1)));
			break;
		default:
			break;
		}
	}
	return result;
}

void EventTreeModel::reset() {
	if (this->rowCount() == 0)	return;
	beginResetModel();
	removeRows(0, this->rowCount(), QModelIndex());
	endResetModel();
}
