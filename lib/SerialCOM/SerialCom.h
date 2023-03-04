#ifndef SERIALCOM_H
#define SERIALCOM_H

#include <iostream>
#include <iomanip>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <memory>
#include <thread>
#include <sstream>
using namespace std;
using namespace boost::asio;
 

#define receivelength 12	//*******按照收数据(short int)的数目进行修改,具体为 (消息个数)*2+2
#define sendlength 10	//*******按照收发数据(int)的数目进行修改,具体为 (消息个数)*4+2

typedef u_int8_t any_type;
 
class SerialPort
{
public:
    SerialPort();
    ~SerialPort();

	static double pitch_now, yaw_now;
	bool init(string port_name, uint baud_rate);
	void runService();
	bool open();
	void close();
	void write(float a,float b);

	static void startAsyncRead();
	static void handleRead(const boost::system::error_code &ec,size_t byte_read);
	
private:
	boost::system::error_code errorCode;
	io_service io;
	static shared_ptr<serial_port> serialPort;
	string portName;
	uint baudRate;
 
	static char receiveData[1024];
};

#endif // SERIALCOM#
