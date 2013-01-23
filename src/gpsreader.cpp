/*
    Copyright (c) 2012, <copyright holder> <email>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY <copyright holder> <email> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL <copyright holder> <email> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "gpsreader.h"
#include "gpsreader.moc"
#include "libs/qextserialport/qextserialenumerator.h"
#include <QList>
#include <QRegExp>
#include <iostream>
#include <QStringList>

gpsReader::gpsReader(QObject* parent): QThread(parent)
{
  /* use to search for correct usb to serial converter
  QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
  for (int i = 0; i < ports.size(); i++) 
  {
        std::cout << "port name:" << ports.at(i).portName.toLocal8Bit().constData() << std::endl;
        std::cout << "friendly name:" << ports.at(i).friendName.toLocal8Bit().constData() << std::endl;
        std::cout << "physical name:" << ports.at(i).physName.toLocal8Bit().constData() << std::endl;
        std::cout << "enumerator name:" << ports.at(i).enumName.toLocal8Bit().constData() << std::endl;
        std::cout << "vendor ID:" << QString::number(ports.at(i).vendorID, 16).toLocal8Bit().constData() << std::endl;
        std::cout << "product ID:" << QString::number(ports.at(i).productID, 16).toLocal8Bit().constData() << std::endl;
        std::cout << "===================================" << std::endl;
    }
    */
  this->structureState = GPS_STATE_SearchSOS; //Set initial state
  
  this->talker_id.resize(3);; //Talker id (gps, loran-c etc.)
  this->talker_id[2] = 0; //Proper string termination ensured
  
  this->receivedChecksum.reserve(3);
  this->receivedChecksum[2] = 0; //Proper string termination ensured
  
  this->address = new QByteArray; //Initilize address space
  this->address->reserve(32); //No need for constant realloc's
  this->address->fill(0);
  
  this->data = new QList<QByteArray>;
  this->data->reserve(77); //As a message can be of maximum 82 characters and $,<CR>,<NL> and talker id  
                           //removes the first 5 characetrs this is more than enough for any compliant message
  this->dataWord = new QByteArray; //One word of dataWord
  this->dataWord->reserve(77);
  
  this->isGGA = new QByteArrayMatcher("GGA");
  
  stateMachineLock = new QMutex();
  stateMachineLock->unlock();
  
  satellites = new SatList;
  satellites->reserve(32);
  Satellite blank;
  blank.healthy = false;
  blank.used = false;
  blank.snr = 0;
  blank.azm = 0;
  blank.ele = 0;
  blank.prn = -1;
  for(int i=1;i<=32;i++)
  {
    satellites->insert(i, blank);
  }
  
  connect(this, SIGNAL(newNMEASentence(QByteArray,QByteArray,QList<QByteArray>)), this, SLOT(processNewNMEA(QByteArray,QByteArray,QList<QByteArray>)));
  
  connect(this, SIGNAL(newGGA(QByteArray,QByteArray,char,QByteArray,char,int,int,float,float,char,QByteArray,char,float,int)),
          this, SLOT(GGATest(QByteArray,QByteArray,char,QByteArray,char,int,int,float,float,char,QByteArray,char,float,int))
         );
  
  connect(this, SIGNAL(newRMC(QByteArray,char,QByteArray,char,QByteArray,char,float,float,QByteArray,float,char)),
          this, SLOT(RMCTest(QByteArray,char,QByteArray,char,QByteArray,char,float,float,QByteArray,float,char))
         );
  
  connect(this, SIGNAL(newGSA(char,char,QList<int>,float,float,float)), SLOT(GSATest(char,char,QList<int>,float,float,float)));
  
  connect(this, SIGNAL(newSubGSV(int,int,int,QList<int>,QList<int>,QList<int>,QList<int>)), SLOT(GSVCollector(int,int,int,QList<int>,QList<int>,QList<int>,QList<int>)));
  this->start();
}

void gpsReader::run()
{
  PortSettings settings;
  settings.BaudRate = BAUD115200;
  settings.DataBits = DATA_8;
  settings.FlowControl = FLOW_OFF;
  settings.Parity = PAR_NONE;
  settings.StopBits = STOP_1;
  settings.Timeout_Millisec = 250;
  buffer = new QByteArray();
  serial = new QextSerialPort("/dev/RTK-GPS", settings, QextSerialPort::EventDriven);
  connect(this->serial, SIGNAL(readyRead()), this, SLOT(newDataAvailable()));
  serial->open(QIODevice::ReadOnly);  
  QThread::run();
}


void gpsReader::newDataAvailable(void )
{
    int avail = serial->bytesAvailable();
    if( avail > 0 ) {
        QByteArray data;
        data.resize(avail);
        int read = serial->read(data.data(), data.size());
        if( read > 0 ) {
            stateMachine(data);
        }
    }
}



void gpsReader::processNewNMEA(QByteArray talker, QByteArray command, QList< QByteArray > arg)
{
  //As all known non-vendor specific commands is 3 bytes, we convert it to and int for cheap comparison
  int icommand;
  if(command.count()!=3) //This is aparently a unknown commands
    return;
  icommand = *((int*)(command.constData())) & 0x00ffffff;
 // std::cout << std::endl << icommand << ":" << command.constData();
  switch (icommand)
  {
    case 4278087: //GGA - Global Positioning System Fix Data
        if(receivers(SIGNAL(newGGA(QByteArray,QByteArray,char,QByteArray,char,int,int,float,float,char,QByteArray,char,float,int))) > 0)
          emit(newGGA(arg[0], //Time
                      arg[1], //Latitude
                      arg[2].at(0), //Latitude heading
                      arg[3], //Longitude
                      arg[4].at(0), //Longitude heading
                      arg[5].toInt(0, 10), //GPS Quality
                      arg[6].toInt(0, 10), //Number of sats in view
                      arg[7].toFloat(), //Horizontal dilution
                      arg[8].toFloat(), //altitude
                      arg[9].at(0), //Unit of altitude
                      arg[10], //Geoidal seperation
                      arg[11].at(0), //Geoidal seperation unit
                      arg[12].toFloat(), //dGPS age
                      arg[13].toInt(0,10) //dGPS station
                    )); 
        break;
    case 4410706: //RMC - Recommend Minimum Specific GPS/TRANSIT Data
      if(receivers(SIGNAL(newRMC(QByteArray,char,QByteArray,char,QByteArray,char,float,float,QByteArray,float,char))) > 0)
        emit(newRMC(arg[0], //Time
                    arg[1].at(0), //Status
                    arg[2], //Latitude
                    arg[3].at(0), //Latitude heading
                    arg[4], //Longitude
                    arg[5].at(0), //Longitude heading
                    arg[6].toFloat(), //Speed over ground
                    arg[7].toFloat(), //Track made good
                    arg[8], //Date
                    arg[9].toFloat(), //Magnetic variation
                    arg[10].count()>0 ? arg[10].at(0) : '0'//Magnetic variation heading
        ));
        break;
    case 4281159: //GSA - GPS DOP and active satellites
        if(receivers(SIGNAL(newGSA(char,char,QList<int>,float,float,float))) > 0)
        {
          QList<int> sats;
          sats.reserve(12);
          for(int i=2;i<14;i++)
            sats.push_back(arg[i].toInt());
          emit(newGSA(arg[0].at(0), //Selection mode
                      arg[1].at(0), //Mode
                      sats,         //ID of satellites used for fix
                      arg[14].toFloat(), //Position dilution
                      arg[15].toFloat(), //Horizontal dilution
                      arg[16].toFloat()  //Vertical dilution
                   )
            );
        }
        break;
    case 5657415: //GSV - Satellites in view
        if(
          (receivers(SIGNAL(newSubGSV(int,int,QList<int>,QList<int>,QList<int>,QList<int>)))>1) || (receivers(SIGNAL(satellitesUpdated(SatList)))>0))
        {
          QList<int> prn, elevation, azimuth, snr;
          for(int i=3;i<arg.count();i+=4)
          {
            prn.push_back(arg[i].toInt());
            elevation.push_back(arg[i+1].toInt());
            azimuth.push_back(arg[i+2].toInt());
            snr.push_back(arg[i+3].toInt());
          }
          emit(newSubGSV(arg[0].toInt(), //Number of messages
                         arg[1].toInt(), //Message nr
                         arg[2].toInt(), //Number of satellites in view
                         prn, //satellite PRN number 
                         elevation, //elevation in degrees (00-90) (leading zeros sent)
                         azimuth, //azimuth in degrees to true north (000-359) (leading zeros sent)
                         snr //SNR in dB (00-99)
          ));
        }
        break;
    case 4674646: //VTG - Velocity made good
      if(receivers(SIGNAL(newVTG(QByteArray, QByteArray,
	      QByteArray, QByteArray,
	      QByteArray, QByteArray,
	      float, QByteArray))) > 0)
        emit(newVTG(arg[0], //True track made good
                    arg[1], //True track made good indicator
                    arg[2], //Magnetic track made good
                    arg[3], //Magnetic track made good indicator
                    arg[4], //Ground speed in knots
                    arg[5], //Ground speed in knots indicator
                    arg[6].toFloat(), //Ground speed in kmh
                    arg[7] //Ground speed in kmh unit
        ));
	break;
    default:
      break;
  }
}


void gpsReader::GGATest(QByteArray time, QByteArray latitude, char latitudeHeading, 
                        QByteArray longitude, char longitudeHeading, int GPSQuality, 
                        int sattelitesInView, float horizontalDilution, float altitude, 
                        char altitudeUnit, QByteArray geoidalSeperation, 
                        char geoidalSeperationUnit, float dGPSAge, int dGPSStation)
{
// std::cout << std::endl << "Received GGA at " << time.constData() << " GPS Quality is " << GPSQuality;
// std::cout << " Dilution is " << horizontalDilution << " and we have " << sattelitesInView << " Sats in view";
}

void gpsReader::RMCTest(QByteArray time, char status, QByteArray latitude, char latitudeHeading, 
                        QByteArray longitude, char longitudeHeading, float speedOverGround, 
                        float trackMadeGood, QByteArray date, float magneticVariation, 
                        char magneticVariationUnit)
{
//std::cout << std::endl << "Received RMC at " << time.constData() << " GPS status is " << status;
//std::cout << " Date is " << date.constData();
}

void gpsReader::GSATest(char selMode, char mode, QList< int > satellites, float pDOP, float hDOP, float vDOP)
{
  for(int i=0;i<satellites.count();i++)
  {
    int prn = satellites.at(i);
    if(prn!=0)
    {
      Satellite sat = this->satellites->at(prn-1);
      if(sat.used==false)
      {
        sat.used = true;
        this->satellites->replace(prn-1, sat);
      }
    }
  }
}

void gpsReader::GSVCollector(int numMessages, int messageNum, int satsInView, QList< int > prn, QList< int > elevation, QList< int > azimuthh, QList< int > snr)
{
  for(int i=0;i<prn.count();i++)
  {
    if(prn[i] != 0)
    {
//      std::cout << std::endl << "prn:" << prn[i];
      Satellite sat;
      sat.azm = azimuthh[i];
      sat.ele = elevation[i];
      sat.prn = prn[i];
      sat.snr = snr[i];
      sat.used = satellites->at(sat.prn-1).used;
      sat.healthy = satellites->at(sat.prn-1).healthy;
      satellites->replace(sat.prn-1, sat);
    }
  }
  emit(satellitesUpdated(*satellites));
}


void gpsReader::stateMachine(QByteArray data)
{
  this->stateMachineLock->lock();
  for(int i=0;i<data.size();i++)
  {
//    std::cout << data.at(i);
      switch (this->structureState)
      {
        case GPS_STATE_SearchSOS:
          if(data.at(i) == '$') //If we received a start character
          {
//            std::cout << std::endl << "Found SOS, moving to retrieve talker" << std::endl << std::flush;
            this->structureState = GPS_STATE_RetrieveTalker_1;
            this->calcChecksum = 0;
          }
          break;
        case GPS_STATE_RetrieveTalker_1:
          talker_id[0] = data.at(i);
          this->calcChecksum ^= data.at(i);
//          std::cout << std::endl << "Moving to retrieve talker 2" << std::endl << std::flush;
          this->structureState = GPS_STATE_RetrieveTalker_2;
          break;
        case GPS_STATE_RetrieveTalker_2:
          talker_id[1] = data.at(i);
          this->address->clear();
//          std::cout << std::endl << "Moving to retrieve Address" << std::endl << std::flush;
          this->calcChecksum ^= data.at(i);
          this->structureState = GPS_STATE_RetrieveAddress;
          break;
        case GPS_STATE_RetrieveAddress:
          this->calcChecksum ^= data.at(i);
          if(data.at(i) == ',')
          {
//            std::cout << std::endl << "Moving to retrieve Data" << std::endl << std::flush;
            this->data->clear();
            this->structureState = GPS_STATE_RetrieveData;
          }
          else
          {
            this->address->push_back(data.at(i));
          }
          break;
        case GPS_STATE_RetrieveData:
          if(data.at(i) == '*')
          {
//            std::cout << std::endl << "Moving to checksum verification" << std::endl << std::flush;
            this->data->push_back(*(this->dataWord));
            this->dataWord->clear();
            this->structureState = GPS_STATE_RetrieveChecksum_1;
          }
          else if(data.at(i) == ',')
          {
            this->calcChecksum ^= data.at(i);
            this->data->push_back(*(this->dataWord));
            this->dataWord->clear();
          }
          else
          {
            this->calcChecksum ^= data.at(i);
            this->dataWord->push_back(data.at(i));
          }
          break;
        case GPS_STATE_RetrieveChecksum_1:
          this->receivedChecksum[0] = data.at(i);
//          std::cout << std::endl << "Moving to checksum verification 2" << std::endl << std::flush;
          this->structureState = GPS_STATE_RetrieveChecksum_2;
          break;
        case GPS_STATE_RetrieveChecksum_2:
//          std::cout << std::endl << "Starting over" << std::endl;
          this->receivedChecksum[1] = data.at(i);
          //this->receivedChecksum = (data.at(i)-0x30)*10 + (this->receivedChecksum-0x30); //atoi
          //TODO: Verify the checksum here
          //this->receivedChecksum.toUInt(0, 16);
          //std::cout << std::endl << "read Checksum:" << this->receivedChecksum.toUInt(0, 16) << " calculated checksum:" << (int)(this->calcChecksum) << std::endl << std::flush;
          if(this->receivedChecksum.toUInt(0, 16) == (this->calcChecksum)) //If checksum verified
          {
            emit(newNMEASentence(this->talker_id, *(this->address), *(this->data)));
          }
          this->structureState = GPS_STATE_SearchSOS;
          break;  
        default: //Should not be possible
          this->structureState = GPS_STATE_SearchSOS;
      }
  }
  this->stateMachineLock->unlock();
}

