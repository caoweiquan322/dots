/* Copyright © 2015 DynamicFatty. All Rights Reserved. */

#include "DotsException.h"
#include<QDebug>

DotsException::DotsException(QString message) throw()
{
    this->message = message;
}

DotsException::DotsException(const DotsException &e) throw()
{
    this->message = e.message;
}

DotsException::~DotsException() throw()
{

}

QString DotsException::getMessage()
{
    return this->message;
}

