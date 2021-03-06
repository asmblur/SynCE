/***************************************************************************
 * Copyright (c) 2003 Volker Christian <voc@users.sourceforge.net>         *
 *                    Christian Fremgen <cfremgen@users.sourceforge.net>   *
 *                                                                         *
 * Permission is hereby granted, free of charge, to any person obtaining a *
 * copy of this software and associated documentation files (the           *
 * "Software"), to deal in the Software without restriction, including     *
 * without limitation the rights to use, copy, modify, merge, publish,     *
 * distribute, sublicense, and/or sell copies of the Software, and to      *
 * permit persons to whom the Software is furnished to do so, subject to   *
 * the following conditions:                                               *
 *                                                                         *
 * The above copyright notice and this permission notice shall be included *
 * in all copies or substantial portions of the Software.                  *
 *                                                                         *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF              *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY    *
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,    *
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE       *
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                  *
 ***************************************************************************/

#include "syncelocalkonnectorconfig.h"

#include "syncelocalkonnector.h"

#include <libkcal/resourcelocal.h>

#include <kconfig.h>
#include <klocale.h>
#include <kabc/resourcefile.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <klineedit.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

using namespace KSync;

SynCELocalKonnectorConfig::SynCELocalKonnectorConfig( QWidget *parent, const char *name )
        : SynCEKonnectorConfigBase( parent, name )
{
    QBoxLayout * topLayout = new QVBoxLayout( this );

    topLayout->addWidget( new QLabel( i18n( "Calendar file:" ), this ) );

    mCalendarFile = new KURLRequester( this );
    mCalendarFile->setMode( KFile::File | KFile::LocalOnly );
    topLayout->addWidget( mCalendarFile );

    QPushButton *button =
        new QPushButton( i18n( "Select From Existing Calendars..." ), this );
    connect( button, SIGNAL( clicked() ), SLOT( selectCalendarResource() ) );
    topLayout->addWidget( button );

    topLayout->addSpacing( 4 );

    topLayout->addWidget( new QLabel( i18n( "Address book file:" ), this ) );

    mAddressBookFile = new KURLRequester( this );
    mAddressBookFile->setMode( KFile::File | KFile::LocalOnly );
    topLayout->addWidget( mAddressBookFile );

    button = new QPushButton( i18n( "Select From Existing Address Books..." ), this );
    connect( button, SIGNAL( clicked() ), SLOT( selectAddressBookResource() ) );
    topLayout->addWidget( button );
}

SynCELocalKonnectorConfig::~SynCELocalKonnectorConfig()
{}

void SynCELocalKonnectorConfig::loadSettings( KRES::Resource *r )
{
    SynCELocalKonnector * konnector = dynamic_cast<SynCELocalKonnector *>( r );
    if ( konnector ) {
        mCalendarFile->setURL( konnector->calendarFile() );
        mAddressBookFile->setURL( konnector->addressBookFile() );
    }
}

void SynCELocalKonnectorConfig::saveSettings( KRES::Resource *r )
{
    SynCELocalKonnector * konnector = dynamic_cast<SynCELocalKonnector *>( r );
    if ( konnector ) {
        konnector->setCalendarFile( mCalendarFile->url() );
        konnector->setAddressBookFile( mAddressBookFile->url() );
    }
}

void SynCELocalKonnectorConfig::selectAddressBookResource()
{
    QStringList files;

    KRES::Manager<KABC::Resource> manager( "contact" );
    manager.readConfig();

    KRES::Manager<KABC::Resource>::Iterator it;
    for ( it = manager.begin(); it != manager.end(); ++it ) {
        if ( ( *it ) ->inherits( "KABC::ResourceFile" ) ) {
            KABC::ResourceFile * r = static_cast<KABC::ResourceFile *>( *it );
            files.append( r->fileName() );
        }
    }

    if ( files.isEmpty() ) {
        KMessageBox::sorry( this, i18n( "No file resources found." ) );
    } else {
        QString file = KInputDialog::getItem( i18n( "Select File" ),
                                              i18n( "Please select an addressbook file:" ), files, 0, false, 0, this );
        if ( !file.isEmpty() ) {
            mAddressBookFile->lineEdit() ->setText( file );
        }
    }
}

void SynCELocalKonnectorConfig::selectCalendarResource()
{
    QStringList files;

    KCal::CalendarResourceManager manager( "calendar" );
    manager.readConfig();

    KCal::CalendarResourceManager::Iterator it;
    for ( it = manager.begin(); it != manager.end(); ++it ) {
        if ( ( *it ) ->inherits( "KCal::ResourceLocal" ) ) {
            KCal::ResourceLocal * r = static_cast<KCal::ResourceLocal *>( *it );
            files.append( r->fileName() );
        }
    }

    if ( files.isEmpty() ) {
        KMessageBox::sorry( this, i18n( "No file resources found." ) );
    } else {
        QString file = KInputDialog::getItem( i18n( "Select File" ),
                                              i18n( "Please select a calendar file:" ), files, 0, false, 0, this );
        if ( !file.isEmpty() ) {
            mCalendarFile->lineEdit() ->setText( file );
        }
    }
}

void SynCELocalKonnectorConfig::enableRaki()
{}

#include "syncelocalkonnectorconfig.moc"
