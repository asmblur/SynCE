//
// C++ Implementation: pimsyncmanager
//
// Description:
//
//
// Author: Christian Fremgen; Volker Christian <cfremgen@users.sourceforge.net>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "pimsyncmanager.h"

#include <synceengine.h>
#include <kitchensync/multisynk/konnectorpair.h>
#include "syncekonnectorbase.h"
#include "konnectormanager.h"
#include "paireditordialog.h"
#include <kconfig.h>

#include <klocale.h>

QMap<QString, PimSyncManager *> PimSyncManager::pimSyncMap;

PimSyncManager::PimSyncManager(QString pdaName ) : konnectorsLoaded( false ), pair(NULL), pdaName( pdaName )
{}


PimSyncManager::~PimSyncManager()
{
    if ( konnectorsLoaded ) {
        delete mEngine;
        mEngine = 0;
    }

    if (pair) {
        delete pair;
    }

    pimSyncMap.erase( pdaName );
}


PimSyncManager *PimSyncManager::self(QString pdaName )
{
    if ( !pimSyncMap[ pdaName ] ) {
        pimSyncMap[ pdaName ] = new PimSyncManager(pdaName );
    }

    return pimSyncMap[ pdaName ];
}


void PimSyncManager::setActualSyncType(int type)
{
    if (pair) {
        KonnectorManager * pmanager = pair->manager();
        KonnectorManager::Iterator kit;
        for ( kit = pmanager->begin(); kit != pmanager->end(); ++kit ) {
            kdDebug( 2120 ) << "Trying to subscribe konnector" << endl;
            KSync::SynCEKonnectorBase *k = dynamic_cast<KSync::SynCEKonnectorBase *>( *kit );
            if ( k ) {
                kdDebug( 2120 ) << "Yes, konnector from type SynCEKonnectorBase ... set actual synctype" << pdaName << endl;
                k->actualSyncType( type );
            }
        }
    }
}


void PimSyncManager::subscribeTo( int type )
{
    if (pair) {
        KonnectorManager * pmanager = pair->manager();
        KonnectorManager::Iterator kit;
        for ( kit = pmanager->begin(); kit != pmanager->end(); ++kit ) {
            kdDebug( 2120 ) << "Trying to subscribe konnector" << endl;
            KSync::SynCEKonnectorBase *k = dynamic_cast<KSync::SynCEKonnectorBase *>( *kit );
            if ( k ) {
                kdDebug( 2120 ) << "Yes, konnector from type SynCEKonnectorBase ... subscribe and write pdaName " << pdaName << endl;
                k->setPdaName(pdaName);
                k->setPairUid(pair->uid());
                k->subscribeTo( type );
            }
        }
        pair->save();
    }
}


void PimSyncManager::unsubscribeFrom( int type )
{
    if (pair) {
        KonnectorManager * pmanager = pair->manager();
        KonnectorManager::Iterator kit;
        for ( kit = pmanager->begin(); kit != pmanager->end(); ++kit ) {
            KSync::SynCEKonnectorBase *k = dynamic_cast<KSync::SynCEKonnectorBase *>( *kit );
            if ( k ) {
                k->unsubscribeFrom( type );
            }
        }
        pair->save();
    }
}


bool PimSyncManager::loadKonnectors( KConfig* ksConfig)
{
    if ( !konnectorsLoaded ) {
        ksConfig->setGroup("Pim Synchronizer");
        QString pairUid = ksConfig->readEntry( "PairUid", "---" );
        pair = new KonnectorPair();
        if ( pairUid != "---" ) {
            pair->setUid( pairUid );
            pair->load();
            kdDebug(2120) << "Debug: Pair-Manager: " << ( void * ) pair->manager() << endl;
        } else {
            PairEditorDialog pairEditorDialog(0, "PairEditorDialog", pdaName);
            pairEditorDialog.setPair(pair);
            pair->load();
            kdDebug(2120) << "Debug: Pair-Manager: " << ( void * ) pair->manager() << endl;
        }
        mEngine = new KSync::SynCEEngine();

        konnectorsLoaded = true;
    }

    return true;
}


void PimSyncManager::startSync()
{
    kdDebug( 2120 ) << "Debug: Pair-Manager: " << ( void * ) pair->manager() << " - Engine: " << ( void * ) mEngine << endl;
    connect( pair->manager(), SIGNAL( synceesRead( KSync::Konnector* ) ),
             mEngine, SLOT( slotSynceesRead( KSync::Konnector* ) ) );
    connect( pair->manager(), SIGNAL( synceeReadError( KSync::Konnector* ) ),
             mEngine, SLOT( slotSynceeReadError( KSync::Konnector* ) ) );
    connect( pair->manager(), SIGNAL( synceesWritten( KSync::Konnector* ) ),
             mEngine, SLOT( slotSynceesWritten( KSync::Konnector* ) ) );
    connect( pair->manager(), SIGNAL( synceeWriteError( KSync::Konnector* ) ),
             mEngine, SLOT( slotSynceeWriteError( KSync::Konnector* ) ) );
    connect( mEngine, SIGNAL( doneSync() ),
             this, SLOT( syncDone() ) );

    mEngine->go( pair );
}


void PimSyncManager::syncDone()
{
    kdDebug( 2120 ) << "Debug: Pair-Manager: " << ( void * ) pair->manager() << " - Engine: " << ( void * ) mEngine << endl;
    disconnect( pair->manager(), SIGNAL( synceesRead( KSync::Konnector* ) ),
                mEngine, SLOT( slotSynceesRead( KSync::Konnector* ) ) );
    disconnect( pair->manager(), SIGNAL( synceeReadError( KSync::Konnector* ) ),
                mEngine, SLOT( slotSynceeReadError( KSync::Konnector* ) ) );
    disconnect( pair->manager(), SIGNAL( synceesWritten( KSync::Konnector* ) ),
                mEngine, SLOT( slotSynceesWritten( KSync::Konnector* ) ) );
    disconnect( pair->manager(), SIGNAL( synceeWriteError( KSync::Konnector* ) ),
                mEngine, SLOT( slotSynceeWriteError( KSync::Konnector* ) ) );
    disconnect( mEngine, SIGNAL( doneSync() ),
                this, SLOT( syncDone() ) );

    pair->save();
}


void PimSyncManager::configure(QWidget *parent, KConfig* ksConfig)
{
    PairEditorDialog pairEditorDialog(parent, "PairEditorDialog", pdaName);

    KonnectorPair *tmpPair;

    if (pair) {
        tmpPair = pair;
    } else {
        tmpPair = new KonnectorPair();
    }

    pairEditorDialog.setPair( tmpPair );

    kdDebug(2120) << "PairEditorDialog exec" << endl;
    if (pairEditorDialog.exec()) {
        ksConfig->setGroup("Pim Synchronizer");
        tmpPair = pairEditorDialog.pair();
        ksConfig->writeEntry( "PairUid", tmpPair->uid());
        ksConfig->sync();
        pair = tmpPair;
        pair->save();
        kdDebug(2120) << "Debug: Pair-Manager: " << ( void * ) pair->manager() << endl;
    } else if (!pair) {
        kdDebug(2120) << "Delete tmpPair" << endl;
        delete tmpPair;
        tmpPair = NULL;
    }
}


#include "pimsyncmanager.moc"
