/*
   Copyright 2012 Last.fm Ltd.
      - Primarily authored by Michael Coffey

   This file is part of the Last.fm Desktop Application Suite.

   lastfm-desktop is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   lastfm-desktop is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with lastfm-desktop.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "lib/unicorn/widgets/Label.h"

#include "LicensesDialog.h"
#include "ui_LicensesDialog.h"

LicensesDialog::LicensesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LicensesDialog)
{
    ui->setupUi(this);

    setAttribute( Qt::WA_DeleteOnClose );

    QString licenseText;
    licenseText.append( "<div id=\"content\">" );

    licenseText.append( "<h3>Third-Party Licenses</h3>" );
    licenseText.append( "<p>The Last.fm Desktop App wouldn't be such a thing of beauty without the frameworks and software listed below. A big shoutout from everyone at Last.HQ, and a tip-of-the-hat to all the developers involved in bringing these great open source projects to life.</p>" );

    // Qt
    licenseText.append( "<hr>" );
    licenseText.append( "<h3>Qt</h3>" );
    licenseText.append( "<ul>" );
    licenseText.append( "<li>" + unicorn::Label::anchor( "http://qt.nokia.com", "Qt" ) + "</li>");
    licenseText.append( "<li>" + unicorn::Label::anchor( "http://qt.nokia.com/products/licensing/", "Qt License" ) + "</li>");
    licenseText.append( "<li>" + unicorn::Label::anchor( "http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html", "GNU Lesser General Public License (LGPL) version 2.1" ) + "</li>");
    licenseText.append( "</ul>" );

#ifdef Q_OS_MAC
    // Sparkle (updating framework)
    licenseText.append( "<hr>" );
    licenseText.append( "<h3>Sparkle</h3>" );
    licenseText.append( "<ul>" );
    licenseText.append( "<li>" + unicorn::Label::anchor( "http://sparkle.andymatuschak.org/", "Sparkle" ) + "</li>");
    licenseText.append( "<li>" + unicorn::Label::anchor( "https://github.com/andymatuschak/Sparkle/blob/master/License.txt", "Sparkle License" ) + "</li>");
    licenseText.append( "</ul>" );
    licenseText.append( "<p>Copyright (c) 2006 Andy Matuschak</p>" );
    licenseText.append( "<p>Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the \"Software\"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:</p>" );
    licenseText.append( "<p>The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.</p>" );
    licenseText.append( "<p>THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.</p>" );
#endif

#ifdef Q_OS_WIN
    // Winsparkle
    licenseText.append( "<hr>" );
    licenseText.append( "<h3>winsparkle</h3>" );
    licenseText.append( "<ul>" );
    licenseText.append( "<li>" + unicorn::Label::anchor( "https://github.com/vslavik/winsparkle/blob/master/COPYING", "Winsparkle License" ) + "</li>" );
    licenseText.append( "</ul>" );
    licenseText.append( "<p>Copyright (c) 2009-2010 Vaclav Slavik</p>" );
    licenseText.append( "<p>Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the \"Software\"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:</p>" );
    licenseText.append( "<p>The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.</p>" );
    licenseText.append( "<p>THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.</p>" );
#endif

#ifdef Q_OS_MAC
    // Growl
    licenseText.append( "<hr>" );
    licenseText.append( "<h3>Growl</h3>" );
    licenseText.append( "<ul>" );
    licenseText.append( "<li>" + unicorn::Label::anchor( "http://growl.info", "Growl" ) + "<li>" );
    licenseText.append( "<li>" + unicorn::Label::anchor( "http://growl.info/documentation/developer/bsd-license.txt", "Growl License" ) + "<li>" );
    licenseText.append( "</ul>" );
    licenseText.append( "<p>Copyright (c) The Growl Project, 2004</p>" );
    licenseText.append( "<p>All rights reserved.</p>" );
    licenseText.append( "<p>Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:</p>" );
    licenseText.append( "<ol>" );
    licenseText.append( "<li>Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.</li>" );
    licenseText.append( "<li>Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.</li>" );
    licenseText.append( "<li>Neither the name of Growl nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.</li>" );
    licenseText.append( "</ol>" );
    licenseText.append( "<p>THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.</p>" );
#endif
    // Breakpad
    licenseText.append( "<hr>" );
    licenseText.append( "<h3>Breakpad</h3>" );
    licenseText.append( "<ul>" );
    licenseText.append( "<li>" + unicorn::Label::anchor( "http://code.google.com/p/google-breakpad/", "Breakpad" ) + "<li>" );
    licenseText.append( "<li>" + unicorn::Label::anchor( "http://code.google.com/p/google-breakpad/source/browse/trunk/COPYING", "Breakpad License" ) + "<li>" );
    licenseText.append( "</ul>" );
    licenseText.append( "<p>Copyright (c) 2006, Google Inc.</p>" );
    licenseText.append( "<p>All rights reserved.</p>" );
    licenseText.append( "<p>Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:</p>" );
    licenseText.append( "<ol>" );
    licenseText.append( "<li>Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.</li>" );
    licenseText.append( "<li>Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.</li>" );
    licenseText.append( "<li>Neither the name of Google Inc. nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.</li>" );
    licenseText.append( "</ol>" );
    licenseText.append( "<p>THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.</p>" );

    licenseText.append( "</div>" );

    licenseText = QString( "<html><head><style type=text/css>\n"
                     "#content {margin: 20px;}"
                     "a:link {color:%1; font-weight: bold; text-decoration:none;}"
                     "a:hover {color:%1; font-weight: bold; text-decoration:none;}"
                     "</style></head><body>%2</body></html>" ).arg( QColor( 0xd71005 ).name(), licenseText );

    ui->textBrowser->setText( licenseText );
}

LicensesDialog::~LicensesDialog()
{
    delete ui;
}
