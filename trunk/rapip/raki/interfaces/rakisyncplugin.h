/***************************************************************************
 * Copyright (c) 2003 Volker Christian <voc@users.sourceforge.net>         *
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
 
#ifndef SYNCPLUGIN_H
#define SYNCPLUGIN_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <librra.h>

#include <qobject.h>
#include <qstring.h>
#include <qevent.h>

/**
@author Volker Christian,,,
*/

class WorkerThreadInterface;
class SyncTaskListItem;
class Rra;

class RakiSyncPlugin : public QObject
{
    Q_OBJECT
public:
    RakiSyncPlugin();

    bool doSync(WorkerThreadInterface *workerThread, ObjectType *objectTypeString, QString pdaName, SyncTaskListItem *progressItem, Rra *rra);
    void incTotalSteps(int inc);
    void decTotalSteps(int dec);
    void advanceProgress();
    void setTotalSteps(int steps);
    void setProgress(int progress);
    void setTask(QString task);
    uint32_t getObjectTypeId();
    bool isRunning();

private:
    virtual bool sync() = 0;
    
    ObjectType *objectType;

protected:
    QString pdaName;
    SyncTaskListItem *progressItem;
    Rra *rra;
    WorkerThreadInterface *workerThread;
};

#endif
