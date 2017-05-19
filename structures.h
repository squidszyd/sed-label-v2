#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <QVector>
#include <opencv/cv.h>

struct Box {
	int _fid;
	int _x1;
	int _y1;
	int _x2;
	int _y2;
	int _class;
	float _confidence;
	cv::Scalar _color;

	Box(const Box& other):
		_fid(other._fid), _x1(other._x1), _y1(other._y1),_x2(other._x2), 
		_y2(other._y2), _class(other._class), _confidence(other._confidence), _color(other._color){}

	Box(int fid = 0, int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0, int cls = 0, float confidence = 0.0):
		_fid(fid), _x1(x1), _y1(y1), _x2(x2), _y2(y2), _class(cls), _confidence(confidence){
		setColor(_class);
	}

	void setColor(int cls) {
		_color = cv::Scalar((cls * 30 + 30) % 256, (cls * 80 + 80) % 256, (cls * 130 + 130) % 256);
	}

	bool operator< (const Box& other)const { return this->_fid < other._fid; }
};

Q_DECLARE_METATYPE(Box)

struct Track {
	int _tid;
	int _start_fid;
	int _end_fid;
	int _box_num;
	QVector<Box> _box_list;
    cv::Scalar _color;

	Track():_tid(0), _start_fid(0), _end_fid(0), _box_num(0), _color(0, 0, 0){}

	Track(const Track& other) :
		_tid(other._tid), _start_fid(other._start_fid), _end_fid(other._end_fid),
		_box_num(other._box_num), _color(other._color) {
		_box_list.clear();
		foreach(const Box& box, other._box_list)
			_box_list << box;
	}

	Track(int tid, QVector<Box> box_list):_tid(tid), _box_list(box_list){
		_box_num = box_list.size();
		if(_box_num == 0){
			_start_fid = 0;
			_end_fid = 0;
		}
		else{
			_start_fid = _box_list.front()._fid;
			_end_fid = _box_list.back()._fid;
		}
		setColor(_tid);
	}

	void setColor(int tid) {
        _color = cv::Scalar((_tid * 30 + 30)%256, (_tid * 80 + 80)%256, (_tid * 130 + 130)%256);
	}

	bool operator<(const Track& other) const { return this->_start_fid < other._start_fid; }
};

Q_DECLARE_METATYPE(Track)

struct Event {
	QString _name;
	int _start_fid;
	int _end_fid;
	int _track_num;
	QVector<Track> _track_list;

    Event():_name(), _start_fid(0), _end_fid(0){}
    Event(const QString name, int start_fid, int end_fid, QVector<Track> track_list):
		_name(name), _start_fid(start_fid), _end_fid(end_fid), _track_list(track_list){}
	
	bool operator<(const Event& other) const { return this->_start_fid < other._start_fid; }
};

Q_DECLARE_METATYPE(Event)

#endif
