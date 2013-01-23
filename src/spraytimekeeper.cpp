/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "spraytimekeeper.h"
#include <qt4/QtCore/QTimer>
#include <qt4/QtCore/QElapsedTimer>
#include <iostream>
#include "spraytimekeeper.moc"

sprayTimeKeeper::sprayTimeKeeper(void )
{
  this->start();
}


int sprayTimeKeeper::init_spray(int decision, int timeout)
{
  this->keyLock.lock();
  int currentkey;
  if(keyPointer < numBuffers)
     currentkey = keyPointer++;
  else
  {
    keyPointer = 0;
    currentkey = 0;
  }
  this->decisions[currentkey] = decision;
  this->times[currentkey] = zero.elapsed() + timeout;
  this->keyLock.unlock();
  return currentkey;
}

void sprayTimeKeeper::update_spray(int decision, int key)
{
  this->keyLock.lock();
  this->decisions[key] = decision;
  this->keyLock.unlock();
}

int sprayTimeKeeper::get_decision(int key)
{
  this->keyLock.lock();
  int decision = this->decisions[key];
  this->keyLock.unlock();
  return decision;
}

void sprayTimeKeeper::run()
{
  zero.start();
  for(int i=0;i<numBuffers;i++)
  {
    times[i]=2147483640;
  }
    while(true)
    {
      for(int i=0;i<numBuffers;i++)
      {
	if(zero.elapsed() > times[i])
	{
	  emit(spray(decisions[i]));
	  times[i] = 2147483640;
	}
      }
      this->msleep(10);
    }
QThread::run();
}
