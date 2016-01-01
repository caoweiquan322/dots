/* Copyright © 2015 DynamicFatty. All Rights Reserved. */

#ifndef DOTSEXCEPTION_H
#define DOTSEXCEPTION_H

#include<QException>
#include<QString>

/**
 * @brief The DotsException class defines an exception used by this package. It contains a member describing cause of
 * this exception.
 */
class DotsException : public QException
{
public:
    /**
     * @brief DotsException is the default constructor.
     * @param message describes cause of this exception.
     */
    DotsException(QString message) throw();

    /**
     * @brief DotsException is the copy constructor.
     * @param e is the DotsException instance to copy.
     */
    DotsException(const DotsException &e) throw();

    /**
     * @brief ~DotsException is the default deconstructor.
     */
    ~DotsException() throw();

    /**
     * @brief getMessage retrieves the cause of this exception.
     * @return cause of this exception.
     */
    QString getMessage();

    /**
     * @brief raise just inherits from QException to complement the definition.
     */
    void raise() const { throw *this; }

    /**
     * @brief clone just inherits from QException to complement the definition.
     * @return the copied instance of this object.
     */
    DotsException *clone() const { return new DotsException(*this); }

protected:
    /**
     * @brief message stores cause of this exception. It would be shown to users to understand what happened before
     * the app crashed.
     */
    QString message;
};

#endif // DOTSEXCEPTION_H
