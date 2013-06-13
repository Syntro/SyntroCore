//
//  Copyright (c) 2012 Pansenti, LLC.
//	
//  This file is part of SyntroLib
//
//  SyntroLib is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  SyntroLib is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with SyntroLib.  If not, see <http://www.gnu.org/licenses/>.
//

#include "SyntroThread.h"
#include "LogWrapper.h"

/*!
    \class SyntroThread
    \brief SyntroThread is a lightweight wrapper for QThread.
	\inmodule SyntroLib
 
	SyntroThread is a thin wrapper layer around Qt’s QThread. It provides a 
	Windows-style message passing queue and associated functions. The 
	normal sequence of events is to create a SyntroThread, perform any external 
	initialization that might be required and then to call resumeThread(). 
	To kill the thread, call exitThread().

	SyntroThread also processes QThread timer events and maps them to SyntroThread messages with 
	a message code of SYNTROTHREAD_TIMER_MESSAGE. These can be handled in the processMessage() 
	function. The timer ID is passed in the intParam of the message to allow multiple timers to be supported.

	SyntroThread would normally be subclassed to provide customized thread functionality.
*/

/*!
	\internal
*/

SyntroThreadMsg::SyntroThreadMsg(QEvent::Type nEvent) : QEvent(nEvent)
{
}

/*!
	This is the constructor for SyntroThread. It may be called with \a threadName, a 
	name for the thread – this can be useful for debugging.
*/

SyntroThread::SyntroThread(QString threadName) : QThread()
{
	m_run = false;
	m_name = threadName;
	m_event = QEvent::registerEventType();					// get an event number
}

/*!
	\internal
*/

SyntroThread::~SyntroThread()
{
}

/*!
	Calling the constructor for SyntroThread doesn’t actually start the thread 
	running to allow for the caller to perform any necessary initialization.
	resumeThread() should be called when everything is ready for the thread to run. 
	resumeThread() calls initThread() and then start(), to get the threads event loop running.
*/

void SyntroThread::resumeThread()
{
	initThread();
	start();
}

/*!
	\internal
*/


void SyntroThread::run()
{
	installEventFilter(this);
	m_run = true;
	exec();	
}

/*!
	isRunning() will return true if the thread is still running, false if it has exited the run loop.
*/

bool SyntroThread::isRunning()
{
	return m_run;
}

/*!
	intiThread() is called as part of the resumeThread() processing and should be used for 
	any initialization that the thread needs to perform prior to normal execution.
*/

void SyntroThread::initThread()
{
}

/*!
	exitThread() should be called when the component wishes to shutdown the thread prior to exit. 
	This function should be overriden to perform any clean up that the thread requires. 
	The default exitThread() provided by SyntroThread includes these two lines:

	\list
    \li m_run = false;
    \li exit();
	\endlist

	If this function is overridden, these two lines should be added to the overriding function.
*/

void SyntroThread::exitThread()
{
	m_run = false;
	exit();
}

/*!
	processMessage is called when there is a message to be processed on the thread’s message queue. 
	\a msg contains the message. This function should be overridden in order to process the messages. 
	The function should return true if the messages was processed by the function and no further 
	processing should be performed on this message, false if not.
*/

bool SyntroThread::processMessage(SyntroThreadMsg* msg)
{
	logDebug(QString("Message on default PTM - %1").arg(msg->type()));
	return true;
}

/*!
	This function can be called to place a message for thread on its queue. A message consists of three values:

	\list
	\li \a message. This is a code that identifies the message. See SyntroThread.h for examples.
	\li \a intParam. This is an integer parameter that is passed with the message.
	\li \a ptrParam. This is a pointer parameter that is passed with the message.
	\endlist

	Control is returned to the caller immediately as this function just queues the message for later processing by the thread.
*/

void SyntroThread::postThreadMessage(int message, int intParam, void *ptrParam)
{
	if (!m_run)
		return;

	SyntroThreadMsg *msg = new SyntroThreadMsg((QEvent::Type)m_event);
	msg->message = message;
	msg->intParam = intParam;
	msg->ptrParam = ptrParam;
	qApp->postEvent(this, msg);
}

/*!
	\internal
*/

bool SyntroThread::eventFilter(QObject *obj, QEvent *event)
 {
	 if (!m_run)
		 return QObject::eventFilter(obj, event);

	 if (event->type() == m_event) {
		processMessage((SyntroThreadMsg *)event);
		return true;
	}
 
	if (event->type() == QEvent::Timer) {
		SyntroThreadMsg *msg = new SyntroThreadMsg((QEvent::Type)m_event);
		msg->message = SYNTROTHREAD_TIMER_MESSAGE;
		msg->intParam = ((QTimerEvent *)event)->timerId();
		processMessage((SyntroThreadMsg *)msg);
		delete msg;
		return true;
	}

	//	Just do default processing 
    return QObject::eventFilter(obj, event);
 }
