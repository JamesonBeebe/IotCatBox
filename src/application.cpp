#include "application.h"

Application::Application(float weight = 1.8){
    this->platformWeight = weight;
}

Application::~Application(){
}

float Application::setPlatformWeight(float weight){
    this->platformWeight = weight;
    return this->platformWeight;
}


float Application::getPlatformWeight(void){
    return this->platformWeight;
}