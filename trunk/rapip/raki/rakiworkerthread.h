/***************************************************************************
 *   Copyright (C) 2003 by Volker Christian,,,                             *
 *   voc@soft.uni-linz.ac.at                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _RakiWorkerThread_H_
#define _RakiWorkerThread_H_

#include <qthread.h>
#include <klistbox.h>

#include "workerthreadinterface.h"


/**
 * 
 * Volker Christian,,,
 **/

class RakiWorkerThread : public QThread
{

protected:
    RakiWorkerThread();
    ~RakiWorkerThread();
    
public:
    void run();
    void start(WorkerThreadInterface *wti, 
        enum WorkerThreadInterface::threadType type = (enum WorkerThreadInterface::threadType) 0);
    void stop();
    
private:
    WorkerThreadInterface *wti;
    
public:
    static RakiWorkerThread *rakiWorkerThread;
};

#endif
