#include <Loop/LoopManager.h>
#include "IntentState.h"
#include "IdleState.h"
#include "../Intent/Intent.hpp"

IntentState* IntentState::instance = nullptr;

IntentState::IntentState(Intent* intent, IntentInfo::Upsell* upsell) : intent(intent), upsell(upsell){
	instance = this;
}

IntentState::~IntentState(){
	instance = nullptr;
}

void IntentState::intentDone(){
	State::changeState(new IdleState());
}

void IntentState::loop(uint micros){
	if(intent == nullptr) return;
	intent->loop(micros);
}

void IntentState::enter(){
	LoopManager::addListener(this);
	intent->enter();
}

void IntentState::exit(){
	intent->exit();
	LoopManager::removeListener(this);

	delete intent;
	intent = nullptr;
}
