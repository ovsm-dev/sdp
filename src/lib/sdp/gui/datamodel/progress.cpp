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



#include "../api.h"
#include <sdp/gui/datamodel/progress.h>
#include <sdp/gui/datamodel/utils.h>
#include "ui_progress.h"
#include <QVBoxLayout>
#include <QMovie>
#include <QThread>


namespace SDP {
namespace Qt4 {


Progress::Progress(QWidget* parent, Qt::WFlags f) :
		QDialog(parent, f), __ui(new Ui::ProgressDialog) {

	setModal(true);
	this->thread()->setPriority(QThread::HighPriority);

	__ui->setupUi(this);

	QMovie* movie = new QMovie(":images/loader2.gif", QByteArray(), this);
	movie->setScaledSize(QSize(100, 100));
	__ui->labelAnim->setMovie(movie);
	__ui->labelAnim->setAlignment(Qt::AlignCenter);
	movie->start();
}


Progress::~Progress() {}


void Progress::show(const QString& info) {
	__ui->labelInfo->setText(info);
	if ( !isVisible() ) QWidget::show();
	Utils::responsiveDelay(100);
}


void Progress::hide() {
	Utils::responsiveDelay(1000);
	QWidget::hide();
}


} // namespace Qt4
} // namespace SDP

