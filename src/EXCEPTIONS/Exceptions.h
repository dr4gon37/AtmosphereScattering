#pragma once
#include <exception>
#include <string>
#include <iostream>
#include <utility>

class MyException : public std::exception
{
public:
	MyException(std::string msg) : m_msg(std::move(msg))
	{
		std::cout << "MyException::MyException - set m_msg to:" << m_msg << std::endl;
	}

	~MyException()
	{
		std::cout << "MyException::~MyException" << std::endl;
	}

	virtual const char* what() const throw ()
	{
		return m_msg.c_str();
	}
private:
	const std::string m_msg;
};
