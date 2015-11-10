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



#ifndef __SDP_QT4_Splashscreen_H__
#define __SDP_QT4_Splashscreen_H__


#include <sdp/gui/datamodel/singleton.h>
#include <QWidget>
#include <QDialog>

namespace Ui {
class Splashscreen;
}


namespace SDP {
namespace Qt4 {


class Splashscreen : public QDialog, public Singleton<Splashscreen> {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit Splashscreen(QWidget* = NULL, Qt::WFlags = 0);
		~Splashscreen();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void show(const QString&);
		void hide();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QScopedPointer<Ui::Splashscreen> __ui;
};


} // namespace Qt4
} // namespace SDP

#endif
