#include "SerialTelemetrySource.h"

#include <QtSerialPort/QSerialPort> //opens and reads serial port (COM port)
#include <QString> //serial port uses string for names
#include <QByteArray> // for raw serial bytes.

namespace eclipse::io
{
    //hidden implementation
    struct SerialTelemetrySource::Impl
    {
        QSerialPort port;
        QByteArray rxBuffer;
    };

    //constructor, cfg copies config (port name, baud rate etc) into _cfg
    SerialTelemetrySource::SerialTelemetrySource(const SerialTelemetryConfig& cfg)
        : _cfg(cfg)
    {   //allocates hidden implementation object on the heap. 
        _impl = new Impl();
    }

    //deconstuctor
    SerialTelemetrySource::~SerialTelemetrySource()
    {
        //ensures seial port is closed and buffer is cleared,
        close();
        //frees the heap memory allocated in the constructor
        delete _impl;
        //prevents accidental use after deletion.
        _impl = nullptr;
    }

    
    bool SerialTelemetrySource::open()
    {
        //build a new implementation, if it's null
        if (!_impl) _impl = new Impl();

        //if it's already open don't reopen it, return success immediately. 
        if (_impl->port.isOpen()) {
            return true;
        }

        //set config data.
        _impl->port.setPortName(QString::fromStdString(_cfg.portName));
        _impl->port.setBaudRate(static_cast<qint32>(_cfg.baudRate));
        _impl->port.setDataBits(QSerialPort::Data8); //reccomended features from here down
        _impl->port.setParity(QSerialPort::NoParity); 
        _impl->port.setStopBits(QSerialPort::OneStop);
        _impl->port.setFlowControl(QSerialPort::NoFlowControl);

        //ReadOnly is enough for telemetry input
        const bool ok = _impl->port.open(QIODevice::ReadOnly);

        //Clear any old buffered bytes on open
        _impl->rxBuffer.clear();

        return ok;
    }

    //closing a port and resetting buffer.
    void SerialTelemetrySource::close()
    {
        if (!_impl) return;

        if (_impl->port.isOpen()) {
            _impl->port.close();
        }
        _impl->rxBuffer.clear();
    }
    //checks if port is open and returns true or false.
    bool SerialTelemetrySource::isOpen() const
    {
        return (_impl && _impl->port.isOpen());
    }

    //turns a stream of raw serial bytes into one clean complee telemetry CSV line.
    std::optional<std::string> SerialTelemetrySource::pollLine()
    {
        //checks if port is open. If not open, no line to read.
        if (!isOpen()) return std::nullopt;

        //no blocking, reading what is available now.
        const QByteArray chunk = _impl->port.readAll();
        //if there is nothing to read append new bytes to buffer
        if (!chunk.isEmpty()) {
            _impl->rxBuffer.append(chunk);
        }

        //Look for a newline in the buffer
        const int nl = _impl->rxBuffer.indexOf('\n');
        //if position of new line is less than 0 return nullopt (object that doesn't contian a value)
        if (nl < 0) {
            return std::nullopt; // no complete line yet
        }

        //extract one full line (up to '\n')
        QByteArray lineBytes = _impl->rxBuffer.left(nl);
        _impl->rxBuffer.remove(0, nl + 1); //+1 to remove '\n'

        //Handle Windows-style "\r\n"
        if (!lineBytes.isEmpty() && lineBytes.endsWith('\r')) {
            lineBytes.chop(1);
        }

        //Convert to std::string
        return lineBytes.toStdString();
    }
}