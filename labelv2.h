#ifndef LABELV2_H
#define LABELV2_H

#include "ui_labelv2.h"
#include <QMainWindow>

#include <QAction>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QListWidget>
#include <QMap>
#include <QPushButton>
#include <QSettings>
#include <QSlider>
#include <QSplitter>
#include <QString>
#include <QTimer>
#include <QTreeView>
#include <QVector>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "boxperframetreemodel.h"
#include "eventtreemodel.h"
#include "imagelabel.h"
#include "structures.h"
#include "tracktreemodel.h"

class LabelV2 : public QMainWindow
{
	Q_OBJECT

public:
	LabelV2(QWidget *parent = 0);
	~LabelV2();

private:
	Ui::LabelV2Class* ui;

protected:
	QPointF cvtWidgetPt2ImgPt(const QWidget* w, const cv::Mat& m, const QPointF& pt);
	void drawImg(cv::Mat& img);
	void drawBoxes(cv::Mat& img);
	void drawTracks(cv::Mat& img);
	const Box* findNearestBox(int x, int y) const;
	void findOnGoingEvent();
	void jumpToFrame(int fid);
	void readSettings();
	void resizeEvent(QResizeEvent* event);
	void resetVideoRelatedVar();
	void resetBoxList();
	void resetEventList();
	void resetTrackList();
	void setUpLayout();

signals:
	void videoFileChanged();
	void newBoxInserted(const Box&);
	void boxRemoved(int&, int&);

private slots:
	void fetchMouseContent(QMouseEvent*);
	void closeEvent(QCloseEvent*);
	void insertNewBox(const Box&);
	void insertNewEvent(const Event&);
	void onSliderReleased();
	void onBtnPlay();
	void onBtnStop();
	void onBtnPrevFrame();
	void onBtnNextFrame();
	void onBtnPrevEvent();
	void onBtnNextEvent();
	void onChangingDetectionConfidenceThreshold(double);
	void onChangingBoxClass(int);
	void onClikedEventList(const QModelIndex&);
	void openDetectionFile();
	void onDoubleClickedBoxPerFrameList(const QModelIndex&);
	void onDoubleClickedEventList(const QModelIndex&);
	void onDoubleClickedTrackList(const QModelIndex&);
	void openEventFile();
	void openTrackFile();
	void openVideoFile();
	void onCheckShowDetectionResult(int);
	void onCheckShowTracks(int);
	void playOneFrame();
	void removeBoxInTreeView(int&, int&);

private:
	QSettings _app_settings;

	const int _min_wnd_height = 700;
	const int _min_wnd_width = 1400;
	const int _min_lst_width = 300;
	const int _draw_box_thickness = 2;
	const int _draw_track_thickness = 5;
	const int _track_show_delay = 10;
	const cv::Scalar _highlight_box_color = cv::Scalar(255, 255, 0);
	const cv::Scalar _highlight_track_color = cv::Scalar(255, 100, 255);

	QString _video_file;
	QString _detection_file;
	QString _track_file;

	cv::VideoCapture _cap;
	int _curr_frame_id;
	int _total_frame_num;
	int _fps;
	cv::Mat _curr_frame;
	double _detection_confidence_threshold;
	int _selected_box_class;
	bool _show_detection_result;
	bool _show_tracks;
	bool _is_playing;

	QVector<QVector<Box>> _box_list;
	QVector<Track> _track_list;
	QMap<QString, QVector<Event>> _event_list;
	QMap<QString, int> _event_name_index_map;

	QWidget* _main_widget;

	QAction* _act_open_video;
	QAction* _act_open_detection;
	QAction* _act_open_track;
	QAction* _act_open_event;

	QPushButton* _btn_open_video;
	QPushButton* _btn_open_detection;
	QPushButton* _btn_open_event;
	QPushButton* _btn_open_track;
	QPushButton* _btn_next_frame;
	QPushButton* _btn_prev_frame;
	QPushButton* _btn_next_event;
	QPushButton* _btn_prev_event;
	QPushButton* _btn_play;
	QPushButton* _btn_stop;

	QTreeView* _lst_boxes_per_frame;
	QTreeView* _lst_tracks;
	QTreeView* _lst_events;

	BoxPerFrameTreeModel* _model_boxes_per_frame;
	TrackTreeModel* _model_tracks;
	EventTreeModel* _model_events;

	QSplitter* _splt_lists;

	QCheckBox* _chk_show_detection_result;
	QCheckBox* _chk_show_tracks;

	QSpinBox* _spin_box_class;
	QDoubleSpinBox* _spin_detection_confidence_threshold;

	QSlider* _sldr_video_progress;

	QLabel* _lab_curr_frame_id;
	QLabel* _lab_total_frame_id;

	QLabel* _lab_curr_box_confidence;
	QLabel* _lab_curr_box_coord;
	QLabel* _lab_curr_box_class;

	QLabel* _lab_on_going_events;

	QLabel* _lab_box_class;
	QLabel* _lab_detction_confidence_threshold;

	QLabel* _lab_box_list;
	QLabel* _lab_track_list;
	QLabel* _lab_event_list;

	ImageLabel* _img_label;

	QTimer* _timer;
};

#endif // LABELV2_H
