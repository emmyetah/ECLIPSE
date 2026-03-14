#include "SerialTelemetrySource.h"

#include <QtSerialPort/QSerialPort> //opens and reads serial port (COM port)
#include <QString> //serial port uses string for names
#include <QByteArray> // for raw serial bytes.
#include <QDebug>

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
        qDebug() << "Trying to open port:" << QString::fromStdString(_cfg.portName);
        qDebug() << "Trying baud rate:" << static_cast<qint32>(_cfg.baudRate);

        _impl->port.setPortName(QString::fromStdString(_cfg.portName));
        _impl->port.setBaudRate(static_cast<qint32>(_cfg.baudRate));
        _impl->port.setDataBits(QSerialPort::Data8); //reccomended features from here down
        _impl->port.setParity(QSerialPort::NoParity); 
        _impl->port.setStopBits(QSerialPort::OneStop);
        _impl->port.setFlowControl(QSerialPort::NoFlowControl);

        //ReadOnly is enough for telemetry input
        _impl->rxBuffer.clear();//Clear any old buffered bytes on open
        const bool ok = _impl->port.open(QIODevice::ReadWrite);
        
        
        qDebug() << "QSerialPort open ok:" << ok;
        qDebug() << "QSerialPort error:" << _impl->port.errorString();

        if (ok) {
            _impl->port.setDataTerminalReady(true);
            _impl->port.setRequestToSend(false);

            qDebug() << "Port readable:" << _impl->port.isReadable();
            qDebug() << "Port writable:" << _impl->port.isWritable();
            qDebug() << "DTR now:" << _impl->port.isDataTerminalReady();
            qDebug() << "RTS now:" << _impl->port.isRequestToSend();
            qDebug() << "Bytes available after open:" << _impl->port.bytesAvailable();

            qDebug() << "Waiting up to 3000 ms for first bytes...";
            const bool ready = _impl->port.waitForReadyRead(3000);
            qDebug() << "waitForReadyRead result:" << ready;
            qDebug() << "Bytes available after wait:" << _impl->port.bytesAvailable();

            if (_impl->port.bytesAvailable() > 0) {
                const QByteArray firstChunk = _impl->port.readAll();
                qDebug() << "Immediate raw bytes after wait:" << firstChunk;
                _impl->rxBuffer.append(firstChunk);
            }
        }
        
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
        if (!isOpen()) {
            qDebug() << "Serial port is not open";
            return std::nullopt;
        }

        // If we do not already have a full line buffered, wait briefly for more data.
        if (_impl->rxBuffer.indexOf('\n') < 0 && _impl->port.bytesAvailable() == 0) {
            const bool ready = _impl->port.waitForReadyRead(300);
            qDebug() << "waitForReadyRead in pollLine:" << ready;
        }

        qDebug() << "Bytes available before readAll:" << _impl->port.bytesAvailable();

        const QByteArray chunk = _impl->port.readAll();

        qDebug() << "Bytes read this tick:" << chunk.size();
        if (!chunk.isEmpty()) {
            qDebug() << "Chunk raw:" << chunk;
            _impl->rxBuffer.append(chunk);
        }

        const int nl = _impl->rxBuffer.indexOf('\n');
        if (nl < 0) {
            qDebug() << "No newline found yet";
            return std::nullopt;
        }

        QByteArray lineBytes = _impl->rxBuffer.left(nl);
        _impl->rxBuffer.remove(0, nl + 1);

        if (!lineBytes.isEmpty() && lineBytes.endsWith('\r')) {
            lineBytes.chop(1);
        }

        qDebug() << "Complete line extracted:" << lineBytes;
        return lineBytes.toStdString();
    }
}