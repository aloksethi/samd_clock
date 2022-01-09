/*******************************************************************************
 System Tasks File

  File Name:
    tasks.c

  Summary:
    This file contains source code necessary to maintain system's polled tasks.

  Description:
    This file contains source code necessary to maintain system's polled tasks.
    It implements the "SYS_Tasks" function that calls the individual "Tasks"
    functions for all polled MPLAB Harmony modules in the system.

  Remarks:
    This file requires access to the systemObjects global data structure that
    contains the object handles to all MPLAB Harmony module objects executing
    polled in the system.  These handles are passed into the individual module
    "Tasks" functions to identify the instance of the module to maintain.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "configuration.h"
#include "definitions.h"


// *****************************************************************************
// *****************************************************************************
// Section: RTOS "Tasks" Routine
// *****************************************************************************
// *****************************************************************************
/* Handle for the APP_Tasks. */
TaskHandle_t xAPP_Tasks;
TaskHandle_t xI2C_Tasks;

void _USB_DEVICE_Tasks(  void *pvParameters  )
{
    while(1)
    {
				 /* USB Device layer tasks routine */
        USB_DEVICE_Tasks(sysObj.usbDevObject0);
                DRV_USBFSV1_Tasks(sysObj.drvUSBFSV1Object);

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

//void _DRV_USBFSV1_Tasks(  void *pvParameters  )
//{
//    while(1)
//    {
//				 /* USB FS Driver Task Routine */
//        vTaskDelay(10 / portTICK_PERIOD_MS);
//    }
//}

void _APP_Tasks(  void *pvParameters  )
{   

    while(1)
    {
        APP_Tasks();
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

#define APP_SENSOR_I2C_CLOCK_SPEED			   100000
#define APP_SENSOR_I2C_SLAVE_ADDR			   0x0068

typedef struct
{
	bool isInitDone;
	DRV_HANDLE i2cHandle;
	DRV_I2C_TRANSFER_SETUP i2cSetup;
	uint8_t i2cRxBuffer[1];
} APP_SENSOR_THREAD_DATA;

APP_SENSOR_THREAD_DATA app_sensorData;


void I2C_Task(  void *pvParameters  )
{   
    bool ret_val;
    	uint8_t registerAddr = 0;

    app_sensorData.i2cHandle = DRV_I2C_Open( DRV_I2C_INDEX_0, DRV_IO_INTENT_READWRITE );
    	if (app_sensorData.i2cHandle != DRV_HANDLE_INVALID)
		{
			/* Got valid handle, now configure the I2C clock speed for sensor */
			app_sensorData.i2cSetup.clockSpeed = APP_SENSOR_I2C_CLOCK_SPEED;

			/* Setup I2C transfer @ 100 kHz to interface with Sensor */
            DRV_I2C_TransferSetup(app_sensorData.i2cHandle, &app_sensorData.i2cSetup);
			
		}
		else
		{
			/* Handle error */
			vTaskSuspend(NULL);
		}
    
    
    
    while(1)
    {

        led2_Clear();
    #if 1
        uint8_t control_val = 0;
        uint8_t status_val = 0; 
        uint8_t second_val = 0;
        
        registerAddr = 0xe;
        ret_val =  DRV_I2C_WriteReadTransfer(app_sensorData.i2cHandle, APP_SENSOR_I2C_SLAVE_ADDR, (void*)&registerAddr, 1, (void*)app_sensorData.i2cRxBuffer, 1);
        control_val = app_sensorData.i2cRxBuffer[0];
        
                registerAddr = 0xf;
        ret_val =  DRV_I2C_WriteReadTransfer(app_sensorData.i2cHandle, APP_SENSOR_I2C_SLAVE_ADDR, (void*)&registerAddr, 1, (void*)app_sensorData.i2cRxBuffer, 1);
        status_val = app_sensorData.i2cRxBuffer[0];

        registerAddr = 0x0;
        ret_val =  DRV_I2C_WriteReadTransfer(app_sensorData.i2cHandle, APP_SENSOR_I2C_SLAVE_ADDR, (void*)&registerAddr, 1, (void*)app_sensorData.i2cRxBuffer, 1);
        second_val = app_sensorData.i2cRxBuffer[0];

        if (true == ret_val)
        led2_Toggle();
#endif
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}




// *****************************************************************************
// *****************************************************************************
// Section: System "Tasks" Routine
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void SYS_Tasks ( void )

  Remarks:
    See prototype in system/common/sys_module.h.
*/
void SYS_Tasks ( void )
{
    /* Maintain system services */
    

    /* Create OS Thread for USB_DEVICE_Tasks. */
    xTaskCreate( _USB_DEVICE_Tasks,
        "USB_DEVICE_TASKS",
        896,
        (void*)NULL,
        1,
        (TaskHandle_t*)NULL
    );
    
	/* Create OS Thread for USB Driver Tasks. */
//    xTaskCreate( _DRV_USBFSV1_Tasks,
//        "DRV_USBFSV1_TASKS",
//        896,
//        (void*)NULL,
//        1,
//        (TaskHandle_t*)NULL
//    );
//
 xTaskCreate( APP_USB_Tasks,
        "APP_USB_TASKS",
        896,
        (void*)NULL,
        1,
        (TaskHandle_t*)NULL
    );

    /* Maintain the application's state machine. */
        /* Create OS Thread for APP_Tasks. */
    xTaskCreate((TaskFunction_t) _APP_Tasks,
                "APP_Tasks",
                64,
                NULL,
                1,
                &xAPP_Tasks);

/*
    xTaskCreate((TaskFunction_t) I2C_Task,
            "i2c_task",
            64,
            NULL,
            2,
            &xI2C_Tasks);
*/
    /* Start RTOS Scheduler. */
    
     /**********************************************************************
     * Create all Threads for APP Tasks before starting FreeRTOS Scheduler *
     ***********************************************************************/
    vTaskStartScheduler(); /* This function never returns. */

}

/*******************************************************************************
 End of File
 */

