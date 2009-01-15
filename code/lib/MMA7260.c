/* *********************************************************
	MMA7260 Accelerometer Library. 
	Must include the MMA7260.h header file.  
	X_CHANNEL, Y_CHANNEL, and Z_CHANNEL must be defined
	in an external header file that is included
	
	Library contains the following functions:
	
	int MMA_get_x(void);
	int MMA_get_y(void);
	int MMA_get_z(void);
	void MMA_change_mode(char mode);
	
	Written by Ryan Owens - 2/1/2008
  ********************************************************* */

//********************************************************************
//
//                  Header files
//
//********************************************************************
#include "MMA7260.h"
#include "LPC214x.h"
#include "MP3Dev.h"

//********************************************************************
//
//				"Private" Function Definitions
//
//********************************************************************
static int get_adc_1(int channel);


/**********************************************************
                       Functions
**********************************************************/

//Usage: x_accel = MMA_get_x();
//Inputs: None
//Outputs: None
//Description: Returns an integer value corresponding to the acceleration
//             of the x-axis of the MMA7260 accelerometer.
int MMA_get_x(void)
{
    int value;
    value = get_adc_1(X_CHANNEL);
    return value;
}

//Usage: y_accel = MMA_get_y();
//Inputs: None
//Outputs: None
//Description: Returns an integer value corresponding to the acceleration
//             of the y-axis of the MMA7260 accelerometer.
int MMA_get_y(void)
{
    int value;
    value = get_adc_1(Y_CHANNEL);
    return value;
}

//Usage: z_accel = MMA_get_z();
//Inputs: None
//Outputs: None
//Description: Returns an integer value corresponding to the acceleration
//             of the z-axis of the MMA7260 accelerometer.
int MMA_get_z(void)
{
    int value;
    value = get_adc_1(Z_CHANNEL);
    return value;
}

//Usage: accel = get_adc_1(X_CHANNEL);
//Inputs: int channel - integer corresponding to the ADC channel to be converted
//Outputs: None
//Description: Returns the raw analog to digital conversion of the input channel.  
//**NOTE** Because this function is static, it is essentially a "private" function
//         and can only be used within this file!
static int get_adc_1(int channel)
{
    int val;
    AD1CR = 0;
    AD1GDR = 0;

    AD1CR = 0x00200600 | channel;
    AD1CR |= 0x01000000;
    do
    {
        val = AD1GDR;                   // Read A/D Data Register
    }
    while ((val & 0x80000000) == 0);  //Wait for the conversion to complete
    val = ((val >> 6) & 0x03FF);  //Extract the A/D result

    return val;
}

//Usage: MMA_change_mode(RANGE_2G);
//Inputs: char mode - The mode to be set (Modes are defined in MMA7260.h)
//Outputs: None
//Description: Sets the sensitivity of the MMA7260 accelerometer based on
//             the input "mode" value.
void MMA_change_mode(char mode){

    /*
	Setup the MMA g-select
	GS1/GS2
	0/0 = 1.5g
	1/0 = 2g
	0/1 = 4g
	1/1 = 6g
	*/
	IODIR0|=(GS1 | GS2);
	if(mode==RANGE_15G)	IOCLR0=(GS1 | GS2);
	else if(mode==RANGE_2G){
		IOSET0=GS1;
		IOCLR0=GS2;
	}
	else if(mode==RANGE_4G){
		IOCLR0=GS1;
		IOSET0=GS2;
	}
	else if(mode==RANGE_6G)IOSET0=(GS1 | GS2);
}

