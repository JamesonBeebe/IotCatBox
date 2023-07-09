#include "box.h"
#include "cat.h"
#include "application.h"

extern Cat *cat1;

extern Application *application;
Box::Box(float weight = 0.7)
{
    this->boxWeight = boxWeight;
}

Box::~Box()
{
}

float Box::getSandWeight(void)
{
    return this->sandWeight;
}

float Box::setSandWeight(float weight)
{
    this->sandWeight = weight;
    return sandWeight;
}

float Box::getBoxWeight(void)
{
    return this->boxWeight;
}

float Box::getPooWeight(void)
{
    return this->pooWeight;
}

void Box::setPooWeight(float weight)
{
    this->pooWeight = weight;
    return;
}

float Box::getCurrentWeight(void)
{
    return this->currentWeight;
}

void Box::setCurrentWeight(float weight)
{
    this->currentWeight = weight;
    return;
}

float Box::getLastWeight(void)
{
    return this->lastWeight;
}
void Box::setLastWeight(float weight)
{
    this->lastWeight = weight;
}

bool Box::weightIsStable(void)
{
    if (abs(this->currentWeight - this->lastWeight) < MEASUREMENT_TOLERANCE)
    {
        if ((millis() - this->settlingTime) > SETTLING_TIME)
        {
            this->lastStableWeight = this->currentWeight;
            return true;
        }
    }
    else
    {
        this->settlingTime = millis(); // reset timer
    }
    Serial.println("Weight unstable: " + String(this->currentWeight));
    return false;
}

bool Box::catPresent(void)
{
    if (this->currentWeight > (this->pooWeight + cat1->getMinWeight()))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Box::zeroPooValue(void)
{
    Serial.println("Zeroing poo value");
    this->setPooWeight(0);
}