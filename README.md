# IMSI dissector
A tool which enables to dissect an IMSI number in order to retreive below information :
- MCC (Mobile Country Code)
- MNC (Mobile Network Code)
- MSIN (Mobile Subscription Identification Number)
- NETWORK (Local mobile operator)
- COUNTRY (Country name)
- REGION (Geographic region name)

IMSI means **International Mobile Subscriber Identity**

For more details about IMSI, you can take a look here => [IMSI](https://en.wikipedia.org/wiki/International_mobile_subscriber_identity)

## Requirements
`libxml2-devel`

On CentOS 7, you can install the package with the following command line :
`yum install libxml2-devel`

## Clone and Build
```Shell Session
git clone git@github.com:mellonguie/imsi_dissector.git
cd imsi_dissector/
make # to build in release mode
make DEBUG=1 # to build in debug mode
make clean # to clean the program
make mrproper # to clean the entire program
```

## Usage
```Shell Session
./imsi_dissector 310150123456789
```

## Output results
```Shell Session
MCC;MNC;MSIN;NETWORK;COUNTRY;REGION
310;15;0123456789;Unknown;United States;North America and the Caribbean
310;150;123456789;AT&T Wireless Inc.;United States;North America and the Caribbean
```
