#include "labelv2.h"
#include "ui_labelv2.h"
#include "cvtimage.h"
#include "dataio.h"
#include <QBoxLayout>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QTimer>
#include <QToolTip>

#include <algorithm>

using namespace cv;

inline double fid2msec(int fid, int fps) {
	return 1000 * static_cast<double>(fid) / fps;
}

LabelV2::LabelV2(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::LabelV2Class) {

	ui->setupUi(this);
	delete ui->mainToolBar;

	_main_widget = new QWidget(this);
	_main_widget->setMinimumWidth(_min_wnd_width);
	_main_widget->setMinimumHeight(_min_wnd_height);
	this->setCentralWidget(_main_widget);

	_act_open_video = new QAction(tr("&Open video"), this);
	_act_open_video->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_V));
	connect(_act_open_video, SIGNAL(triggered()), this, SLOT(openVideoFile()));

	_act_open_detection = new QAction(tr("&Open detection"), this);
	_act_open_detection->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
	_act_open_detection->setEnabled(false);
	connect(_act_open_detection, SIGNAL(triggered()), this, SLOT(openDetectionFile()));

	_act_open_track = new QAction(tr("&Open track"), this);
	_act_open_track->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
	_act_open_track->setEnabled(false);
	connect(_act_open_track, SIGNAL(triggered()), this, SLOT(openTrackFile()));

	_act_open_event = new QAction(tr("&Open event"), this);
	_act_open_event->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
	_act_open_event->setEnabled(false);
	connect(_act_open_event, SIGNAL(triggered()), this, SLOT(openEventFile()));

	QMenu* file = menuBar()->addMenu(tr("&File"));
	file->addAction(_act_open_video);
	file->addAction(_act_open_detection);
	file->addAction(_act_open_track);
	file->addAction(_act_open_event);

	_lab_curr_frame_id = new QLabel(tr(" "), this);
	_lab_curr_frame_id->setMinimumWidth(30);
	_lab_total_frame_id = new QLabel(tr(" "), this);
	_lab_total_frame_id->setMinimumWidth(30);

	_lab_curr_box_coord = new QLabel(tr(" "), this);
	_lab_curr_box_confidence = new QLabel(tr(" "), this);
	_lab_curr_box_class = new QLabel(tr(" "), this);
	_lab_box_class = new QLabel(tr("Box class"), this);
	_lab_detction_confidence_threshold = new QLabel(tr("Confidence"), this);
	_lab_box_list = new QLabel(tr("Box list"), this);
	_lab_box_list->setMaximumHeight(10);
	_lab_track_list = new QLabel(tr("Track list"), this);
	_lab_track_list->setMaximumHeight(10);
	_lab_event_list = new QLabel(tr("Event list"), this);
	_lab_event_list->setMaximumHeight(10);

	statusBar();
	statusBar()->addWidget(_lab_curr_box_coord);
	statusBar()->addWidget(_lab_curr_box_confidence);
	statusBar()->addWidget(_lab_curr_box_class);

	connect(this, SIGNAL(videoFileChanged()),
		this, SLOT(onVideoFileChanged()));

	_btn_open_video = new QPushButton(tr("Video"), this);
	connect(_btn_open_video, SIGNAL(clicked()),
		this, SLOT(openVideoFile()));

	_btn_open_track = new QPushButton(tr("Track"), this);
	_btn_open_track->setEnabled(false);
	connect(_btn_open_track, SIGNAL(clicked()),
		this, SLOT(openTrackFile()));

	_btn_open_detection = new QPushButton(tr("Detection"), this);
	_btn_open_detection->setEnabled(false);
	connect(_btn_open_detection, SIGNAL(clicked()),
		this, SLOT(openDetectionFile()));

	_btn_open_event = new QPushButton(tr("Event"), this);
	_btn_open_event->setEnabled(false);
	connect(_btn_open_event, SIGNAL(clicked()),
		this, SLOT(openEventFile()));

	_btn_play = new QPushButton(this);
	_btn_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	_btn_play->setEnabled(false);
	_btn_play->setShortcut(QKeySequence(Qt::Key_Space));
	connect(_btn_play, SIGNAL(clicked()),
		this, SLOT(onBtnPlay()));

	_btn_stop = new QPushButton(this);
	_btn_stop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
	_btn_stop->setEnabled(false);
	connect(_btn_stop, SIGNAL(clicked()),
		this, SLOT(onBtnStop()));

	_btn_next_frame = new QPushButton(this);
	_btn_next_frame->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
	_btn_next_frame->setEnabled(false);
	_btn_next_frame->setShortcut(QKeySequence(Qt::Key_Period));
	connect(_btn_next_frame, SIGNAL(clicked()),
		this, SLOT(onBtnNextFrame()));

	_btn_prev_frame = new QPushButton(this);
	_btn_prev_frame->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
	_btn_prev_frame->setEnabled(false);
	_btn_prev_frame->setShortcut(QKeySequence(Qt::Key_Comma));
	connect(_btn_prev_frame, SIGNAL(clicked()),
		this, SLOT(onBtnPrevFrame()));

	_btn_prev_event = new QPushButton(tr("<<"), this);
	_btn_prev_event->setEnabled(false);
	_btn_prev_event->setShortcut(QKeySequence(Qt::Key_B));
	_btn_prev_event->setToolTip(tr("previous event"));
	connect(_btn_prev_event, SIGNAL(clicked()),
		this, SLOT(onBtnPrevEvent()));

	_btn_next_event = new QPushButton(tr(">>"), this);
	_btn_next_event->setEnabled(false);
	_btn_next_event->setShortcut(QKeySequence(Qt::Key_N));
	_btn_next_event->setToolTip(tr("next event"));
	connect(_btn_next_event, SIGNAL(clicked()),
		this, SLOT(onBtnNextEvent()));

	_img_label = new ImageLabel(this);
	_img_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	QPalette p = palette();
	p.setColor(QPalette::Window, Qt::black);
	_img_label->setPalette(p);
	_img_label->setAttribute(Qt::WA_OpaquePaintEvent);
	_img_label->setMouseTracking(true);
	_img_label->setMinimumHeight(576);
	_img_label->setMinimumWidth(720);
	connect(_img_label, SIGNAL(mouseMoved(QMouseEvent*)),
		this, SLOT(fetchMouseContent(QMouseEvent*)));

	_chk_show_detection_result = new QCheckBox(tr("Det"), this);
	connect(_chk_show_detection_result, SIGNAL(stateChanged(int)), 
		this, SLOT(onCheckShowDetectionResult(int)));
	_show_detection_result = true;
	_chk_show_detection_result->setChecked(true);

	_chk_show_tracks = new QCheckBox(tr("Tracks"), this);
	connect(_chk_show_tracks, SIGNAL(stateChanged(int)),
		this, SLOT(onCheckShowTracks(int)));
	_show_tracks = true;
	_chk_show_tracks->setChecked(true);

	_spin_detection_confidence_threshold = new QDoubleSpinBox(this);
	_spin_detection_confidence_threshold->setRange(0.0, 1.0);
	_spin_detection_confidence_threshold->setSingleStep(0.02);
	_spin_detection_confidence_threshold->
		setToolTip(tr("detection confidence threshold"));
	connect(_spin_detection_confidence_threshold, SIGNAL(valueChanged(double)),
		this, SLOT(onChangingDetectionConfidenceThreshold(double)));
	_detection_confidence_threshold = 0.7;
	_spin_detection_confidence_threshold->setValue(_detection_confidence_threshold);

	_spin_box_class = new QSpinBox(this);
	_spin_box_class->setRange(0, INT_MAX);
	_spin_box_class->setSingleStep(1);
	_spin_box_class->setToolTip(tr("box class selection"));
	connect(_spin_box_class, SIGNAL(valueChanged(int)),
		this, SLOT(onChangingBoxClass(int)));

	_sldr_video_progress = new QSlider(Qt::Horizontal, this);
	_sldr_video_progress->setEnabled(false);
	connect(_sldr_video_progress, SIGNAL(sliderReleased()), this, SLOT(onSliderReleased()));

	_lst_boxes_per_frame = new QTreeView(this);
	_lst_boxes_per_frame->setMinimumWidth(_min_lst_width);
	_model_boxes_per_frame = new BoxPerFrameTreeModel(_box_list, this);
	_lst_boxes_per_frame->setModel(_model_boxes_per_frame);
	_lst_boxes_per_frame->setColumnWidth(0, 80);
	_lst_boxes_per_frame->setColumnWidth(1, 30);
	_lst_boxes_per_frame->setColumnWidth(2, 45);
	_lst_boxes_per_frame->setColumnWidth(3, 100);
	connect(_lst_boxes_per_frame, SIGNAL(doubleClicked(const QModelIndex&)), 
		this, SLOT(onDoubleClickedBoxPerFrameList(const QModelIndex&)));

	_lst_tracks = new QTreeView(this);
	_lst_tracks->setMinimumWidth(_min_lst_width);
	_model_tracks = new TrackTreeModel(_track_list, this);
	_lst_tracks->setModel(_model_tracks);
	_lst_tracks->setColumnWidth(0, 75);
	_lst_tracks->setColumnWidth(1, 75);
	_lst_tracks->setColumnWidth(2, 75);
	_lst_tracks->setColumnWidth(3, 75);
	connect(_lst_tracks, SIGNAL(doubleClicked(const QModelIndex&)),
		this, SLOT(onDoubleClickedTrackList(const QModelIndex&)));

	_lst_events = new QTreeView(this);
	_lst_events->setMinimumWidth(_min_lst_width);
	_model_events = new EventTreeModel(_event_list, this);
	_lst_events->setModel(_model_events);
	_lst_events->setColumnWidth(0, 120);
	_lst_events->setColumnWidth(1, 75);
	_lst_events->setColumnWidth(2, 75);
	_lst_events->setColumnWidth(3, 75);
	connect(_lst_events, SIGNAL(doubleClicked(const QModelIndex&)),
		this, SLOT(onDoubleClickedEventList(const QModelIndex&)));
	connect(_lst_events, SIGNAL(clicked(const QModelIndex&)),
		this, SLOT(onClikedEventList(const QModelIndex&)));

	_splt_lists = new QSplitter(Qt::Orientation::Vertical, this);
	_splt_lists->addWidget(_lab_box_list);
	_splt_lists->addWidget(_lst_boxes_per_frame);
	_splt_lists->addWidget(_lab_track_list);
	_splt_lists->addWidget(_lst_tracks);
	_splt_lists->addWidget(_lab_event_list);
	_splt_lists->addWidget(_lst_events);

	connect(this, SIGNAL(newBoxInserted(const Box&)), this, SLOT(insertNewBox(const Box&)));
	connect(this, SIGNAL(boxRemoved(int&, int&)), this, SLOT(removeBoxInTreeView(int&, int&)));

	setUpLayout();
	readSettings();

	_timer = new QTimer(this);
	connect(_timer, SIGNAL(timeout()), this, SLOT(playOneFrame()));
}

LabelV2::~LabelV2() {
	delete ui;
}

void LabelV2::setUpLayout() {
	QBoxLayout* videoLayout = new QHBoxLayout;
	QBoxLayout* sliderLayout = new QHBoxLayout;
	QBoxLayout* btnLayout = new QHBoxLayout;
	QBoxLayout* fileLayout = new QHBoxLayout;
	QBoxLayout* controlLayout = new QHBoxLayout;
	QBoxLayout* layout = new QVBoxLayout;

	videoLayout->addWidget(_img_label, 3);
	videoLayout->addWidget(_splt_lists, 1);

	sliderLayout->addWidget(_lab_curr_frame_id);
	sliderLayout->addWidget(_sldr_video_progress);
	sliderLayout->addWidget(_lab_total_frame_id);

	fileLayout->addWidget(_btn_open_video);
	fileLayout->addWidget(_btn_open_detection);
	fileLayout->addWidget(_btn_open_track);
	fileLayout->addWidget(_btn_open_event);

	controlLayout->addWidget(_btn_prev_frame);
	controlLayout->addWidget(_btn_play);
	controlLayout->addWidget(_btn_stop);
	controlLayout->addWidget(_btn_next_frame);
	controlLayout->addSpacing(25);
	controlLayout->addWidget(_btn_prev_event);
	controlLayout->addWidget(_btn_next_event);
	controlLayout->addSpacing(25);
	controlLayout->addWidget(_chk_show_detection_result);
	controlLayout->addWidget(_chk_show_tracks);
	controlLayout->addSpacing(10);
	controlLayout->addWidget(_lab_detction_confidence_threshold);
	controlLayout->addWidget(_spin_detection_confidence_threshold);
	controlLayout->addSpacing(10);
	controlLayout->addWidget(_lab_box_class);
	controlLayout->addWidget(_spin_box_class);

	btnLayout->addLayout(fileLayout);
	btnLayout->addSpacing(25);
	btnLayout->addLayout(controlLayout);

	layout->addLayout(videoLayout);
	layout->addLayout(sliderLayout);
	layout->addLayout(btnLayout);

	_main_widget->setLayout(layout);
}

QPointF LabelV2::cvtWidgetPt2ImgPt(const QWidget* w, const cv::Mat& m, const QPointF& pt) {
	QPointF imgPt;
	cv::Size imgSize = m.size();
	int imgW(imgSize.width), imgH(imgSize.height);
	int wW(w->width()), wH(w->height());
	imgPt.setX((float)imgW / wW * pt.x());
	imgPt.setY((float)imgH / wH * pt.y());
	return imgPt;
}

void LabelV2::readSettings() {
	resize(_app_settings.value("labelv2/size", QSize(_min_wnd_width, _min_wnd_height)).toSize());
	move(_app_settings.value("labelv2/pos", QPoint(200, 200)).toPoint());
}

void LabelV2::resizeEvent(QResizeEvent* event) {
	drawImg(_curr_frame);
	QWidget::resizeEvent(event);
	_app_settings.setValue("labelv2/size", this->size());
	_app_settings.setValue("labelv2/pos", this->pos());
}

void LabelV2::closeEvent(QCloseEvent* event) {
	event->accept();
	_app_settings.setValue("labelv2/pos", this->pos());
	_app_settings.setValue("labelv2/size", this->size());
}

void LabelV2::drawImg(cv::Mat& img) {
	if (img.empty())	return;

	Mat im_copy;
	img.copyTo(im_copy);

	if (!_box_list.empty() && _show_detection_result)
		drawBoxes(im_copy);

	if (_show_tracks)
		drawTracks(im_copy);
	QImage im_to_show = cvMat2QImage(im_copy);
	_img_label->setPixmap(QPixmap::fromImage(im_to_show).scaled(_img_label->width(),
		_img_label->height(), Qt::IgnoreAspectRatio));
}

void LabelV2::drawBoxes(cv::Mat& img) {
	const auto& boxes = _box_list[_curr_frame_id];
	for (const auto& box : boxes) {
		if (box._confidence < _detection_confidence_threshold)	continue;
		auto rct = cv::Rect(cv::Point(box._x1, box._y1), cv::Point(box._x2, box._y2));
		cv::rectangle(img, rct, cv::Scalar(255, 255, 255), _draw_box_thickness, CV_AA);
		cv::rectangle(img, rct, box._color, 1, CV_AA);
	}
}

void LabelV2::drawTracks(cv::Mat& img) {
	int end = _curr_frame_id - _track_show_delay;
	std::string str_tid;
	cv::Point c;
	for (const auto& track : _track_list) {
		if (track._end_fid < end || track._start_fid > _curr_frame_id)	continue;
		str_tid = QString::number(track._tid).toStdString();
		for (const auto& box : track._box_list) {
			if (box._fid < end)	continue;
			if (box._fid > _curr_frame_id) break;
			c.x = (box._x1 + box._x2) >> 1, c.y = (box._y1 + box._y2) >> 1;
			if (box._fid == _curr_frame_id) {
				cv::circle(img, c, _draw_track_thickness, track._color, -1, CV_AA);
				cv::circle(img, c, _draw_track_thickness - 1, cv::Scalar(255, 255, 255), -1, CV_AA);
				cv::putText(img, str_tid, cv::Point(c.x, c.y - 5), 
					CV_FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), _draw_track_thickness, CV_AA);
				cv::putText(img, str_tid, cv::Point(c.x, c.y - 5), 
					CV_FONT_HERSHEY_SIMPLEX, 0.7, track._color, 1, CV_AA);
			}
			else cv::circle(img, c, 2, track._color, -1, CV_AA);
		}
	}
}

void LabelV2::jumpToFrame(int fid) {
	if (_is_playing)	onBtnPlay();
	if (fid < 0 || fid >= _total_frame_num)
		return;
	_cap.set(CV_CAP_PROP_POS_MSEC, fid2msec(fid, _fps));
	_cap >> _curr_frame;
	_curr_frame_id = fid;

	_lab_curr_frame_id->setText(QString::number(_curr_frame_id));
	_sldr_video_progress->setValue(_curr_frame_id);

	drawImg(_curr_frame);
}

void LabelV2::fetchMouseContent(QMouseEvent* event) {
	if (!_curr_frame.empty()) {
		auto pt = event->pos();
		auto img_pt = cvtWidgetPt2ImgPt(_img_label, _curr_frame, pt);
		QToolTip::showText(event->globalPos(),
			QString::number(int(img_pt.x())) + ", " +
			QString::number(int(img_pt.y())),
			this, rect());

		const auto* pbox = findNearestBox(img_pt.x(), img_pt.y());
		if (pbox == nullptr) {
			_lab_curr_box_class->setText(tr(" "));
			_lab_curr_box_confidence->setText(tr(" "));
			_lab_curr_box_coord->setText(tr(" "));
		}
		else {
			_lab_curr_box_class->setText(QString("[CLS] %1").arg(pbox->_class));
			_lab_curr_box_confidence->setText(QString("[CONF] ") + QString::number(pbox->_confidence, 'f', 3));
			_lab_curr_box_coord->setText(QString("[COORD] (X1: %1, Y1: %2), (X2: %3, Y2: %4)").
				arg(pbox->_x1).arg(pbox->_y1).arg(pbox->_x2).arg(pbox->_y2));
		}
	}
	QMainWindow::mouseMoveEvent(event);
}

void LabelV2::resetVideoRelatedVar() {
	_curr_frame.release();
	_curr_frame_id = 0;
	_cap.set(CV_CAP_PROP_POS_MSEC, fid2msec(_curr_frame_id, _fps));
	_cap >> _curr_frame;
	_is_playing = false;
	_total_frame_num = _cap.get(CV_CAP_PROP_FRAME_COUNT);
	_fps = _cap.get(CV_CAP_PROP_FPS);

	_box_list.clear();
	_track_list.clear();
	_event_list.clear();
	_event_name_index_map.clear();
	resetBoxList();
	resetTrackList();

	_lab_curr_frame_id->setText(tr("0"));
	_lab_total_frame_id->setText(QString::number(_total_frame_num));
	_lab_box_class->setText(tr(" "));
	_lab_curr_box_confidence->setText(tr(" "));
	_lab_curr_box_coord->setText(tr(" "));
	_lab_curr_box_confidence->setText(tr(" "));

	_btn_open_detection->setEnabled(true);
	_btn_open_track->setEnabled(true);
	_btn_open_event->setEnabled(true);
	_btn_prev_frame->setEnabled(true);
	_btn_next_frame->setEnabled(true);
	_btn_prev_event->setEnabled(true);
	_btn_next_event->setEnabled(true);
	_btn_play->setEnabled(true);
	_btn_stop->setEnabled(true);

	_act_open_detection->setEnabled(true);
	_act_open_track->setEnabled(true);
	_act_open_event->setEnabled(true);

	_sldr_video_progress->setMinimum(0);
	_sldr_video_progress->setMaximum(_total_frame_num - 1);
	_sldr_video_progress->setSliderPosition(0);
	_sldr_video_progress->setEnabled(true);
}

void LabelV2::resetBoxList() {
	_model_boxes_per_frame->reset();
	QModelIndex& root = QModelIndex();
	for (int i = 0; i < _total_frame_num; ++i) {
		_model_boxes_per_frame->insertRow(i, root);
		QModelIndex& index = _model_boxes_per_frame->index(i, 0, root);
		_model_boxes_per_frame->setData(index, QVariant(QString("%1 [0]").arg(i)));
	}
}

void LabelV2::resetTrackList() {
	_model_tracks->reset();
	for (auto& track : _track_list) {
		int row = _model_tracks->rowCount();
		_model_tracks->insertRow(row, QModelIndex());
		_model_tracks->setData(QModelIndex(), row, track);
	}
}

void LabelV2::resetEventList() {
	_model_events->reset();
	QModelIndex& root = QModelIndex();
	for (auto it = _event_list.begin(); it != _event_list.end(); ++it) {
		int row = _model_events->rowCount();
		_model_events->insertRow(row, root);
		QModelIndex& index = _model_events->index(row, 0, root);
		_model_events->setData(index, QVariant(it.key()));
	}
}

void LabelV2::insertNewBox(const Box& box) {
	int fid = box._fid;
	QModelIndex& frame_row = _model_boxes_per_frame->index(fid, 0);
	int row = _model_boxes_per_frame->rowCount(frame_row);
	_model_boxes_per_frame->setData(frame_row, QVariant(QString("%1 [%2]").arg(fid).arg(_box_list[fid].count())));
	_model_boxes_per_frame->insertRow(row, frame_row);
	_model_boxes_per_frame->setData(frame_row, row, box);
}

void LabelV2::insertNewEvent(const Event& evt) {
	QString name = evt._name;
	QModelIndex& event_list_parent = _model_events->index(_event_name_index_map.find(name).value(), 0);
	int row = _model_events->rowCount(event_list_parent);
	_model_events->insertRow(row, event_list_parent);
	_model_events->setData(event_list_parent, row, evt);
}

void LabelV2::removeBoxInTreeView(int& fid, int& ind) {
	QModelIndex& frame_row = _model_boxes_per_frame->index(fid, 0);
	_model_boxes_per_frame->setData(frame_row, QVariant(QString("%1 [%2]").arg(fid).arg(_box_list[fid].count())));
	_model_boxes_per_frame->removeRow(ind, frame_row);
}

void LabelV2::onSliderReleased() {
	bool is_playing = _is_playing;
	if (_is_playing)	onBtnPlay();
	int fid = _curr_frame_id;
	int pos = _sldr_video_progress->sliderPosition();
	_cap.set(CV_CAP_PROP_POS_MSEC, fid2msec(pos, _fps));
	_cap >> _curr_frame;
	if (_curr_frame.empty()) {
		_cap.set(CV_CAP_PROP_POS_MSEC, fid2msec(fid, _fps));
		_cap >> _curr_frame;
		_sldr_video_progress->setSliderPosition(fid);
		return;
	}
	_curr_frame_id = pos;
	_lab_curr_frame_id->setText(QString::number(pos));
	drawImg(_curr_frame);
	if(is_playing)	onBtnPlay();
}

void LabelV2::openDetectionFile() {
	if (_is_playing)	onBtnPlay();
	QString det_file = QFileDialog::getOpenFileName(this, tr("Open detection file"),
		_app_settings.value("labelv2/lastdetectionpath", "C:/").toString(),
		"Detection files ( *.data *.dtm )");
	if (det_file.isEmpty())	return;
	
	auto ext = QFileInfo(det_file).suffix();
	QVector<QVector<Box>> box_list;
	bool ret = true;
	if (ext == "data")	ret = _readData(det_file, box_list, _detection_confidence_threshold);
	else if (ext == "dtm")	ret = _readDtm(det_file, box_list);
	else {
		QMessageBox::warning(this, tr("Error"), tr("Unsupported format"));
		return;
	}
	if (!ret) {
		QMessageBox::warning(this, tr("Error"), tr("Error reading file: broken file"));
		return;
	}
	if (box_list.size() > _total_frame_num) {
		QMessageBox::warning(this, tr("Error"), tr("Error reading file: unmatched video and detection result"));
		return;
	}
	_app_settings.setValue("labelv2/lastdetectionpath", QFileInfo(det_file).dir().path());

	_box_list.clear();
	_box_list = box_list;
	resetBoxList();
	
	for (auto& box_per_frame : _box_list)
		for (auto& box : box_per_frame)
			emit newBoxInserted(box);

	drawImg(_curr_frame);
}

void LabelV2::openEventFile() {
	if (_is_playing)	onBtnPlay();
	QString event_file = QFileDialog::getOpenFileName(this, tr("Open event file"),
		_app_settings.value("labelv2/lasteventpath", "C:/").toString(),
		"Event span file ( *.eventSpan )");
	if (event_file.isEmpty())	return;
	bool ret = true;
	QMap<QString, QVector<Event>> event_list;
	ret = _readEventSpan(event_file, event_list);
	if (!ret) {
		QMessageBox::warning(this, tr("Error"), tr("Unsupported format"));
		return;
	}
	_app_settings.setValue("labelv2/lasteventpath", QFileInfo(event_file).dir().path());
	_event_list.clear();
	_event_list = event_list;
	_event_name_index_map.clear();
	int index = 0;
	for (auto it = _event_list.begin(); it != _event_list.end(); ++it) {
		_event_name_index_map.insert(it.key(), index++);
		auto& lst = it.value();
		std::sort(lst.begin(), lst.end());
	}
	resetEventList();

	for (auto& list : _event_list)
		for (auto& evt : list)
			emit insertNewEvent(evt);
}

void LabelV2::openTrackFile() {
	if (_is_playing)	onBtnPlay();
	QString track_file = QFileDialog::getOpenFileName(this, tr("Open track file"),
		_app_settings.value("labelv2/lasttrackpath", "C:/").toString(),
		"Track file ( *.fused_track *.track )");
	if (track_file.isEmpty())	return;
	auto ext = QFileInfo(track_file).suffix();
	QVector<Track> track_list;
	bool ret = true;
	if (ext == "fused_track" || ext == "track")	ret = _readFusedTrack(track_file, track_list);
	else {
		QMessageBox::warning(this, tr("Error"), tr("Unsupported format"));
		return;
	}
	if (!ret) {
		QMessageBox::warning(this, tr("Error"), tr("Error reading file: broken file"));
		return;
	}
	_app_settings.setValue("labelv2/lasttrackpath", QFileInfo(track_file).dir().path());
	_track_list.clear();
	_track_list = track_list;
	std::sort(_track_list.begin(), _track_list.end());
	resetTrackList();

	if (_show_tracks)
		drawImg(_curr_frame);
}

void LabelV2::openVideoFile() {
	if (_is_playing)	onBtnPlay();
	QString video_file = QFileDialog::getOpenFileName(this, tr("Open video file"),
		_app_settings.value("labelv2/lastvideopath", "C:/").toString(),
		"Video files ( *.mp4 *.mpeg *.avi *.mov )");
	if (video_file.isEmpty())	return;
	
	auto new_cap = VideoCapture(video_file.toStdString());
	if (!new_cap.isOpened()) {
		QMessageBox::warning(this, tr("Error"), tr("Cannot open video file"));
		return;
	}

	QFileInfo video_file_info(video_file);
	_app_settings.setValue("labelv2/lastvideopath", video_file_info.dir().path());

	_cap = new_cap;
	// reset anything related to current video
	resetVideoRelatedVar();

	drawImg(_curr_frame);
}

void LabelV2::onBtnPlay() {
	if (_is_playing) {
		_timer->stop();
		_is_playing = false;
		_btn_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	}
	else {
		_is_playing = true;
		_btn_play->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
		_timer->start(_fps);
	}
}

void LabelV2::onBtnStop() {
	if (_is_playing)	onBtnPlay();

	_box_list.clear();
	_track_list.clear();
	_event_list.clear();
	_event_name_index_map.clear();
	_model_boxes_per_frame->reset();
	_model_events->reset();
	resetTrackList();

	_curr_frame.release();
	_cap.release();
	_curr_frame_id = 0;
	_total_frame_num = 0;
	drawImg(Mat(576, 720, CV_8UC3, cv::Scalar(0, 0, 0)));

	_lab_curr_frame_id->setText(tr(" "));
	_lab_total_frame_id->setText(tr(" "));
	_lab_box_class->setText(tr(" "));
	_lab_curr_box_confidence->setText(tr(" "));
	_lab_curr_box_coord->setText(tr(" "));
	_lab_curr_box_confidence->setText(tr(" "));

	_act_open_detection->setEnabled(false);
	_act_open_track->setEnabled(false);
	_act_open_event->setEnabled(false);

	_btn_play->setEnabled(false);
	_btn_next_frame->setEnabled(false);
	_btn_prev_frame->setEnabled(false);
	_btn_next_event->setEnabled(false);
	_btn_prev_event->setEnabled(false);
	_btn_stop->setEnabled(false);
	_btn_open_detection->setEnabled(false);
	_btn_open_track->setEnabled(false);
	_btn_open_event->setEnabled(false);
	
	_sldr_video_progress->setEnabled(false);
	_sldr_video_progress->setValue(0);
}

void LabelV2::onBtnPrevFrame() {
	if (_is_playing)	onBtnPlay();
	if (_curr_frame_id == 0)	return;
	_cap.set(CV_CAP_PROP_POS_MSEC, fid2msec(--_curr_frame_id, _fps));
	_cap >> _curr_frame;
	_lab_curr_frame_id->setText(QString::number(_curr_frame_id));
	_sldr_video_progress->setSliderPosition(_curr_frame_id);
	drawImg(_curr_frame);
}

void LabelV2::onBtnNextFrame() {
	if (_is_playing)	onBtnPlay();
	if (_curr_frame_id == _total_frame_num - 1)	return;
	_cap.set(CV_CAP_PROP_POS_MSEC, fid2msec(++_curr_frame_id, _fps));
	_cap >> _curr_frame;
	_lab_curr_frame_id->setText(QString::number(_curr_frame_id));
	_sldr_video_progress->setSliderPosition(_curr_frame_id);
	drawImg(_curr_frame);
}

void LabelV2::onBtnPrevEvent() {
	if (_is_playing)	onBtnPlay();
	int max_start_fid = INT_MIN;
	for (auto it = _event_name_index_map.begin(); it != _event_name_index_map.end(); ++it) {
		QModelIndex index = _model_events->index(it.value(), 0);
		if (_model_events->data(index, Qt::CheckStateRole) == Qt::Unchecked)
			continue;
		auto& lst = _event_list[it.key()];
		int count = lst.count(), last_found = -1;
		for (int i = 0; i < count; ++i) {
			if (lst[i]._start_fid >= _curr_frame_id)	break;
			if (_model_events->data(_model_events->index(i, 0, index), Qt::CheckStateRole) == Qt::Checked)	last_found = i;
		}
		if (last_found != -1)
			max_start_fid = max(max_start_fid, lst[last_found]._start_fid);
	}
	if (max_start_fid >= 0 && max_start_fid <_total_frame_num)
		jumpToFrame(max_start_fid);
}

void LabelV2::onBtnNextEvent() {
	if (_is_playing)	onBtnPlay();
	int min_start_fid = INT_MAX;
	for (auto it = _event_name_index_map.begin(); it != _event_name_index_map.end(); ++it) {
		QModelIndex index = _model_events->index(it.value(), 0);
		if (_model_events->data(index, Qt::CheckStateRole) == Qt::Unchecked)
			continue;
		auto& lst = _event_list[it.key()];
		int count = lst.count(), last_found = -1;
		for (int i = count - 1; i >= 0; --i) {
			if (lst[i]._start_fid <= _curr_frame_id)	break;
			if (_model_events->data(_model_events->index(i, 0, index), Qt::CheckStateRole) == Qt::Checked)	last_found = i;
		}
		if (last_found != -1)
			min_start_fid = min(min_start_fid, lst[last_found]._start_fid);
	}
	if (min_start_fid >= 0 && min_start_fid <_total_frame_num)
		jumpToFrame(min_start_fid);
}

void LabelV2::onChangingDetectionConfidenceThreshold(double th) {
	_detection_confidence_threshold = th;
	drawImg(_curr_frame);
}

void LabelV2::onChangingBoxClass(int cls) {
	_selected_box_class = cls;
}

void LabelV2::onCheckShowDetectionResult(int state) {
	_show_detection_result = state == Qt::Checked;
	drawImg(_curr_frame);
}

void LabelV2::onCheckShowTracks(int state) {
	_show_tracks = state == Qt::Checked;
	drawImg(_curr_frame);
}

void LabelV2::onDoubleClickedBoxPerFrameList(const QModelIndex& index) {
	if (index == QModelIndex())	return;
	const QModelIndex* frame_row = &index;
	int box_ind = -1;
	if (index.parent() != QModelIndex()) {
		frame_row = &index.parent();
		box_ind = index.row();
	}
	int fid = frame_row->row();
	if (box_ind == -1 && fid == _curr_frame_id)	return;
	jumpToFrame(fid);
	if (box_ind != -1) {
		auto& box = _box_list[fid][box_ind];
		auto color = box._color;
		box._color = _highlight_box_color;
		drawImg(_curr_frame);
		box._color = color;
	}
}

void LabelV2::onDoubleClickedEventList(const QModelIndex& index) {
	if (index == QModelIndex() || index.parent() == QModelIndex())	return;
	int event_ind = index.row();
	int event_parent_ind = index.parent().row();
	auto it = _event_name_index_map.begin() + event_parent_ind;
	int fid = _event_list[it.key()][event_ind]._start_fid;
	if (fid < 0 || fid >_total_frame_num)	return;
	jumpToFrame(fid);
	drawImg(_curr_frame);
}

void LabelV2::onClikedEventList(const QModelIndex& index) {
	if (index == QModelIndex())	return;
	_model_events->setData(index, QVariant(), Qt::CheckStateRole);
}

void LabelV2::onDoubleClickedTrackList(const QModelIndex& index) {
	if (index == QModelIndex())	return;
	int track_ind = index.row();
	auto& selected_track = _track_list[track_ind];
	int fid = selected_track._start_fid;
	if (fid < 0 || fid >= _total_frame_num)	return;
	jumpToFrame(fid);
	auto color = selected_track._color;
	selected_track._color = _highlight_track_color;
	drawImg(_curr_frame);
	selected_track._color = color;
}

void LabelV2::playOneFrame() {
	if (_curr_frame_id >= _total_frame_num - 1 && _is_playing)	onBtnPlay();
	_cap.set(CV_CAP_PROP_POS_MSEC, fid2msec(++_curr_frame_id, _fps));
	_cap >> _curr_frame;
	if (_curr_frame.empty()) {
		QMessageBox::warning(this, tr("Error"), tr("Cannot read next frame"));
		if (_is_playing)	onBtnPlay();
	}
	_lab_curr_frame_id->setText(QString::number(_curr_frame_id));
	_sldr_video_progress->setSliderPosition(_curr_frame_id);
	drawImg(_curr_frame);
}

const Box* LabelV2::findNearestBox(int x, int y) const {
	if (_box_list.empty() || !_show_detection_result ||  _is_playing || _box_list[_curr_frame_id].empty())	return nullptr;
	int dx, dy, d, min_d(INT_MAX), cx, cy;
	const Box* pbox = nullptr;
	for (const auto& box : _box_list[_curr_frame_id]) {
		if (x<box._x1 || x>box._x2 || y < box._y1 || y>box._y2 || box._confidence < _detection_confidence_threshold)	continue;
		cx = (box._x1 + box._x2) >> 1, cy = (box._y1 + box._y2) >> 1;
		dx = std::abs(x - cx), dy = std::abs(y - cy);
		d = dx * dx + dy * dy;
		if (d < min_d) {
			min_d = d;
			pbox = &box;
		}
	}
	return pbox;
}