#include <QMap>
#include <QString>
#include <QVector>

#include <fstream>
#include <string>
#include <sstream>
using std::ifstream;
using std::ofstream;
using std::string;
using std::stringstream;

#include "structures.h"

bool _readData(const QString& path, QVector<QVector<Box>>& box_list, float conf_th = 0.7) {
	box_list.clear();
	ifstream ifs = ifstream(path.toStdString(), std::ios::binary);
	if (!ifs.is_open())
		return false;
	Box box;
	int fid, num;
	try {
		while (ifs.read((char*)&fid, sizeof(int))) {
			ifs.read((char*)&num, sizeof(int));
			while (box_list.size() <= fid)
				box_list.push_back(QVector<Box>());
			for (int i = 0; i < num; ++i) {
				ifs.read((char*)&box._x1, sizeof(int));
				ifs.read((char*)&box._y1, sizeof(int));
				ifs.read((char*)&box._x2, sizeof(int));
				ifs.read((char*)&box._y2, sizeof(int));
				ifs.read((char*)&box._confidence, sizeof(float));
				if (box._confidence < conf_th)	continue;
				box._class = 0;
				box._fid = fid;
				box.setColor(box._class);
				box_list[fid] << box;
			}
		}
	}
	catch (...) {
		box_list.clear();
		return false;
	}
	ifs.close();
	return true;
}

bool _readDtm(const QString& path, QVector<QVector<Box>>& box_list) {
	box_list.clear();
	ifstream ifs(path.toStdString(), std::ios::binary);
	if (!ifs.is_open())
		return false;
	Box box;
	try {
		while (ifs.read((char*)&box._fid, sizeof(int))) {
			ifs.read((char*)&box._confidence, sizeof(float));
			ifs.read((char*)&box._class, sizeof(int));
			ifs.read((char*)&box._x1, sizeof(int));
			ifs.read((char*)&box._y1, sizeof(int));
			ifs.read((char*)&box._x2, sizeof(int));
			ifs.read((char*)&box._y2, sizeof(int));
			while (box_list.size() <= box._fid)
				box_list.push_back(QVector<Box>());
			box_list[box._fid] << box;
		}
	}
	catch (...) {
		box_list.clear();
		return false;
	}
	ifs.close();
	return true;
}

bool _writeData(const QString& path, const QVector<QVector<Box>>& box_list) {
	ofstream ofs(path.toStdString(), std::ios::binary);
	if (!ofs.is_open())
		return false;
	int num;
	try {
		for (int fid = 0; fid < box_list.size(); ++fid) {
			const auto box_per_frame = box_list[fid];
			num = box_per_frame.size();
			ofs.write((char*)&fid, sizeof(int));
			ofs.write((char*)&num, sizeof(int));
			for (const auto& box : box_per_frame) {
				ofs.write((char*)&box._x1, sizeof(int));
				ofs.write((char*)&box._y1, sizeof(int));
				ofs.write((char*)&box._x2, sizeof(int));
				ofs.write((char*)&box._y2, sizeof(int));
				ofs.write((char*)&box._confidence, sizeof(float));
			}
		}
	}
	catch (...) {
		return false;
	}
	ofs.close();
	return true;
}

bool _writeDtm(const QString& path, const QVector<QVector<Box>>& box_list) {
	ofstream ofs(path.toStdString(), std::ios::binary);
	if (!ofs.is_open())
		return false;
	try {
		for (const auto& box_per_frame : box_list) {
			for (const auto& box : box_per_frame) {
				ofs.write((char*)&box._fid, sizeof(int));
				ofs.write((char*)&box._confidence, sizeof(float));
				ofs.write((char*)&box._class, sizeof(int));
				ofs.write((char*)&box._x1, sizeof(int));
				ofs.write((char*)&box._y1, sizeof(int));
				ofs.write((char*)&box._x2, sizeof(int));
				ofs.write((char*)&box._y2, sizeof(int));
			}
		}
	}
	catch (...) {
		return false;
	}
	ofs.close();
	return true;
}

bool _readFusedTrack(const QString& path, QVector<Track>& track_list) {
	track_list.clear();
	ifstream ifs(path.toStdString(), std::ios::binary);
	if (!ifs.is_open())
		return false;
	Track track;
	Box box;
	int dumb_value;
	try {
		while (ifs.read((char*)&track._tid, sizeof(int))) {
			track._box_list.clear();
			ifs.read((char*)&track._start_fid, sizeof(int));
			ifs.read((char*)&track._end_fid, sizeof(int));
			ifs.read((char*)&track._box_num, sizeof(int));
			for (int i = 0; i < track._box_num; ++i) {
				ifs.read((char*)&box._confidence, sizeof(float));
				ifs.read((char*)&box._fid, sizeof(int));
				ifs.read((char*)&dumb_value, sizeof(int));
				ifs.read((char*)&box._x1, sizeof(int));
				ifs.read((char*)&box._y1, sizeof(int));
				ifs.read((char*)&box._x2, sizeof(int));
				ifs.read((char*)&box._y2, sizeof(int));
				ifs.read((char*)&dumb_value, sizeof(int));
				track._box_list << box;
			}
			track.setColor(track._tid);
			track_list << track;
		}
	}
	catch (...) {
		track_list.clear();
		return false;
	}
	ifs.close();
	return true;
}

bool _readEventSpan(const QString& path, QMap<QString, QVector<Event>>& event_list) {
	event_list.clear();
	ifstream ifs(path.toStdString());
	if (!ifs.is_open())
		return false;
	string video_info, event_info, name;
	int id;
	Event evt;
	try {
		std::getline(ifs, video_info);
		while (std::getline(ifs, event_info)) {
			stringstream ss(event_info);
			ss >> name >> id >> evt._start_fid >> evt._end_fid;
			evt._name = QString::fromStdString(name);
			evt._track_num = 0;
			event_list[evt._name] << evt;
		}
	}
	catch (...) {
		event_list.clear();
		return false;
	}
	ifs.close();
	return true;
}

