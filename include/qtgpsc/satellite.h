#ifndef SATELLITE_H
#define SATELLITE_H
#include <QList>
//! Support class for Gpsd. Holds position and status data for one satellite
class Satellite {
    
    public:
        
        Satellite();
        
        Satellite(int prn, double azm, double ele,
                  double snr, bool used, bool healthy);
                  
    public:
        int prn;
        double azm, ele, snr;
        bool used, healthy;
};

//! Convenience type to hold a list of satellites
typedef QList<Satellite> SatList;
#endif