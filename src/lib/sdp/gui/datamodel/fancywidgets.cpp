/**************************************************************************
 *                                                                        *
 *  Copyright (C) 2015 OVSM/IPGP                                          *
 *                                                                        *
 *  This file is part of Seismic Data Playback 'SDP'.                     *
 *                                                                        *
 *  SDP is free software: you can redistribute it and/or modify           *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                   *
 *                                                                        *
 *  SDP is distributed in the hope that it will be useful,                *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with SDP. If not, see <http://www.gnu.org/licenses/>.           *
 *                                                                        *
 **************************************************************************/


#include <sdp/gui/datamodel/fancywidgets.h>
#include <sdp/gui/datamodel/macros.h>
#include <QtGui>


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

static const float DEFAULT_DPI = 96.0;

qreal windowsDpiScale(const qreal& logicalX) {
	return logicalX / DEFAULT_DPI;
}

static const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const int stringLength = sizeof(alphanum) - 1;

QString genRandomString(const size_t& length) {

	QString alpha;
	for (size_t i = 0; i < length; ++i)
		alpha += alphanum[rand() % stringLength];

	return alpha;
}

QImage grayscaleImage(QImage& m) {

	int pixels = m.width() * m.height();
	if ( pixels * (int) sizeof(QRgb) <= m.byteCount() ) {
		QRgb* data = (QRgb*) m.bits();
		for (int i = 0; i < pixels; i++) {
			int val = qGray(data[i]);
			data[i] = qRgba(val, val, val, qAlpha(data[i]));
		}
	}

	return m;
}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



namespace SDP {
namespace Qt4 {


//! General pannel's padding
static int const pBM = 10;


FancyButton::FancyButton(QWidget* parent) :
		QAbstractButton(parent), __normal(NULL), __hovered(NULL),
		__clicked(NULL), __blinked(NULL), __grayscaled(NULL), __type(Separator),
		__state(Normal), __capabitlity(Dead), __activated(false),
		__animationCounter(0) {

	setEnabled(false);

	__timer = new QTimer(this);
	connect(__timer, SIGNAL(timeout()), this, SLOT(refresh()));

	//! Handler High DPI displays
	__pixelsRatio = (::windowsDpiScale(logicalDpiX()) > 2.0) ? 15. : .9;
}


FancyButton::FancyButton(const QString& text, QWidget* parent) :
		QAbstractButton(parent), __normal(NULL), __hovered(NULL),
		__clicked(NULL), __blinked(NULL), __grayscaled(NULL), __type(TextOnly),
		__state(Normal), __capabitlity(Standard), __activated(true),
		__animationCounter(0) {

	setText(text);
	setCheckable(true);
	setMouseTracking(true);

	__timer = new QTimer(this);
	connect(__timer, SIGNAL(timeout()), this, SLOT(refresh()));

	//! Handler High DPI displays
	__pixelsRatio = (::windowsDpiScale(logicalDpiX()) > 2.0) ? 15. : .9;
}


FancyButton::FancyButton(const QPixmap& icon, QWidget* parent) :
		QAbstractButton(parent), __normal(NULL), __hovered(NULL),
		__clicked(NULL), __blinked(NULL), __grayscaled(NULL), __icon(icon),
		__type(IconOnly), __state(Normal), __capabitlity(Standard),
		__activated(true), __animationCounter(0) {

	setCheckable(true);
	setMouseTracking(true);

	__timer = new QTimer(this);
	connect(__timer, SIGNAL(timeout()), this, SLOT(refresh()));

	//! Handler High DPI displays
	__pixelsRatio = (::windowsDpiScale(logicalDpiX()) > 2.0) ? 15. : .9;
}


FancyButton::FancyButton(const QPixmap& icon, const QString& text, QWidget* parent) :
		QAbstractButton(parent), __normal(NULL), __hovered(NULL),
		__clicked(NULL), __blinked(NULL), __grayscaled(NULL), __icon(icon),
		__type(IconText), __state(Normal), __capabitlity(Standard),
		__activated(true), __animationCounter(0) {

	setText(text);
	setCheckable(true);
	setMouseTracking(true);

	__timer = new QTimer(this);
	connect(__timer, SIGNAL(timeout()), this, SLOT(refresh()));

	//! Handler High DPI displays
	__pixelsRatio = (::windowsDpiScale(logicalDpiX()) > 2.0) ? 15. : .9;
}


FancyButton::~FancyButton() {
	delete __normal;
	delete __hovered;
	delete __clicked;
	delete __blinked;
	delete __grayscaled;
}

void FancyButton::resizeEvent(QResizeEvent* event) {
	Q_UNUSED(event);
	initPixmaps();
}

void FancyButton::paintEvent(QPaintEvent* event) {

	Q_UNUSED(event);

	QPainter painter(this);

	if ( isEnabled() ) {
		switch ( __state ) {
			case Normal:
				if ( __normal ) painter.drawPixmap(0, 0, *__normal);
				break;
			case Hovered:
				if ( __hovered && __capabitlity != Dead )
					painter.drawPixmap(0, 0, *__hovered);
				else if ( __normal ) painter.drawPixmap(0, 0, *__normal);
				break;
			case Clicked:
				if ( __clicked && isCheckable() )
					painter.drawPixmap(0, 0, *__clicked);
				else {
					if ( __normal ) painter.drawPixmap(0, 0, *__normal);
				}
				break;
			case Blinked:
				if ( __blinked ) painter.drawPixmap(0, 0, *__blinked);
				break;
		}
	}
	else {
		if ( __grayscaled )
			painter.drawPixmap(0, 0, *__grayscaled);
		else if ( __normal ) painter.drawPixmap(0, 0, *__normal);
	}
}

void FancyButton::enterEvent(QEvent* event) {

	setCursor(Qt::PointingHandCursor);

	Q_UNUSED(event);
	if ( __state == Normal )
	    __state = Hovered, update();
}

void FancyButton::leaveEvent(QEvent* event) {

	Q_UNUSED(event);

	setCursor(Qt::ArrowCursor);

	if ( __state != Clicked )
	    __state = Normal, update();
}

void FancyButton::mousePressEvent(QMouseEvent* event) {

	Q_UNUSED(event);
	__state = Clicked;

	emit clicked();

	update();
}


void FancyButton::setActivated(const bool& activated) {
	setEnabled(activated);
	initPixmaps();
}


void FancyButton::animate(const int& msec) {
	__animationCounter = 0;
	__timer->start(msec);
}


void FancyButton::redraw() {
	initPixmaps();
	update();
}


void FancyButton::refresh() {

	if ( __state == Normal )
		__state = Blinked;
	else
		__state = Normal;

	if ( __animationCounter == 20 ) {
		__timer->stop();
		__state = Normal;
	}
	else
		++__animationCounter;

	update();
}


const FancyButton::FancyButtonType& FancyButton::type() const {
	return __type;
}


void FancyButton::setState(const FancyButtonState& state) {
	__state = state;
	update();
}


const FancyButton::FancyButtonState& FancyButton::buttonState() const {
	return __state;
}


void FancyButton::setButtonCapability(const FancyButtonCapability& c) {
	__capabitlity = c;
}


const FancyButton::FancyButtonCapability& FancyButton::buttonCapability() const {
	return __capabitlity;
}


void FancyButton::initPixmaps() {

	initPixmap(&__normal);
	initPixmap(&__hovered);
	initPixmap(&__clicked);
	initPixmap(&__blinked);
	initPixmap(&__grayscaled);

	switch ( __type ) {
		case TextOnly:
			initTextOnly();
			break;
		case IconText:
			initIconText();
			break;
		case IconOnly:
			initIconOnly();
			break;
		case Separator:
			initSeparator();
			break;
	}
}


void FancyButton::initPixmap(QPixmap** pixmap) {
	delete *pixmap;
	*pixmap = new QPixmap(size());
	(*pixmap)->fill(Qt::transparent);
}


void FancyButton::initTextOnly() {

	QLinearGradient hoveredGrad(QPointF(0, 0), QPointF(0, height() - 2));
	hoveredGrad.setColorAt(0, QColor(255, 255, 255, 20));
	hoveredGrad.setColorAt(.5, QColor(255, 255, 255, 10));
	hoveredGrad.setColorAt(1, QColor(255, 255, 255, 20));

	QFont textFont(font());
	textFont.setBold(true);

	QTextOption textOption;
	textOption.setAlignment(Qt::AlignCenter);

	QPolygon border;
	border << QPoint(0, 0)
	    << QPoint(width() - 2, 0)
	    << QPoint(width() - 2, height() - 2)
	    << QPoint(0, height() - 2);

	QPainter painter;

	//! Normal
	painter.begin(__normal);
	painter.setFont(textFont);
	painter.setPen(Qt::white);
	painter.drawText(this->rect(), text(), textOption);
	painter.end();

	//! Hovered
	painter.begin(__hovered);
	painter.setFont(textFont);
	painter.setPen(QColor(255, 255, 255, 70));
	painter.setBrush(hoveredGrad);
	painter.drawPolygon(border);
	painter.setPen(Qt::white);
	painter.drawText(this->rect(), text(), textOption);
	painter.end();

	//! Clicked
	painter.begin(__clicked);
	painter.setFont(textFont);
	painter.setPen(QColor(0, 0, 0, 210));
	painter.setBrush(QColor(255, 255, 255, 200));
	painter.drawPolygon(border);
	painter.drawText(this->rect(), text(), textOption);
	painter.end();

	//! Blinked
	painter.begin(__blinked);
	painter.setFont(textFont);
//	painter.setPen(QColor(247, 173, 68, 100));
	painter.setPen(QColor(0, 0, 0, 210));
	painter.setBrush(QColor(247, 173, 68, 100));
	painter.drawPolygon(border);
	painter.setPen(Qt::white);
	painter.drawText(this->rect(), text(), textOption);
	painter.end();

	//! Grayscaled
	painter.begin(__grayscaled);
	painter.setFont(textFont);
	painter.setPen(QColor(255, 255, 255, 100));
//	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.drawText(this->rect(), text(), textOption);
	painter.setPen(Qt::white);
	painter.end();
}


void FancyButton::initIconText() {

	QLinearGradient hoveredGrad(QPointF(0, 0), QPointF(0, height()));
	hoveredGrad.setColorAt(0, QColor(255, 255, 255, 20));
	hoveredGrad.setColorAt(.5, QColor(255, 255, 255, 10));
	hoveredGrad.setColorAt(1, QColor(255, 255, 255, 20));

	QFont textFont(font());
	textFont.setBold(true);
#ifndef Q_OS_MAC
	textFont.setPointSize(textFont.pointSize() - 1);
#else
	textFont.setPointSize(textFont.pointSize() - 2);
#endif

	QTextOption textOption;
	textOption.setAlignment(Qt::AlignCenter);

	QPolygon border;
	border << QPoint(0, 0)
	    << QPoint(width() - 2, 0)
	    << QPoint(width() - 2, height() - 2)
	    << QPoint(0, height() - 2);

	QFontMetrics metrics(textFont);
	QRect iconRect(QPoint(0, 0), QSize(width(), height() - metrics.height() * 2));
	QPixmap scaled = __icon.scaled(iconRect.size() * __pixelsRatio, Qt::KeepAspectRatio,
	    Qt::SmoothTransformation);

	QPoint point((width() / 2) - scaled.size().width() / 2, pBM);
	QRect labelRect(iconRect.bottomLeft(), QSize(width(), height() - iconRect.height()));

	QPainter painter;

	//! Normal
	painter.begin(__normal);
	painter.setFont(textFont);
	painter.setPen(Qt::white);
	painter.drawPixmap(point, scaled);
	painter.drawText(labelRect, text(), textOption);
	painter.end();

	//! Hovered
	painter.begin(__hovered);
	painter.setFont(textFont);
	painter.setPen(QColor(255, 255, 255, 70));
	painter.setBrush(hoveredGrad);
	painter.drawPolygon(border);
	painter.setPen(Qt::white);
	painter.drawPixmap(point, scaled);
	painter.drawText(labelRect, text(), textOption);
	painter.end();

	//! Clicked
	painter.begin(__clicked);
	painter.setFont(textFont);
	painter.setPen(QColor(0, 0, 0, 210));
	painter.setBrush(QColor(255, 255, 255, 200));
	painter.drawPolygon(border);
	painter.drawPixmap(point, scaled);
	painter.drawText(labelRect, text(), textOption);
	painter.end();

	//! Blinked
	painter.begin(__blinked);
	painter.setFont(textFont);
//	painter.setPen(QColor(247, 173, 68, 100));
	painter.setPen(QColor(0, 0, 0, 210));
	painter.setBrush(QColor(247, 173, 68, 100));
	painter.drawPolygon(border);
	painter.drawPixmap(point, scaled);
	painter.setPen(Qt::white);
	painter.drawText(labelRect, text(), textOption);
	painter.end();

	//! Grayscaled
	QImage m = __icon.scaled(iconRect.size() * __pixelsRatio, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage();
	scaled = QPixmap::fromImage(::grayscaleImage(m));
	painter.begin(__grayscaled);
	painter.setFont(textFont);
	painter.setPen(QColor(255, 255, 255, 100));
//	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.drawPixmap(point, scaled);
	painter.drawText(labelRect, text(), textOption);
	painter.end();
}


void FancyButton::initIconOnly() {

	QLinearGradient hoveredGrad(QPointF(0, 0), QPointF(0, height()));
	hoveredGrad.setColorAt(0, QColor(255, 255, 255, 20));
	hoveredGrad.setColorAt(.5, QColor(255, 255, 255, 10));
	hoveredGrad.setColorAt(1, QColor(255, 255, 255, 20));

	QPolygon border;
	border << QPoint(0, 0)
	    << QPoint(width() - 2, 0)
	    << QPoint(width() - 2, height() - 2)
	    << QPoint(0, height() - 2);

	QRect iconRect(QPoint(0, 0), QSize(width(), height()));
	QPixmap scaled = __icon.scaled(iconRect.size() * __pixelsRatio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QPoint point((width() / 2) - scaled.size().width() / 2, (height() / 2) - scaled.size().height() / 2);

	QPainter painter;

	//! Normal
	painter.begin(__normal);
	painter.setPen(Qt::white);
	painter.drawPixmap(point, scaled);
	painter.end();

	//! Hovered
	painter.begin(__hovered);
	painter.setPen(QColor(255, 255, 255, 70));
	painter.setBrush(hoveredGrad);
	painter.drawPolygon(border);
	painter.setPen(Qt::white);
	painter.drawPixmap(point, scaled);
	painter.end();

	//! Clicked
	painter.begin(__clicked);
	painter.setPen(QColor(0, 0, 0, 210));
	painter.setBrush(QColor(255, 255, 255, 200));
	painter.drawPolygon(border);
	painter.drawPixmap(point, scaled);
	painter.end();

	//! Blinked
	painter.begin(__blinked);
	painter.setPen(QColor(0, 0, 0, 210));
	painter.setBrush(QColor(247, 173, 68, 100));
	painter.drawPolygon(border);
	painter.drawPixmap(point, scaled);
	painter.end();

	//! Grayscaled
	QImage m = __icon.scaled(iconRect.size() * __pixelsRatio, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage();
	scaled = QPixmap::fromImage(::grayscaleImage(m));
	painter.begin(__grayscaled);
	painter.setPen(QColor(255, 255, 255, 50));
//	painter.setCompositionMode(QPainter::CompositionMode_Source);
//	painter.setOpacity(.2);
	painter.drawPixmap(point, scaled);
	painter.end();
}


void FancyButton::initSeparator() {

	QLinearGradient hoveredGrad(QPointF(0, 0), QPointF(0, height()));
	hoveredGrad.setColorAt(0, QColor(255, 255, 255, 20));
	hoveredGrad.setColorAt(.5, QColor(255, 255, 255, 10));
	hoveredGrad.setColorAt(1, QColor(255, 255, 255, 20));

	QLinearGradient gradient(QPointF(0, 0), QPointF(width(), height()));
	gradient.setColorAt(0, QColor(Qt::white));
	gradient.setColorAt(0.5, QColor(64, 64, 64));
	gradient.setColorAt(0, QColor(Qt::white));

	QPolygon border;
	border << QPoint(0, 0)
	    << QPoint(width() - 2, 0)
	    << QPoint(width() - 2, height() - 2)
	    << QPoint(0, height() - 2);

	//! Normal
	QPainter painter;
	painter.begin(__normal);
	painter.setPen(QColor(255, 255, 255, 70));
	painter.setBrush(hoveredGrad);
	painter.drawPolygon(border);
	painter.end();

	//! Grayscaled
	painter.begin(__grayscaled);
	painter.setPen(QColor(255, 255, 255, 50));
	painter.setBrush(gradient);
	painter.drawLine(QPointF(2, height() / 2), QPointF(width() - 2, height() / 2));
//	painter.drawPolygon(border);
	painter.end();
}


FancyPanel::FancyPanel(QWidget* parent, const Orientation& o,
                       const int& size, const Theme& t) :
		QWidget(parent), __cache(NULL), __orientation(o),
		__selection(SingleLinkedSelection) {

	(__orientation == Vertical) ? setFixedWidth(size) : setFixedHeight(size);
	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	setTheme(t);
}


FancyPanel::~FancyPanel() {
	delete __cache;
	qDeleteAll(__buttons);
}


void FancyPanel::resizeEvent(QResizeEvent* event) {

	Q_UNUSED(event);

	delete __cache; // Remove old cache

	__cache = new QPixmap(size()); // Create a cache with same size as the widget
	__cache->fill(Qt::transparent);  // Create a the transparent background

	QPainter painter(__cache); // Start painting the cache

	QColor gradientStart(__gradientStart);
	QColor gradientEnd(__gradientEnd);
	QPen pen(__pen);
	pen.setWidth(2);

	QLinearGradient linearGrad;
	if ( __orientation == Vertical ) {
		linearGrad = QLinearGradient(QPointF(2, 2), QPointF(width() - 2, 2));
		linearGrad.setColorAt(0, gradientStart);
		linearGrad.setColorAt(1, gradientEnd);
	}
	else {
		linearGrad = QLinearGradient(QPointF(2, 2), QPointF(2, height() - 2));
		linearGrad.setColorAt(0, gradientEnd);
		linearGrad.setColorAt(1, gradientStart);
	}

	//! Title bar's frame
	QPolygon frame;

	frame << QPoint(0, 0)
	    << QPoint(width(), 0)
	    << QPoint(width(), height())
	    << QPoint(0, height());

	painter.setPen(pen);
	painter.setBrush(QBrush(linearGrad));
	painter.drawPolygon(frame);

//	int pos = 0;
	int minSize = 0;
	for (FancyButtonMap::iterator it = __buttons.begin();
	        it != __buttons.end(); ++it) {

		if ( minSize == 0 ) {
			if ( __orientation == Vertical ) {
				(*it)->move(0, 0);
				if ( (*it)->type() == FancyButton::Separator ) {
					(*it)->resize(width(), (*it)->height());
					minSize += (*it)->height();
				}
				else {
					(*it)->resize(width(), width() * .8);
					minSize += width() * .8;
				}
			}
			else {
				(*it)->move(0, 0);
				if ( (*it)->type() == FancyButton::Separator ) {
					(*it)->resize((*it)->width(), height());
					minSize += (*it)->width();
				}
				else {
					(*it)->resize(height() * 1.2, height());
					minSize += height() * 1.2;
				}
			}
		}
		else {
			if ( __orientation == Vertical ) {
				(*it)->move(0, minSize);
//				(*it)->move(0, pos * width() * .8);
				if ( (*it)->type() == FancyButton::Separator ) {
					(*it)->resize(width(), (*it)->height());
					minSize += (*it)->height();
				}
				else {
					(*it)->resize(width(), width() * .8);
					minSize += width() * .8;
				}
			}
			else {
				(*it)->move(minSize, 0);
//				(*it)->move(pos * height() * 1.2 + 2, 0);
				if ( (*it)->type() == FancyButton::Separator ) {
					(*it)->resize((*it)->width(), height());
					minSize += (*it)->width();
				}
				else {
					(*it)->resize(height() * 1.2, height());
					minSize += height() * 1.2;
				}
			}
		}
	}

	(__orientation == Vertical) ? setMinimumSize(QSize(width(), minSize))
	    : setMinimumSize(QSize(minSize, height()));
}


void FancyPanel::paintEvent(QPaintEvent* event) {

	Q_UNUSED(event);

	if ( __cache ) {
		QPainter painter(this);
		painter.drawPixmap(0, 0, *__cache);
	}
}


void FancyPanel::setTheme(const Theme& t) {

	__theme = t;

	if ( t == Dark ) {
//		__gradientStart = QColor(38, 38, 38);
		__gradientStart = QColor(64, 64, 64);
		__gradientEnd = QColor(130, 130, 130);
		__pen = QColor(49, 49, 49);
	}
	else if ( t == Gray ) {
//		__gradientStart = QColor(180, 176, 174, 255);
		__gradientStart = QColor(152, 152, 152, 255);
		__gradientEnd = QColor(213, 208, 206, 255);
		__pen = QColor(31, 31, 31, 255);
	}
	else if ( t == YellowLemon ) {
		__gradientStart = QColor(255, 255, 204, 180);
		__gradientEnd = QColor(255, 255, 204, 100);
		__pen = QColor(255, 255, 204, 210);
	}
	else if ( t == GreenLemon ) {
		__gradientStart = QColor(50, 140, 0, 180);
		__gradientEnd = QColor(50, 140, 0, 100);
		__pen = QColor(50, 140, 0, 210);
	}
}


void FancyPanel::setCustomTheme(const QColor& start, const QColor& end,
                                const QColor& pen) {
	__gradientStart = start;
	__gradientEnd = end;
	__pen = pen;
}


void FancyPanel::addFancyButton(FancyButton* button) {

	SDPASSERT(button);

	for (FancyButtonMap::iterator it = __buttons.begin();
	        it != __buttons.end(); ++it)
		if ( (*it) == button ) {
			qWarning() << "Object " << button << " already in pannel";
			return;
		}

	QString id = genRandomString(4);
	while ( __ids.find(id) != __ids.end() )
		id = genRandomString(4);

	button->setObjectName(id);
	connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
	__buttons.insert(__buttons.count(), button);
}


void FancyPanel::addFancyButton(const QString& text) {
	FancyButton* b = new FancyButton(text, this);
	addFancyButton(b);
}


void FancyPanel::addFancyButton(const QString& text, const QPixmap& icon) {
	FancyButton* b = new FancyButton(icon, text, this);
	addFancyButton(b);
}


void FancyPanel::addFancyButton(const QPixmap& icon) {
	FancyButton* b = new FancyButton(icon, this);
	addFancyButton(b);
}


FancyButton* FancyPanel::button(const QString& name) {

	for (FancyButtonMap::iterator it = __buttons.begin();
	        it != __buttons.end(); ++it)
		if ( (*it)->objectName() == name ) return (*it);

	return NULL;
}


void FancyPanel::activateFancyButton(FancyButton* button) {

	for (FancyButtonMap::iterator it = __buttons.begin();
	        it != __buttons.end(); ++it) {
		(button == (*it)) ? (*it)->setState(FancyButton::Clicked) : (*it)->setState(FancyButton::Normal);
		(*it)->update();
	}
}


void FancyPanel::setSelectionMode(const SelectionMode& sm) {
	__selection = sm;
}


const FancyPanel::SelectionMode& FancyPanel::selectionMode() const {
	return __selection;
}


void FancyPanel::buttonClicked() {

	QObject* sender = QObject::sender();
	SDPASSERT(sender);
	for (FancyButtonMap::iterator it = __buttons.begin();
	        it != __buttons.end(); ++it) {

		if ( sender != (*it) ) {
			if ( (*it)->isCheckable() ) {
				if ( __selection == SingleLinkedSelection )
				    (*it)->setState((!(*it)->isChecked()) ?
				        FancyButton::Normal : FancyButton::Clicked);
			}
			else
				(*it)->setState(FancyButton::Normal);
			(*it)->update();
		}
		else {
			if ( (*it)->isCheckable() ) {
				if ( __selection == SingleFreeSelection ) {
					(*it)->setChecked(!(*it)->isChecked());
					(*it)->setState(((*it)->isChecked()) ?
					    FancyButton::Clicked : FancyButton::Normal);
				}
				(*it)->update();
			}
		}
	}
}



FancyHeaderFrame::FancyHeaderFrame(QWidget* parent, const int& height) :
		QWidget(parent), __cache(NULL) {

	setFixedHeight(height);

	QFont f(font());
#ifdef Q_OS_MAC
	f.setPointSize(16);
#else
	f.setPointSize(14);
#endif
	f.setBold(true);

	__title = new QLabel("-", this);
	__title->setFont(f);

#ifdef Q_OS_MAC
	f.setPointSize(11);
#else
	f.setPointSize(9);
#endif

	__description = new QLabel("-", this);
	__description->setFont(f);

	QPalette palette = __title->palette();
	palette.setColor(__title->foregroundRole(), Qt::white);
	__title->setPalette(palette);
	__description->setPalette(palette);

	QFrame* line = new QFrame(this);
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);

	QVBoxLayout* layout = new QVBoxLayout(this);
//	layout->setMargin(8);
	layout->setSpacing(0);
	layout->addWidget(__title);
	layout->addWidget(line);
	layout->addWidget(__description);
}


FancyHeaderFrame::~FancyHeaderFrame() {
	delete __cache;
}


void FancyHeaderFrame::resizeEvent(QResizeEvent* event) {

	Q_UNUSED(event);

	delete __cache; // Remove old cache

	__cache = new QPixmap(size()); // Create a cache with same size as the widget
	__cache->fill(Qt::transparent); // Create a the transparent background

	QPainter painter(__cache); // Start painting the cache
	QColor gradientStart(64, 64, 64);
	QColor gradientEnd(130, 130, 130);
	QPen pen(QColor(49, 49, 49));
	pen.setWidth(2);

	QLinearGradient linearGrad = QLinearGradient(QPointF(2, 2), QPointF(2, height() - 2));
	linearGrad.setColorAt(0, gradientEnd);
	linearGrad.setColorAt(1, gradientStart);

	QPolygon frame;
	frame << QPoint(0, 0)
	    << QPoint(width(), 0)
	    << QPoint(width(), height())
	    << QPoint(0, height());

	painter.setPen(pen);
	painter.setBrush(QBrush(linearGrad));
	painter.drawPolygon(frame);
}


void FancyHeaderFrame::paintEvent(QPaintEvent* event) {

	Q_UNUSED(event);

	if ( __cache ) {
		QPainter painter(this);
		painter.drawPixmap(0, 0, *__cache);
	}
}


void FancyHeaderFrame::setTitle(const QString& s) {
	__title->setText(s);
}


void FancyHeaderFrame::setDescription(const QString& s) {
	__description->setText(s);
}


ScriptEditor::ScriptEditor(const int& margin, QWidget* parent) :
		QPlainTextEdit(parent), __lineNumberMargin(margin) {

	setFrameShape(QFrame::NoFrame);

	QFont f(font());
	f.setFamily("Monospace");
#ifdef Q_OS_MAC
	f.setFamily("Monaco");
#endif
	f.setFixedPitch(true);

	setFont(f);

	__lineNumberArea = new LineNumberArea(this);

	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
	connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

	updateLineNumberAreaWidth(0);
	highlightCurrentLine();
}


int ScriptEditor::lineNumberAreaWidth() {

	int digits = 1;
	int max = qMax(1, blockCount());
	while ( max >= 10 ) {
		max /= 10;
		++digits;
	}

	int space = __lineNumberMargin + fontMetrics().width(QLatin1Char('9')) * digits;

	return space;
}


void ScriptEditor::updateLineNumberAreaWidth(int newBlockCount) {
	Q_UNUSED(newBlockCount);
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}


void ScriptEditor::updateLineNumberArea(const QRect &rect, int dy) {

	if ( dy )
		__lineNumberArea->scroll(0, dy);
	else
		__lineNumberArea->update(0, rect.y(), __lineNumberArea->width(), rect.height());

	if ( rect.contains(viewport()->rect()) )
	    updateLineNumberAreaWidth(0);
}


void ScriptEditor::resizeEvent(QResizeEvent* event) {
	QPlainTextEdit::resizeEvent(event);
	QRect cr = contentsRect();
	__lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}


void ScriptEditor::highlightCurrentLine() {

	QList<QTextEdit::ExtraSelection> extraSelections;

	if ( !isReadOnly() ) {
		QTextEdit::ExtraSelection selection;
		selection.format.setBackground(QColor(0, 0, 0, 20));
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
	}

	setExtraSelections(extraSelections);
}


void ScriptEditor::lineNumberAreaPaintEvent(QPaintEvent* event) {

	QPainter painter(__lineNumberArea);
	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int) blockBoundingRect(block).height();

	while ( block.isValid() && top <= event->rect().bottom() ) {

		if ( block.isVisible() && bottom >= event->rect().top() ) {
			QString number = QString::number(blockNumber + 1);
			painter.setPen(Qt::black);
			painter.drawText(-(__lineNumberMargin / 2), top,
			    __lineNumberArea->width(), fontMetrics().height(),
			    Qt::AlignRight, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + (int) blockBoundingRect(block).height();
		++blockNumber;
	}
}


FancyLabel::FancyLabel(QWidget* parent) :
		QLabel(parent), __backgroundColor(Qt::black) {

	setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	QFont f(font());
	f.setBold(true);
	setFont(f);

	QPalette palette = this->palette();
	palette.setColor(QPalette::Background, __backgroundColor);
	palette.setColor(QPalette::Foreground, Qt::white);
	setPalette(palette);
	setAutoFillBackground(true);
}


FancyLabel::~FancyLabel() {}


void FancyLabel::setBackgroundColor(const QColor& c) {

	__backgroundColor = c;
	QPalette palette = this->palette();
	palette.setColor(QPalette::Background, __backgroundColor);
	setPalette(palette);
}


const QColor& FancyLabel::backgroundColor() const {
	return __backgroundColor;
}


void FancyLabel::setFontColor(const QColor& c) {

	__fontColor = c;
	QPalette palette = this->palette();
	palette.setColor(QPalette::WindowText, __fontColor);
	setPalette(palette);
}


const QColor& FancyLabel::fontColor() const {
	return __fontColor;
}


} // namespace Qt4
} // namespace SDP

