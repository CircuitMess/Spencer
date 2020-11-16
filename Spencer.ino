#include <Arduino.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <CircuitOS.h>
#include <Input/InputGPIO.h>
#include "src/Speech/SpeechToIntent.h"
#include "src/Services/Audio/Playback.h"
#include "Spencer.hpp"
#include "src/LEDmatrix/LEDmatrix.h"
#include "src/State/State.h"
#include "src/State/IdleState.h"
#include "src/Util/FlashTools.h"
#include "src/Intent/IntentStore.h"
#include <Loop/LoopManager.h>
#include <WiFi.h>
#include "src/Services/TimeService/TimeService.h"
#include "src/Intent/TimeIntent.h"
#include "src/Services/Audio/Recording.h"
#include <Util/Task.h>
#include "src/Settings.h"
#include "src/Net.h"
#include "src/State/SetupState.h"

void setup(){
	Serial.begin(115200);

	disableCore0WDT();
	disableCore1WDT();

	SPIClass spi(3);
	spi.begin(18, 19, 23, FLASH_CS_PIN);
	if(!SerialFlash.begin(spi, FLASH_CS_PIN)){
		Serial.println("Flash fail");
		return;
	}

	pinMode(BTN_LED, OUTPUT);

	if(!LEDmatrix.begin()){
		Serial.println("couldn't start matrix");
		for(;;);
	}
	LEDmatrix.clear();
	LEDmatrix.setBrightness(20);
	LEDmatrix.setRotation(2);

	if(!Settings.begin()){
		Settings.reset();
		Settings.store();
	}

	I2S* i2s = new I2S();
	i2s_driver_uninstall(I2S_NUM_0); //revert wrong i2s config from esp8266audio
	i2s->begin();

	Playback.begin(i2s);
	Recording.begin(i2s);
	IntentStore::fillStorage();

	LoopManager::addListener(&Playback);
	LoopManager::addListener(&LEDmatrix);
	LoopManager::addListener(&TimeService);
	LoopManager::addListener(new InputGPIO());

	if(!Settings.begin()){
		Settings.reset();
		Settings.store();

		Playback.playMP3(SampleStore::load(Generic, "firstStartup"));
		Playback.setPlaybackDoneCallback([](){
			State::changeState(new SetupState());
		});
	}else{
		Net.set(Settings.get().SSID, Settings.get().pass);
		Net.connect();
	}

	Net.setStatusCallback([](wl_status_t status){
		if(status != WL_CONNECTED){
			Playback.playMP3(SampleStore::load(Generic, "noNet"));
			Playback.setPlaybackDoneCallback([](){
				State::changeState(new SetupState());
			});
		}else{
			State::changeState(new IdleState());
		}
	});

	LoopManager::setStackSize(10240);
	Task::setPinned(true);
	LoopManager::startTask(10);
	Task::setPinned(false);
}

void loop(){
	vTaskDelete(nullptr);
}