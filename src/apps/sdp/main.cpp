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


#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include "mainwindow.h"
#include <sdp/gui/datamodel/errorhandler.h>


int main(int argc, char* argv[]) {

	qInstallMsgHandler(errorHandler);
	QApplication app(argc, argv);

	/*
	QFont f;
#ifdef Q_OS_MAC
	f.setFamily("Monaco");
#else
//	f.setFamily("Courier");
	f.setPointSize(9);
#endif
	app.setFont(f);
	*/

	QString translatorFileName = QLatin1String("qt_");
	translatorFileName += QLocale::system().name();
	QTranslator* translator = new QTranslator(&app);
	if ( translator->load(translatorFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)) )
	    app.installTranslator(translator);

	SDP::Qt4::SeismicDataPlayback sdp;
	sdp.show();

	return app.exec();
}
