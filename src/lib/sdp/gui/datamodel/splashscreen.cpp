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
#include <sdp/gui/datamodel/splashscreen.h>
#include <sdp/gui/datamodel/utils.h>
#include "ui_splashscreen.h"
#include <QVBoxLayout>
#include <QMovie>


namespace SDP {
namespace Qt4 {


Splashscreen::Splashscreen(QWidget* parent, Qt::WFlags f) :
		QDialog(parent, f), __ui(new Ui::Splashscreen) {

	setModal(true);
	__ui->setupUi(this);

	QMovie* movie = new QMovie(":images/colored.gif", QByteArray(), this);
	movie->setScaledSize(QSize(120, 120));
	__ui->labelAnimation->setMovie(movie);
	__ui->labelAnimation->setAlignment(Qt::AlignCenter);
	movie->start();

	__ui->labelAppName->setText(PROJECT_NAME);
	__ui->labelAppVersion->setText(QString("%1\nv%2").arg(PROJECT_COPYRIGHT).arg(PROJECT_VERSION));
	__ui->labelLogo->setPixmap(QPixmap(":images/Logo_short_RGB_72_OVS_horiz.png").scaled(80, 80));
}


Splashscreen::~Splashscreen() {}


void Splashscreen::show(const QString& info) {
	__ui->labelInfo->setText(info);
	if ( !isVisible() ) QWidget::show();
	Utils::responsiveDelay(10);
}


void Splashscreen::hide() {
	Utils::responsiveDelay(2000);
	QWidget::hide();
}


} // namespace Qt4
} // namespace SDP

