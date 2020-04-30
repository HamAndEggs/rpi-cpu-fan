
#include <iostream>
#include <fstream>
#include <signal.h>
#include <chrono>
#include <thread>
#include <string>

#include "./gpiomem/gpiomem.h"

static std::string CPU_TEMPERATURE_DEV_FILE = "/sys/class/thermal/thermal_zone0/temp";
static float MIN_CPU_TEMPERATURE = 50.0f;
static float MAX_CPU_TEMPERATURE = 65.0f;
static int SAMPLE_PAUSE = 5; // Sleeps for N seconds between checking of temperature.

static bool KeepGoing = true;

void static CtrlHandler(int SigNum)
{
	static int numTimesAskedToExit = 0;
	std::cout << std::endl << "Asked to quit, please wait" << std::endl;
	if( numTimesAskedToExit > 2 )
	{
		std::cout << "Asked to quit to many times, forcing exit in bad way" << std::endl;
		exit(1);
	}
	KeepGoing = false;
}

static float GetCpuTemp()
{
    std::ifstream cpuTemp(CPU_TEMPERATURE_DEV_FILE);
    if( cpuTemp )
    {
        std::string temperatureString;
        std::getline(cpuTemp,temperatureString);
        const float temperatureFloat = std::stof(temperatureString);

        return temperatureFloat / 1000.0f;
    }

    std::cout << "Failed to fetch cpu temperature from ";

    return 0.0f;
}

int main(int argc, char *argv[])
{
	signal (SIGINT,CtrlHandler);

    std::cout << "Hi, boot temperature is " << GetCpuTemp() << "C" << std::endl;

    gpio::GPIOMem GPIO;

    if( GPIO.Open() == false )
    {
        std::cout << "Failed to open GPIO memory interface!" << std::endl;
    }

    std::cout << "GPIO opened" << std::endl;

    GPIO.SetPinMode(18,gpio::GPIOMem::PINMODE_OUT);

    // Do a will boot spin up.

    std::this_thread::sleep_for(std::chrono::seconds(2));
    GPIO.SetPin(18,true);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    GPIO.SetPin(18,false);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    while( KeepGoing )
    {
        const float currentTemperature = GetCpuTemp();
        if( currentTemperature >= MAX_CPU_TEMPERATURE )
        {
            GPIO.SetPin(18,true);
        }
        else if( currentTemperature <= MIN_CPU_TEMPERATURE )
        {
            GPIO.SetPin(18,false);
        }

        // Now sleep before checking again.
        std::this_thread::sleep_for(std::chrono::seconds(SAMPLE_PAUSE));
    }

    GPIO.Close();

    return 0;
}

