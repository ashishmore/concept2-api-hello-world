//
//  main.cpp
//  c2-mac-hello
//
//  Created by Sam Halperin on 7/16/13.
//  Copyright (c) 2013 Sam Halperin. All rights reserved.
//

#include <iostream>
#include "PM3DDICP.h"
#include "PM3CSafeCP.h"
#include "PM3USBCP.h"
#include "csafe.h"
#include <unistd.h>

using namespace std;

/* Sanity test:  If all of our dependencies are in the
 right place, then this will print a numeric DLL version. */
void dllVersion() {
	UINT16_T version;
	version =  tkcmdsetDDI_get_dll_version();
	cout << "dll version: " << version << endl;
}


/* Many of the tkcmdset... functions will set
 a numeric error code.  This function will
 convert that to a human readable value.
 */
void printErrorCodeInfo(ERRCODE_T ecode) {
	char name[40];
	char text[200];
    
	tkcmdsetDDI_get_error_name(ecode,name,sizeof(name));
	tkcmdsetDDI_get_error_text(ecode,text,sizeof(text));
	
	cout << "error: " << name << ": " << text << endl;
}

/* Currently discovers PM4's.  Prints the number connected.
 TODO: discover PM3's... (see PMSDKDemoDlg.cpp in the C2 demo project for reference)
 */
bool ergsConnected() {
	ERRCODE_T ecode = 0;
	UINT16_T numCommunicating = 0;
	UINT16_T numPM4Devices = 0;
    
	ecode = tkcmdsetDDI_discover_pm3s(TKCMDSET_PM4_PRODUCT_NAME, numCommunicating, &numCommunicating);
	
	if (ecode) {
		cout << "ecode (discovery): " << ecode << endl;
		printErrorCodeInfo(ecode);
	} else if (numCommunicating) {
		numPM4Devices = numCommunicating;
		cout << "num PM4's: " << numPM4Devices << endl;
		return true;
	} else {
		cout << "No PM4's found." << endl;
	}
    
	return false;
}

/* Another sanity test.  Can we talk to the erg and get its
 serial number? */
void serialNumber(UINT16_T port) {
	char rspBuffer[32];
	UINT8_T serialLength = 16;
	ERRCODE_T ecode = 0;
    
	ecode = tkcmdsetDDI_serial_number(port, rspBuffer, serialLength);
	if (ecode) {
		cout << "ecode (serial number): " << ecode << endl;
		printErrorCodeInfo(ecode);
	} else {
		cout << "Serial number: " << rspBuffer << endl;
	}
}


// After CSAFE has been intialized (see main below)
// get the per-stroke power output.
// also see http://www.c2forum.com/viewtopic.php?f=15&t=4667
void printPower(UINT16_T port)
{
	UINT16_T rsp_size = 100, cmd_size = 1;
	UINT32_T rsp_data[100], cmd_data[1];
	cmd_data[0] = 0xB4;
	ERRCODE_T ecode = 0;
	int POWER_LSB = 2;  // The position of the least significant byte of the watt data.
	int POWER_MSB = 3;  // and the most significant byte.
    
	ecode = tkcmdsetCSAFE_command(port, cmd_size, cmd_data, &rsp_size, rsp_data);
    
	if (ecode) {
		printErrorCodeInfo(ecode);
	} else {
		// I believe the correct formula here is that power = (MSB * 256) + LSB.
		// So MSB is usually going to be 0, but might be 1 if Xeno Muller is pulling a 500.
		UINT32_T watts = (rsp_data[POWER_MSB] * 256) +   rsp_data[POWER_LSB];
		cout << "Power = " << watts << endl;
	}
}


int main(int argc, const char * argv[])
{

    UINT16_T port = 0;
	tkcmdsetDDI_init();
	dllVersion();
	
	if (ergsConnected()) {
		tkcmdsetCSAFE_init_protocol(1000);
		serialNumber(port);
		for (int i = 0; i< 2400; i++) {
			printPower(port);
			sleep(1);  // the data is once per stroke, so faster is not going to give much beter rez.
		}
	}
	cout << "done" << endl;
	getchar();
	return 0;
}

