#include <string.h>
#include "application.h"
#include "cat.h"

// Constructor
Cat::Cat(double minWeight, double maxWeight) {
    this->minWeight = minWeight;
    this->maxWeight = maxWeight;
}

// Getter functions
float Cat::getMinWeight(void) {
    return this->minWeight;
}

float Cat::getMaxWeight(void) {
    return this->maxWeight;
}

float Cat::getCurrentWeight(void) {
    return this->currentWeight;
}

void Cat::setCurrentWeight(float weight){
    this->currentWeight = weight;
}

byte Cat::getUses(void) {
    return this->uses;
}

byte Cat::setUses(byte numUses){
    this->uses = numUses;
    return this->uses;
}

void Cat::incrementUses(void){
    this->uses++;
    return;
}