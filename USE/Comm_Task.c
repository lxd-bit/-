#include "cmsis_os.h"
#include "BlueSerial.h"
#include "PID.h"
#include "robot_state.h"
#include <string.h>
#include <stdlib.h>
#include "NRF24L01.h"

void Comm_Task(void *argument)
{
    NRF24L01_Init();
    for(;;)
    {
        /* ========== 1. 蓝牙数据解析 ========== */
        if (BlueSerial_RxFlag == 1)
        {
            char *tag = strtok((char *)BlueSerial_RxPacket, ",");
            if (tag != NULL && strcmp(tag, "key") == 0)
            {
                char *name = strtok(NULL, ",");
                char *action = strtok(NULL, ",");
                (void)name;
                (void)action;
            }
            else if (tag != NULL && strcmp(tag, "slider") == 0)
            {
                char *name = strtok(NULL, ",");
                char *value = strtok(NULL, ",");

                if (name != NULL && value != NULL)
                {
                    if (strcmp(name, "AngleKp") == 0)
                        AnglePID.Kp = atof(value);
                    else if (strcmp(name, "AngleKi") == 0)
                        AnglePID.Ki = atof(value);
                    else if (strcmp(name, "AngleKd") == 0)
                        AnglePID.Kd = atof(value);
                    else if (strcmp(name, "SpeedKp") == 0)
                        SpeedPID.Kp = atof(value);
                    else if (strcmp(name, "SpeedKi") == 0)
                        SpeedPID.Ki = atof(value);
                    else if (strcmp(name, "SpeedKd") == 0)
                        SpeedPID.Kd = atof(value);
                    else if (strcmp(name, "TurnKp") == 0)
                        TurnPID.Kp = atof(value);
                    else if (strcmp(name, "TurnKi") == 0)
                        TurnPID.Ki = atof(value);
                    else if (strcmp(name, "TurnKd") == 0)
                        TurnPID.Kd = atof(value);
                    else if (strcmp(name, "Offset") == 0)
                        AnglePID.OutOffset = atof(value);
                }
            }
            else if (tag != NULL && strcmp(tag, "joystick") == 0)
            {
                char *leftH = strtok(NULL, ",");
                char *leftV = strtok(NULL, ",");
                char *rightH = strtok(NULL, ",");
                char *rightV = strtok(NULL, ",");

                if (leftH != NULL && leftV != NULL &&
                    rightH != NULL && rightV != NULL)
                {
                    SpeedPID.Target = atoi(leftV) / 25.0f;
                    TurnPID.Target = atoi(rightH) / 25.0f;
                }
            }

            BlueSerial_RxFlag = 0;
        }
        /* ========== 2. NRF24L01 接收 ========== */
        if (NRF24L01_Receive() == 1)
        {
            uint8_t ID = NRF24L01_RxPacket[0];
            if (ID == 0x00)
            {
                int8_t LV = NRF24L01_RxPacket[2];
                int8_t RH = NRF24L01_RxPacket[3];
                uint8_t KEY = NRF24L01_RxPacket[5];
                
                SpeedPID.Target = LV / 25.0f;
                TurnPID.Target = RH / 25.0f;
                
                if (KEY == 1)
                {
                    Run_Flag = !Run_Flag;
                    if (Run_Flag) {
                        PID_Init(&AnglePID);
                        PID_Init(&SpeedPID);
                        PID_Init(&TurnPID);
                    }
                }
            }
        }
        osDelay(10);
    }
}
