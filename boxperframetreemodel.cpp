#include "boxperframetreemodel.h"
#include "treeitem.h"
#include <QStringList>

BoxPerFrameTreeModel::BoxPerFrameTreeModel(const QVector<QVector<Box>>& data, QObject* parent) {
	QVector<QVariant> root_data;
	root_data << "ID" << "CLS" << "CONF" << "COORD";
	_root_item = new TreeItem(root_data);
	setupModelData(data, _root_item);
}

BoxPerFrameTreeModel::~BoxPerFrameTreeModel() {
	delete _root_item;
}

int BoxPerFrameTreeModel::columnCount(const QModelIndex& parent) const {
	if (parent.isValid())
		return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
	else
		return _root_item->columnCount();
}

int BoxPerFrameTreeModel::rowCount(const QModelIndex& parent) const {
	TreeItem* parent_item;
	if (parent.column() > 0)
		return 0;
	if (!parent.isValid())
		parent_item = _root_item;
	else
		parent_item = static_cast<TreeItem*>(parent.internalPointer());
	return parent_item->childCount();
}

QVariant BoxPerFrameTreeModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid() || role != Qt::DisplayRole)
		return QVariant();
	TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
	return item->data(index.column());
}

QVariant BoxPerFrameTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return _root_item->data(section);
	else return QVariant();
}

QModelIndex BoxPerFrameTreeModel::index(int row, int column, const QModelIndex& parent) const {
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

QModelIndex BoxPerFrameTreeModel::parent(const QModelIndex& index) const {
	if (!index.isValid())
		return QModelIndex();

	TreeItem* child_item = static_cast<TreeItem*>(index.internalPointer());
	TreeItem* parent_item = child_item->parent();

	if (parent_item == _root_item)
		return QModelIndex();
	return createIndex(parent_item->row(), 0, parent_item);
}

Qt::ItemFlags BoxPerFrameTreeModel::flags(const QModelIndex& index) const {
	if (!index.isValid())
		return 0;
	return QAbstractItemModel::flags(index);
}

void BoxPerFrameTreeModel::setupModelData(const QVector<QVector<Box>>& data, TreeItem* parent) {
	for (int f = 0; f < data.count(); ++f) {
		auto& box_list = data[f];
		QVector<QVariant> frame_data;
		frame_data << QString::number(f) << "" << "" << "" << "";
		TreeItem* frame_parent = new TreeItem(frame_data, parent);
		for (int id = 0; id < box_list.count(); ++id) {
			auto& box = box_list[id];
			QVector<QVariant> column_data;
			column_data << QString::number(id)
				<< QString::number(box._class)
				<< QString::number(box._confidence, 'f', 3)
				<< QString("(%1, %2), (%3, %4)").arg(box._x1).arg(box._y1).arg(box._x2).arg(box._y2);
			frame_parent->appendChild(new TreeItem(column_data, frame_parent));
		}
		parent->appendChild(frame_parent);
	}
}

TreeItem* BoxPerFrameTreeModel::getItem(const QModelIndex& index) const {
	if (index.isValid()) {
		TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
		if (item)
			return item;
	}
	return _root_item;
}

bool BoxPerFrameTreeModel::insertRows(int position, int rows, const QModelIndex& parent) {
	TreeItem* parent_item = getItem(parent);
	bool success;

	beginInsertRows(parent, position, position + rows - 1);
	success = parent_item->insertChild(position, rows, _root_item->columnCount());
	endInsertRows();

	return success;
}

bool BoxPerFrameTreeModel::removeRows(int position, int rows, const QModelIndex& parent) {
	TreeItem* parent_item = getItem(parent);
	bool success = true;

	beginRemoveRows(parent, position, position + rows - 1);
	success = parent_item->removeChild(position, rows);
	endRemoveRows();

	return success;
}

bool BoxPerFrameTreeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
	if (role != Qt::DisplayRole)
		return false;

	TreeItem* item = getItem(index);
	bool result = item->setData(index.column(), value);

	if (result)
		emit dataChanged(index, index);
	return result;
}

bool BoxPerFrameTreeModel::setData(const QModelIndex& parent, int row, const Box& box) {
	bool result = true;
	for (int c = 0; c < this->columnCount() && result; ++c) {
		QModelIndex& index = this->index(row, c, parent);
		switch (c) {
		case 0:
			break;
		case 1:
			result = this->setData(index, QVariant(QString::number(box._class)));
			break;
		case 2:
			result = this->setData(index, QVariant(QString::number(box._confidence, 'f', 3)));
			break;
		case 3:
			result = this->setData(index, QVariant(QString("(%1, %2), (%3, %4)").arg(box._x1).arg(box._y1).arg(box._x2).arg(box._y2)));
			break;
		default:
			break;
		}
	}
	return result;
}

void BoxPerFrameTreeModel::reset() {
	if (this->rowCount() == 0)	return;
	beginResetModel();
	removeRows(0, this->rowCount(), QModelIndex());
	endResetModel();
}