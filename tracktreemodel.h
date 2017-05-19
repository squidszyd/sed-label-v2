#ifndef TRACK_TREE_MODEL
#define TRACK_TREE_MODEL

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include "treeitem.h"
#include "structures.h"


class TrackTreeModel : public QAbstractItemModel {
	Q_OBJECT

public:
	TrackTreeModel(const QVector<Track>& data, QObject* parent);
	~TrackTreeModel();

	QVariant data(const QModelIndex& index, int role) const override;
	QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& index) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	bool insertRows(int position, int rows, const QModelIndex& parent = QModelIndex()) override;
	bool removeRows(int position, int rows, const QModelIndex& parent = QModelIndex()) override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole) override;
	bool setData(const QModelIndex& index, int row, const Track& track);
	void reset();

private:
	void setupModelData(const QVector<Track>& data, TreeItem* parent);
	TreeItem* getItem(const QModelIndex& index) const;

	TreeItem* _root_item;
};

#endif