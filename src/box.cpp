#include "box.h"
#include "cat.h"
#include "application.h"

extern Cat *cat1;

extern Application *application;
Box::Box(float weight=0.7)
{
    this->boxWeight = boxWeight;
}

byte Box::getUses(void) {
    return this->uses;
}

float Box::getSandWeight(void){
    return this->sandWeight;
}

float Box::setSandWeight(float weight){
    this->sandWeight = weight;
    return sandWeight;
}

byte Box::setUses(byte numUses){
    this->uses = numUses;
    return this->uses;
}

void Box::incrementUses(void){
    this->uses++;
    return;
}

float Box::getBoxWeight(void){
    return this->boxWeight;
}

float Box::getPooWeight(void){
    return this->pooWeight;
}

void Box::setPooWeight(float weight){
    this->pooWeight = this->currentWeight - (this->boxWeight - this->sandWeight - application->getPlatformWeight());
    return;
}

float Box::getCurrentWeight(void) {
    return this->currentWeight;
}

void Box::setCurrentWeight(float weight){
    this->currentWeight = weight;
    return;
}

float Box::getLastWeight(void){
    return this->lastWeight;
}

void Box::setLastWeight(float weight){
    this->lastWeight = weight;
}

bool Box::catPresent(void){
    //float scaleWeight = scale->get_units(3);
    if(this->currentWeight > (this->pooWeight + cat1->getMinWeight())){
        return true;
    }
    else
    {
        return false;
    }
}