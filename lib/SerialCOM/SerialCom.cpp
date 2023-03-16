#include "SerialCom.h"

shared_ptr<serial_port> SerialPort::serialPort = nullptr;
char SerialPort::receiveData[] = {0};



SerialPort::SerialPort() :
    portName("/dev/ttyUSB0"),
    baudRate(115200)
{
    memset(receiveData,0,sizeof (receiveData));
}
 
SerialPort::~SerialPort()
{
    if(serialPort == nullptr)
        serialPort->close();
}
 
bool SerialPort::init(string port_name, uint baud_rate)
{
    portName = port_name;
    baudRate = baud_rate;
 
    return open();
}
 
void SerialPort::runService()
{
    startAsyncRead();
    io.run();
}
 
bool SerialPort::open()
{
    try
    {
        if(serialPort == nullptr)
            serialPort = shared_ptr<serial_port>(new serial_port(io));
 
        serialPort->open(portName,errorCode);
 
        //设置串口参数
        serialPort->set_option(serial_port::baud_rate(baudRate));
        serialPort->set_option(serial_port::flow_control());
        serialPort->set_option(serial_port::parity());
        serialPort->set_option(serial_port::stop_bits());
        serialPort->set_option(serial_port::character_size(8));
 
        return true;
 
    }
    catch (exception& err)
    {
        cout << "Exception Error: " << err.what() << endl;
    }
 
    return false;
}
 
void SerialPort::startAsyncRead()
{
    memset(receiveData,0,sizeof (receiveData));
    serialPort->async_read_some(boost::asio::buffer(receiveData),
            boost::bind(handleRead,
            boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
}
 
void SerialPort::close()
{
    serialPort->close();
}

void SerialPort::write(float a,float b)// boost::system::error_code &ec
{

    string sent;
    a = a*100;
    b = b*100;
     int s1,s2;

     s1 = (int)a;
     s2 = (int)b;
     sent = "*"+to_string(s2)+";"+"#"+to_string(s1)+"%";
    //char sent[sendlength];


    //
    // s1 = (int)(b*100);
    // s2 = (int)(a*100);
    // cout<<"s1:"<<s1<<"s2:"<<s2<<endl;

    //  union Buffer{
    //     int data[(sendlength-2)/4];
    //     char char_data[(sendlength-2)];
    // }buffer2;

    // buffer2.data[0] = s1;
    // buffer2.data[1] = s2;

    // sent[0] = '*';
    // sent[9] = ';';

    // for (int i = 1;i < sendlength-1;i++)
    // {
    //     sent[i] = buffer2.char_data[i-1];
    // }
    // stringstream ss,s;
    // ss<<this->yaw_now;
    // ss>>sent1;
    // s<<this->pitch_now;
    // s>>sent2;

    //cout<<"send_1:"<<sent1<<"send2:"<<sent2<<endl;
    cout<<"sent:"<<sent<<endl;
    
    serialPort->write_some(boost::asio::buffer(sent));
}

void SerialPort::handleRead(const boost::system::error_code &ec,size_t byte_read)
{

	union Buffer{
		short int data[(receivelength - 2)/2];	// 2 byte
		char char_data[(receivelength - 2)];	// 1 byte
	}buffer1;

	for(int j = 0;j < (receivelength - 2);j++)
		buffer1.char_data[j]=receiveData[j+1];
	if( receiveData[1] != 0 )
	{
		SerialPort::yaw_now = buffer1.data[0] / 100.0;
		SerialPort::pitch_now = buffer1.data[1] / 100.0;
	}	
	startAsyncRead();
}
