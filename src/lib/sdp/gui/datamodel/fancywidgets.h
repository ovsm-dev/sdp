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




#ifndef __SDP_QT4_FANCYWIDGETS_H__
#define __SDP_QT4_FANCYWIDGETS_H__


#include <QWidget>
#include <QAbstractButton>
#include <QPixmap>
#include <QMap>
#include <QColor>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QProcess>
#include <QLabel>


QT_FORWARD_DECLARE_CLASS(QTimer);


namespace SDP {
namespace Qt4 {


/**
 * @class FancyButton
 * @brief This class implements a custom button. This button supports basic
 *        QAbstractButton features and some custom ones like animation.
 */
class FancyButton : public QAbstractButton {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum FancyButtonType {
			TextOnly, IconText, IconOnly, Separator
		};
		enum FancyButtonState {
			Normal, Hovered, Clicked, Blinked
		};
		enum FancyButtonCapability {
			Interactive, Clickable, Dead,
			Standard = Interactive | Clickable

		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit FancyButton(QWidget* parent = NULL);
		explicit FancyButton(const QString&, QWidget* parent = NULL);
		explicit FancyButton(const QPixmap&, QWidget* parent = NULL);
		explicit FancyButton(const QPixmap&, const QString&, QWidget* parent = NULL);
		~FancyButton();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void resizeEvent(QResizeEvent*);
		void paintEvent(QPaintEvent*);
		void enterEvent(QEvent*);
		void leaveEvent(QEvent*);
		void mousePressEvent(QMouseEvent*);

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void setActivated(const bool&);
		void animate(const int&);
		void redraw();

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void refresh();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		const FancyButtonType& type() const;
		void setState(const FancyButtonState&);
		const FancyButtonState& buttonState() const;
		void setButtonCapability(const FancyButtonCapability&);
		const FancyButtonCapability& buttonCapability() const;

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		void initPixmaps();
		void initPixmap(QPixmap**);
		void initTextOnly();
		void initIconText();
		void initIconOnly();
		void initSeparator();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QPixmap* __normal;
		QPixmap* __hovered;
		QPixmap* __clicked;
		QPixmap* __blinked;
		QPixmap* __grayscaled;
		QTimer* __timer;
		QPixmap __icon;

		FancyButtonType __type;
		FancyButtonState __state;
		FancyButtonCapability __capabitlity;
		bool __activated;
		int __animationCounter;
		qreal __pixelsRatio;
};


/**
 * @class FancyPanel
 * @brief This class implements a widget for storing and managing FancyButtons.
 *        It supports vertical and horizontal positioning,
 */
class FancyPanel : public QWidget {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum Orientation {
			Vertical, Horizontal
		};
		typedef QMap<size_t, FancyButton*> FancyButtonMap;
		typedef QMap<QString, QString> IDMap;

		enum Theme {
			Dark, Gray, YellowLemon, GreenLemon
		};

		enum SelectionMode {
			SingleLinkedSelection, SingleFreeSelection
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit FancyPanel(QWidget* parent = NULL, const Orientation& = Vertical,
		                    const int& size = 80, const Theme& = Dark);
		~FancyPanel();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void resizeEvent(QResizeEvent*);
		void paintEvent(QPaintEvent*);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setTheme(const Theme&);
		const Theme& theme() const {
			return __theme;
		}
		void setCustomTheme(const QColor&, const QColor&, const QColor&);
		void addFancyButton(FancyButton*);
		void addFancyButton(const QString&);
		void addFancyButton(const QString&, const QPixmap&);
		void addFancyButton(const QPixmap&);
		FancyButton* button(const QString&);
		void activateFancyButton(FancyButton*);
		void setSelectionMode(const SelectionMode&);
		const SelectionMode& selectionMode() const;

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void buttonClicked();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void buttonClicked(const QString&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QPixmap* __cache;
		FancyButtonMap __buttons;
		Orientation __orientation;
		SelectionMode __selection;
		IDMap __ids;
		Theme __theme;
		QColor __gradientStart;
		QColor __gradientEnd;
		QColor __pen;
};


/**
 * @class FancyHeaderFrame
 * @brief This class implements a custom frame header displaying title and
 *        description for the active section (button clicked) of SDP.
 */
class FancyHeaderFrame : public QWidget {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit FancyHeaderFrame(QWidget* parent = NULL, const int& height = 60);
		~FancyHeaderFrame();

	protected:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		void resizeEvent(QResizeEvent*);
		void paintEvent(QPaintEvent*);

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		void setTitle(const QString&);
		void setDescription(const QString&);

	private:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		QPixmap* __cache;
		QLabel* __title;
		QLabel* __description;
};


class LineNumberArea;

/**
 * @class ScriptEditor
 * @brief This class implements a simple script editor.
 */
class ScriptEditor : public QPlainTextEdit {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit ScriptEditor(const int& margin = 10, QWidget* parent = NULL);

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void resizeEvent(QResizeEvent*);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void lineNumberAreaPaintEvent(QPaintEvent*);
		int lineNumberAreaWidth();

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void updateLineNumberAreaWidth(int newBlockCount);
		void highlightCurrentLine();
		void updateLineNumberArea(const QRect&, int);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QWidget* __lineNumberArea;
		int __lineNumberMargin;
};


class LineNumberArea : public QWidget {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit LineNumberArea(ScriptEditor* editor) :
				QWidget(editor) {
			__codeEditor = editor;
		}

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void paintEvent(QPaintEvent* event) {
			__codeEditor->lineNumberAreaPaintEvent(event);
		}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		QSize sizeHint() const {
			return QSize(__codeEditor->lineNumberAreaWidth(), 0);
		}

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		ScriptEditor* __codeEditor;
};


class FancyLabel : public QLabel {

	Q_OBJECT

	Q_PROPERTY (QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
	Q_PROPERTY (QColor fontColor READ fontColor WRITE setFontColor)

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit FancyLabel(QWidget* = NULL);
		~FancyLabel();

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void setBackgroundColor(const QColor&);
		const QColor& backgroundColor() const;

		void setFontColor(const QColor&);
		const QColor& fontColor() const;

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QColor __backgroundColor;
		QColor __fontColor;
};


} // namespace Qt4
} // namespace SDP
#endif
