/***************************************************************************
 *   Copyright (C) 2003 by Volker Christian,,,                             *
 *   voc@soft.uni-linz.ac.at                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "errorevent.h"


ErrorEvent::ErrorEvent(enum errorTypes errorType, void *data)
    : QCustomEvent(QEvent::User, data)
{
  this->errorType = errorType;
}


ErrorEvent::~ErrorEvent()
{}


int ErrorEvent::getErrorType()
{
  return errorType;
}
