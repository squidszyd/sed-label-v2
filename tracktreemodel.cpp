#include "tracktreemodel.h"
#include "treeitem.h"
#include <QStringList>

TrackTreeModel::TrackTreeModel(const QVector<Track>& data, QObject* parent) {
	QVector<QVariant> root_data;
	root_data << "TID" << "START" << "END" << "LEN";
	_root_item = new TreeItem(root_data);
	setupModelData(data, _root_item);
}

TrackTreeModel::~TrackTreeModel() {
	delete _root_item;
}

int TrackTreeModel::columnCount(const QModelIndex& parent) const {
	if (parent.isValid())
		return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
	else
		return _root_item->columnCount();
}

int TrackTreeModel::rowCount(const QModelIndex& parent) const {
	TreeItem* parent_item;
	if (parent.column() > 0)
		return 0;
	if (!parent.isValid())
		parent_item = _root_item;
	else
		parent_item = static_cast<TreeItem*>(parent.internalPointer());
	return parent_item->childCount();
}

QVariant TrackTreeModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid() || role != Qt::DisplayRole)
		return QVariant();
	TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
	return item->data(index.column());
}

QVariant TrackTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return _root_item->data(section);
	else return QVariant();
}

QModelIndex TrackTreeModel::index(int row, int column, const QModelIndex& parent) const {
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

QModelIndex TrackTreeModel::parent(const QModelIndex& index) const {
	if (!index.isValid())
		return QModelIndex();

	TreeItem* child_item = static_cast<TreeItem*>(index.internalPointer());
	TreeItem* parent_item = child_item->parent();

	if (parent_item == _root_item)
		return QModelIndex();
	return createIndex(parent_item->row(), 0, parent_item);
}

Qt::ItemFlags TrackTreeModel::flags(const QModelIndex& index) const {
	if (!index.isValid())
		return 0;
	return QAbstractItemModel::flags(index);
}

void TrackTreeModel::setupModelData(const QVector<Track>& data, TreeItem* parent) {
	for (int i = 0; i < data.count(); ++i) {
		auto& track = data[i];
		QVector<QVariant> row_data;
		row_data << QString::number(track._tid) << QString::number(track._start_fid)
			<< QString::number(track._end_fid) << QString::number(track._box_num);
		TreeItem* new_track_row = new TreeItem(row_data, parent);
		parent->appendChild(new_track_row);
	}
}

TreeItem* TrackTreeModel::getItem(const QModelIndex& index) const {
	if (index.isValid()) {
		TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
		if (item)
			return item;
	}
	return _root_item;
}

bool TrackTreeModel::insertRows(int position, int rows, const QModelIndex& parent) {
	TreeItem* parent_item = getItem(parent);
	bool success;

	beginInsertRows(parent, position, position + rows - 1);
	success = parent_item->insertChild(position, rows, _root_item->columnCount());
	endInsertRows();

	return success;
}

bool TrackTreeModel::removeRows(int position, int rows, const QModelIndex& parent) {
	TreeItem* parent_item = getItem(parent);
	bool success = true;

	beginRemoveRows(parent, position, position + rows - 1);
	success = parent_item->removeChild(position, rows);
	endRemoveRows();

	return success;
}

bool TrackTreeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
	if (role != Qt::DisplayRole)
		return false;

	TreeItem* item = getItem(index);
	bool result = item->setData(index.column(), value);

	if (result)
		emit dataChanged(index, index);
	return result;
}

bool TrackTreeModel::setData(const QModelIndex& parent, int row, const Track& track) {
	bool result = true;
	for (int c = 0; c < this->columnCount() && result; ++c) {
		QModelIndex& index = this->index(row, c, parent);
		switch (c) {
		case 0:
			result = this->setData(index, QVariant(QString::number(track._tid)));
			break;
		case 1:
			result = this->setData(index, QVariant(QString::number(track._start_fid)));
			break;
		case 2:
			result = this->setData(index, QVariant(QString::number(track._end_fid)));
			break;
		case 3:
			result = this->setData(index, QVariant(QString::number(track._box_num)));
			break;
		default:
			break;
		}
	}
	return result;
}

void TrackTreeModel::reset() {
	if (this->rowCount() == 0)	return;
	beginResetModel();
	removeRows(0, this->rowCount(), QModelIndex());
	endResetModel();
}